#include "stdafx.h"
#include "pausestate.h"

#include "graphicsmanager.h"
#include "fontmanager.h"
#include "stringdrawer.h"
#include "savefile.h"
#include "common_constants.h"
#include "realplayingstate.h"
#include "testplayingstate.h"
#include "menu.h"
#include "globalflagconstants.h"


PauseState::PauseState(GameState* parent) : GameState(parent),
playing_state_{ static_cast<PlayingState*>(parent) },
menu_{ std::make_unique<Menu>(window_, gfx_->fonts_->get_font(Fonts::ABEEZEE, 72)) } {
	menu_->push_entry("Unpause", [this]() { unpause(); });
	if (auto* real_ps = dynamic_cast<RealPlayingState*>(parent)) {
		//menu_->push_entry("Save Game", [this]() { playing_state_->make_subsave(); });
		if (real_ps->savefile_->auto_subsave_ >= 0) {
			menu_->push_entry("Load from Autosave", [this]() { load_last_autosave(); });
		}
		if (playing_state_->global_->has_flag(get_misc_flag(MiscGlobalFlag::WorldResetLearned))) {
			menu_->push_entry("World Reset", [this]() { world_reset(); });
		}
		menu_->push_entry("Save and Quit", [this]() { quit_playing(); });
		//menu_->push_entry("Quit Without Saving", [this]() { quit_without_saving(); });
	} else if (dynamic_cast<TestPlayingState*>(parent)) {
		menu_->push_entry("Quit Test Session", [this]() { quit_playing(); });
	}
}

PauseState::~PauseState() {}


void PauseState::main_loop() {
	menu_->update();
	menu_->handle_input(this);
	draw();
}

void PauseState::draw() {
	double prev_shadow = gfx_->shadow_;
	gfx_->shadow_ *= 0.3;
	gfx_->post_rendering();
	gfx_->shadow_ = prev_shadow;
	menu_->draw();
}

void PauseState::unpause() {
	queue_quit();
}

void PauseState::load_last_autosave() {
	auto* rps = static_cast<RealPlayingState*>(playing_state_);
	rps->savefile_->load_last_autosave();
	rps->play_from_loaded_subsave();
	queue_quit();
}

void PauseState::save() {
	playing_state_->make_subsave(SaveType::Current);
	queue_quit();
}

void PauseState::quit_playing() {
	queue_quit();
	playing_state_->should_save_ = true;
	playing_state_->queue_quit();
}

void PauseState::quit_without_saving() {
	queue_quit();
	playing_state_->should_save_ = false;
	playing_state_->queue_quit();
}

void PauseState::world_reset() {
	playing_state_->world_reset();
	queue_quit();
}
