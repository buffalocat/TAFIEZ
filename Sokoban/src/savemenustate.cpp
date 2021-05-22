#include "stdafx.h"
#include "savemenustate.h"

#include "graphicsmanager.h"
#include "realplayingstate.h"
#include "savefile.h"
#include "stringdrawer.h"
#include "fontmanager.h"
#include "color_constants.h"
#include "common_constants.h"


LoadMenuState::LoadMenuState(GameState* parent) : GameState(parent) {
	rps_ = static_cast<RealPlayingState*>(parent->parent_.get());
	savefile_ = rps_->savefile_;
	font_ = gfx_->fonts_->get_font(Fonts::ABEEZEE, 24);
	big_font_ = gfx_->fonts_->get_font(Fonts::ABEEZEE, 48);
	active_color_ = COLOR_VECTORS[GOLD];
	inactive_color_ = COLOR_VECTORS[LIGHT_BLUE];
	for (int i = 0; i < NUM_AUTO_SAVES; ++i) {
		auto_saves_.push_back({});
	}
	for (int i = 0; i < NUM_MANUAL_SAVES; ++i) {
		manual_saves_.push_back({});
	}
	generate_callbacks();
	generate_text();
	active_string_drawer_ = return_option_.get();
	prev_active_sd_ = active_string_drawer_;
}

LoadMenuState::~LoadMenuState() {}


void LoadMenuState::generate_callbacks() {
	backup_save_.callback = [this]() { return rps_->load_subsave_dispatch(SaveType::Emergency, 0); };
	for (int i = 0; i < auto_saves_.size(); ++i) {
		auto_saves_[i].callback = [this, i]() { return rps_->load_subsave_dispatch(SaveType::Auto, i); };
	}
	for (int i = 0; i < manual_saves_.size(); ++i) {
		manual_saves_[i].callback = [this, i]() { return rps_->load_subsave_dispatch(SaveType::Manual, i); };
	}
}

std::string generate_save_name(SaveType type, int index, SubSave* subsave) {
	std::string base;
	switch (type) {
	case SaveType::Emergency:
		base = std::string("Backup Save");
		break;
	case SaveType::Auto:
		base = std::string("Autosave ") + std::to_string(index);
		break;
	case SaveType::Manual:
		base = std::string("Manual Save ") + std::to_string(index);
		break;
	}
	if (subsave) {
		base += std::string("\nZone ") + std::string(1, subsave->zone_);
		base += "\n";
		base += subsave->time_stamp_;
	} else {
		base += "\nEmpty";
	}
	return base;
}


const float LEFT_EDGE = -0.76f;
const float HOR_SPACE = 0.38f;
const float TOP_ROW = 0.8f;
const float VERT_SPACE = -0.35f;


void LoadMenuState::generate_text() {
	return_option_ = std::make_unique<StringDrawer>(big_font_, inactive_color_,
		"Return", LEFT_EDGE, TOP_ROW, 1.0f, 1.0f, 0.0f);
	string_drawers_.push_back(return_option_.get());
	backup_save_.text = std::make_unique<StringDrawer>(font_, inactive_color_,
		generate_save_name(SaveType::Emergency, 0, savefile_->emergency_save_.get()),
		LEFT_EDGE + HOR_SPACE, TOP_ROW, 1.0f, 1.0f, 0.0f);
	string_drawers_.push_back(backup_save_.text.get());
	for (int i = 0; i < NUM_AUTO_SAVES; ++i) {
		auto_saves_[i].text = std::make_unique<StringDrawer>(font_, inactive_color_,
			generate_save_name(SaveType::Auto, (i + 1), savefile_->auto_saves_[i].get()),
			LEFT_EDGE, TOP_ROW + VERT_SPACE * (i + 1), 1.0f, 1.0f, 0.0f);
		string_drawers_.push_back(auto_saves_[i].text.get());
	}
	for (int i = 0; i < NUM_MANUAL_SAVES; ++i) {
		manual_saves_[i].text = std::make_unique<StringDrawer>(font_, inactive_color_,
			generate_save_name(SaveType::Manual, (i + 1), savefile_->manual_saves_[i].get()),
			LEFT_EDGE + HOR_SPACE * (i % (NUM_SAVE_COLUMNS - 1) + 1),
			TOP_ROW + VERT_SPACE * (i / (NUM_SAVE_COLUMNS - 1) + 1), 1.0f, 1.0f, 0.0f);
		string_drawers_.push_back(manual_saves_[i].text.get());
	}
}


const int MAX_MENU_COOLDOWN = 8;

void LoadMenuState::handle_entry_select_input() {
	can_select_ = true;
	if (key_pressed(GLFW_KEY_UP)) {
		if (input_cooldown_ == 0) {
			move_y(-1);
			input_cooldown_ = MAX_MENU_COOLDOWN;
		} else {
			--input_cooldown_;
		}
	} else if (key_pressed(GLFW_KEY_DOWN)) {
		if (input_cooldown_ == 0) {
			move_y(1);
			input_cooldown_ = MAX_MENU_COOLDOWN;
		} else {
			--input_cooldown_;
		}
	} else 	if (key_pressed(GLFW_KEY_RIGHT)) {
		if (input_cooldown_ == 0) {
			move_x(1);
			input_cooldown_ = MAX_MENU_COOLDOWN;
		} else {
			--input_cooldown_;
		}
	} else if (key_pressed(GLFW_KEY_LEFT)) {
		if (input_cooldown_ == 0) {
			move_x(-1);
			input_cooldown_ = MAX_MENU_COOLDOWN;
		} else {
			--input_cooldown_;
		}
	} else {
		input_cooldown_ = 0;
	}
	set_cur_save_slot();
}

void LoadMenuState::move_x(int dir) {
	menu_x_ += dir;
	if (menu_y_ == 0) {
		menu_x_ = (menu_x_ + 2) % 2;
	} else {
		menu_x_ = (menu_x_ + NUM_SAVE_COLUMNS) % NUM_SAVE_COLUMNS;
	}
}

void LoadMenuState::move_y(int dir) {
	menu_y_ += dir;
	menu_y_ = (menu_y_ + NUM_SAVE_ROWS) % NUM_SAVE_ROWS;
	if (menu_y_ == 0) {
		if (menu_x_ > 1) {
			menu_x_ = 1;
		}
	}
}

void LoadMenuState::set_cur_save_slot() {
	if (menu_y_ == 0) {
		if (menu_x_ == 0) {
			cur_slot_ = nullptr;
			active_string_drawer_ = return_option_.get();
			return;
		} else {
			cur_slot_ = &backup_save_;
		}
	} else {
		if (menu_x_ == 0) {
			cur_slot_ = &auto_saves_[menu_y_ - 1];
		} else {
			int idx = (menu_y_ - 1) * (NUM_SAVE_COLUMNS - 1) + menu_x_ - 1;
			cur_slot_ = &manual_saves_[idx];
		}
	}
	active_string_drawer_ = cur_slot_->text.get();
}

void LoadMenuState::handle_input(GameState* game_state) {
	if (key_pressed(GLFW_KEY_ENTER)) {
		if (can_select_) {
			can_select_ = false;
			if (cur_slot_) {
				if (cur_slot_->callback()) {
					queue_quit();
					parent_->queue_quit();
				}
			} else {
				queue_quit();
			}
		}
	} else {
		handle_entry_select_input();
	}
}


void LoadMenuState::main_loop() {
	set_cur_save_slot();
	update();
	handle_input(this);
	draw();
}


void LoadMenuState::update() {
	prev_active_sd_->set_color(inactive_color_);
	active_string_drawer_->set_color(active_color_);
	prev_active_sd_ = active_string_drawer_;
}

void LoadMenuState::draw() {
	double prev_shadow = gfx_->shadow_;
	gfx_->shadow_ *= 0.1;
	gfx_->post_rendering();
	gfx_->shadow_ = prev_shadow;
	// Draw text
	font_->shader_->use();
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	for (auto& text : string_drawers_) {
		text->render();
	}
}





SaveMenuState::SaveMenuState(GameState* parent) : GameState(parent) {
	rps_ = static_cast<RealPlayingState*>(parent->parent_.get());
	savefile_ = rps_->savefile_;
	font_ = gfx_->fonts_->get_font(Fonts::ABEEZEE, 24);
	big_font_ = gfx_->fonts_->get_font(Fonts::ABEEZEE, 48);
	active_color_ = COLOR_VECTORS[GOLD];
	inactive_color_ = COLOR_VECTORS[LIGHT_BLUE];
	for (int i = 0; i < NUM_MANUAL_SAVES; ++i) {
		manual_saves_.push_back({});
	}
	generate_callbacks();
	generate_text();
	active_string_drawer_ = return_option_.get();
	prev_active_sd_ = active_string_drawer_;
}

SaveMenuState::~SaveMenuState() {}


void SaveMenuState::generate_callbacks() {
	for (int i = 0; i < manual_saves_.size(); ++i) {
		manual_saves_[i].callback = [this, i]() { rps_->make_subsave(SaveType::Manual, i); };
	}
}

void SaveMenuState::generate_text() {
	return_option_ = std::make_unique<StringDrawer>(big_font_, inactive_color_,
		"Return", LEFT_EDGE, TOP_ROW, 1.0f, 1.0f, 0.0f);
	string_drawers_.push_back(return_option_.get());
	for (int i = 0; i < NUM_MANUAL_SAVES; ++i) {
		manual_saves_[i].text = std::make_unique<StringDrawer>(font_, inactive_color_,
			generate_save_name(SaveType::Manual, (i + 1), savefile_->manual_saves_[i].get()),
			LEFT_EDGE + HOR_SPACE * (i % (NUM_SAVE_COLUMNS - 1) + 1),
			TOP_ROW + VERT_SPACE * (i / (NUM_SAVE_COLUMNS - 1)), 1.0f, 1.0f, 0.0f);
		string_drawers_.push_back(manual_saves_[i].text.get());
	}
}



void SaveMenuState::handle_entry_select_input() {
	can_select_ = true;
	if (key_pressed(GLFW_KEY_UP)) {
		if (input_cooldown_ == 0) {
			move_y(-1);
			input_cooldown_ = MAX_MENU_COOLDOWN;
		} else {
			--input_cooldown_;
		}
	} else if (key_pressed(GLFW_KEY_DOWN)) {
		if (input_cooldown_ == 0) {
			move_y(1);
			input_cooldown_ = MAX_MENU_COOLDOWN;
		} else {
			--input_cooldown_;
		}
	} else 	if (key_pressed(GLFW_KEY_RIGHT)) {
		if (input_cooldown_ == 0) {
			move_x(1);
			input_cooldown_ = MAX_MENU_COOLDOWN;
		} else {
			--input_cooldown_;
		}
	} else if (key_pressed(GLFW_KEY_LEFT)) {
		if (input_cooldown_ == 0) {
			move_x(-1);
			input_cooldown_ = MAX_MENU_COOLDOWN;
		} else {
			--input_cooldown_;
		}
	} else {
		input_cooldown_ = 0;
	}
	set_cur_save_slot();
}

void SaveMenuState::move_x(int dir) {
	menu_x_ += dir;
	menu_x_ = (menu_x_ + NUM_SAVE_COLUMNS) % NUM_SAVE_COLUMNS;
	if (menu_x_ == 0) {
		menu_y_ = 0;
	} else {
		
	}
}

void SaveMenuState::move_y(int dir) {
	menu_y_ += dir;
	menu_y_ = (menu_y_ + (NUM_SAVE_ROWS - 1)) % (NUM_SAVE_ROWS - 1);
	if (menu_y_ > 0 && menu_x_ == 0) {
		menu_x_ = 1;
	}
}

void SaveMenuState::set_cur_save_slot() {
	if (menu_x_ == 0) {
		cur_slot_ = nullptr;
		active_string_drawer_ = return_option_.get();
		return;
	} else {
		int idx = (menu_y_) * (NUM_SAVE_COLUMNS - 1) + menu_x_ - 1;
		cur_slot_ = &manual_saves_[idx];
	}
	active_string_drawer_ = cur_slot_->text.get();
}


void SaveMenuState::handle_input(GameState* game_state) {
	if (key_pressed(GLFW_KEY_ENTER)) {
		if (can_select_) {
			can_select_ = false;
			if (cur_slot_) {
				cur_slot_->callback();
				queue_quit();
				parent_->queue_quit();
			} else {
				queue_quit();
			}
		}
	} else {
		handle_entry_select_input();
	}
}


void SaveMenuState::main_loop() {
	set_cur_save_slot();
	update();
	handle_input(this);
	draw();
}


void SaveMenuState::update() {
	prev_active_sd_->set_color(inactive_color_);
	active_string_drawer_->set_color(active_color_);
	prev_active_sd_ = active_string_drawer_;
}

void SaveMenuState::draw() {
	double prev_shadow = gfx_->shadow_;
	gfx_->shadow_ *= 0.1;
	gfx_->post_rendering();
	gfx_->shadow_ = prev_shadow;
	// Draw text
	font_->shader_->use();
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	for (auto& text : string_drawers_) {
		text->render();
	}
}




