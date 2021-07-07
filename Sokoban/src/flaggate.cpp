#include "stdafx.h"
#include "flaggate.h"

#include "gameobject.h"
#include "roommap.h"
#include "mapfile.h"
#include "common_enums.h"
#include "graphicsmanager.h"
#include "moveprocessor.h"
#include "animationmanager.h"
#include "texture_constants.h"
#include "wall.h"
#include "modelinstancer.h"
#include "playingstate.h"
#include "room.h"
#include "savefile.h"

FlagGate::FlagGate(GameObject* parent, int num_flags, int orientation, int count, bool active, bool walls_placed, bool down) :
	Switchable(parent, count, false, false, false, false),
	num_flags_{ num_flags }, orientation_{ orientation }, walls_placed_{ walls_placed }, down_{ down } {
	init_draw_constants();
	spawn_sigils();
}

FlagGate::~FlagGate() {}

void FlagGate::make_str(std::string& str) {
	char buf[32];
	snprintf(buf, 32, "FlagGate:%d", num_flags_);
	str += buf;
}

ModCode FlagGate::mod_code() {
	return ModCode::FlagGate;
}

void FlagGate::serialize(MapFileO& file) {
	file << num_flags_ << orientation_ << count_ << active_ << walls_placed_ << down_;
}

void FlagGate::deserialize(MapFileI& file, RoomMap*, GameObject* parent) {
	unsigned char b[6];
	file.read(b, 6);
	auto fg = std::make_unique<FlagGate>(parent, b[0], b[1], b[2], b[3], b[4], b[5]);
	parent->set_modifier(std::move(fg));
}

std::unique_ptr<ObjectModifier> FlagGate::duplicate(GameObject* parent, RoomMap*, DeltaFrame*) {
	auto dup = std::make_unique<FlagGate>(*this);
	dup->parent_ = parent;
	return std::move(dup);
}

bool FlagGate::can_set_state(bool, RoomMap*) {
	return true;
}

void FlagGate::map_callback(RoomMap* map, DeltaFrame* delta_frame, MoveProcessor* mp) {
	if (!walls_placed_) {
		place_walls(map);
		walls_placed_ = true;
	}
}

void FlagGate::apply_state_change(RoomMap* map, DeltaFrame* delta_frame, MoveProcessor* mp) {
	signal_animation(mp->anims_, mp->delta_frame_);
	if (!down_ && state() && mp->playing_state_->global_->clear_flag_total_ >= num_flags_) {
		remove_walls(map, delta_frame, mp);
		animation_state_ = FlagGateAnimationState::Charging;
		stored_delta_frame_ = delta_frame;
		mp->playing_state_->mandatory_wait_ = true;
	}
}

void FlagGate::signal_animation(AnimationManager* anims, DeltaFrame* delta_frame) {
	if (parent_->tangible_) {
		anims->receive_signal(AnimationSignal::FlagGateOn, parent_, delta_frame);
	} else {
		anims->receive_signal(AnimationSignal::FlagGateOff, parent_, delta_frame);
	}
}

void FlagGate::place_walls(RoomMap* map) {
	Point3 a, b;
	get_gate_extremes(a, b);
	for (int x = a.x; x <= b.x; ++x) {
		for (int y = a.y; y <= b.y; ++y) {
			for (int z = a.z; z <= b.z; ++z) {
				Point3 wall_pos{ x, y, z };
				map->create_in_map(std::make_unique<ArtWall>(wall_pos, 1), false, nullptr);
			}
		}
	}
}

void FlagGate::remove_walls(RoomMap* map, DeltaFrame* delta_frame, MoveProcessor* mp) {
	Point3 a, b;
	get_gate_extremes(a, b);
	for (int x = a.x; x <= b.x; ++x) {
		for (int y = a.y; y <= b.y; ++y) {
			for (int z = a.z; z <= b.z; ++z) {
				auto* wall = map->view(Point3{ x, y, z });
				map->take_from_map(wall, true, false, delta_frame);
				wall->destroy(mp, CauseOfDeath::None);
			}
		}
	}
}

void FlagGate::get_gate_extremes(Point3& a, Point3& b) {
	int width, height;
	get_gate_dims(&width, &height);
	Point3 pos = parent_->pos_;
	switch (orientation_) {
	case 0:
		a.x = pos.x;
		b.x = pos.x + width - 1;
		a.y = pos.y;
		b.y = pos.y;
		break;
	case 1:
		a.x = pos.x;
		b.x = pos.x;
		a.y = pos.y;
		b.y = pos.y + width - 1;
		break;
	case 2:
		a.x = pos.x - width + 1;
		b.x = pos.x;
		a.y = pos.y;
		b.y = pos.y;
		break;
	case 3:
		a.x = pos.x;
		b.x = pos.x;
		a.y = pos.y - width + 1;
		b.y = pos.y;
		break;
	}
	a.z = pos.z + 1;
	b.z = pos.z + height;
}

void FlagGate::get_gate_dims(int* width, int* height) {
	switch (num_flags_) {
	case 1:
		*width = 1;
		*height = 1;
		break;
	case 2:
	case 4:
	case 8:
		*width = 3;
		*height = 3;
		break;
	case 24:
	case 32:
	case 36:
		*width = 7;
		*height = 7;
		break;
	}
}

void FlagGate::init_draw_constants() {
	int width, height;
	get_gate_dims(&width, &height);
	switch (orientation_) {
	case 0:
		center_ = glm::vec3((width - 1) / 2.0, 0.0, 0.5 + height / 2.0);
		scale_ = glm::vec3(width, 1.02f, height);
		break;
	case 1:
		center_ = glm::vec3(0.0, (width - 1) / 2.0, 0.5 + height / 2.0);
		scale_ = glm::vec3(1.02f, width, height);
		break;
	case 2:
		center_ = glm::vec3(- (width - 1) / 2.0, 0.0, 0.5 + height / 2.0);
		scale_ = glm::vec3(width, 1.02f, height);
		break;
	case 3:
		center_ = glm::vec3(0.0, - (width - 1) / 2.0, 0.5 + height / 2.0);
		scale_ = glm::vec3(1.02f, width, height);
		break;
	}
}

void FlagGate::spawn_sigils() {
	if (!sigils_.empty()) {
		return;
	}
	switch (num_flags_) {
	case 1:
		sigils_.push_back(FlagSigil{ center_, 0.0, 0, 1, 0 });
		break;
	case 2:
		for (int i = 0; i < 2; ++i) {
			sigils_.push_back(FlagSigil{ center_, 0.5, 80 * i, 80 * 2, i });
		}
		break;
	case 4:
		for (int i = 0; i < 4; ++i) {
			sigils_.push_back(FlagSigil{ center_, 0.75, 60 * i, 60 * 4, i });
		}
		break;
	case 8:
		sigils_.push_back(FlagSigil{ center_, 0.0, 0, 1, 0 });
		for (int i = 0; i < 7; ++i) {
			sigils_.push_back(FlagSigil{ center_, 1.0, 40 * i, 40 * 7, i+1 });
		}
		break;
	case 24:
		for (int i = 0; i < 3; ++i) {
			sigils_.push_back(FlagSigil{ center_, 0.7, 80 * i, 80 * 3, i });
		}
		for (int i = 0; i < 8; ++i) {
			sigils_.push_back(FlagSigil{ center_, 1.6, -80 * i, -80 * 8, i + 3 });
		}
		for (int i = 0; i < 13; ++i) {
			sigils_.push_back(FlagSigil{ center_, 2.5, 80 * i, 80 * 13, i + 11 });
		}
		for (auto& sigil : sigils_) {
			sigil.lum = 1.0f;
		}
		break;
	case 36:
		for (int i = 0; i < 4; ++i) {
			sigils_.push_back(FlagSigil{ center_, 0.7, -80 * i, -80 * 4, i });
		}
		for (int i = 0; i < 10; ++i) {
			sigils_.push_back(FlagSigil{ center_, 1.6, 80 * i, 80 * 10, i + 4 });
		}
		for (int i = 0; i < 18; ++i) {
			sigils_.push_back(FlagSigil{ center_, 2.5, -80 * i, -80 * 18, i + 14 });
		}
		for (int i = 0; i < 2; i++) {
			for (int j = 0; j < 2; j++) {
				sigils_.push_back(FlagSigil{ center_ + glm::vec3(5 * (i - 0.5), 0, 5 * (j - 0.5)), 0, 0, 1, (2*i + j) + 32 });
			}
		}
		break;
	default:
		break;
	}
}

int FlagGate::get_reset_time() {
	switch (num_flags_) {
	case 1:
		return 1;
	case 2:
		return 160;
	case 4:
		return 240;
	case 8:
		return 280;
	case 24:
		return 24960;
	case 32:
	case 36:
		return 14400;
	default:
		return 1;
	}
}

const int FLAG_GATE_SHRINK_TIME = 20;
const int MAX_FLAG_SIGIL_CHARGE = 20;
const int FLAG_SIGIL_DELAY = 8;

bool FlagGate::update_animation(PlayingState* playing_state) {
	switch (animation_state_) {
	case FlagGateAnimationState::Charging:
		if (sigils_[sigils_.size() - 1].charge == FLAG_SIGIL_DELAY * num_flags_ + MAX_FLAG_SIGIL_CHARGE) {
			animation_state_ = FlagGateAnimationState::Fade;
			break;
		}
		// Fallthrough
	case FlagGateAnimationState::Default:
	{
		++cycle_time_;
		if (cycle_time_ >= get_reset_time()) {
			cycle_time_ = 0;
		}
		if (state()) {
			int flag_count = playing_state->global_->clear_flag_total_;
			int cur_count = flag_count;
			for (auto& sigil : sigils_) {
				sigil.update(cur_count > 0, flag_count);
				--cur_count;
			}
		} else {
			for (auto& sigil : sigils_) {
				sigil.update(false, 0);
			}
		}
		break;
	}
	case FlagGateAnimationState::Fade:
		for (auto& sigil : sigils_) {
			--sigil.opacity;
		}
		if (sigils_[0].opacity == 0) {
			animation_state_ = FlagGateAnimationState::Shrink;
			int height, width;
			get_gate_dims(&width, &height);
			animation_timer_ = FLAG_GATE_SHRINK_TIME * height;
		}
		break;
	case FlagGateAnimationState::Shrink:
		--animation_timer_;
		if (animation_timer_ == 0) {
			playing_state->mandatory_wait_ = false;
			animation_state_ = FlagGateAnimationState::Default;
			down_ = true;
			playing_state->delta_frame_->push(std::make_unique<FlagGateOpenDelta>(this));
			signal_animation(playing_state->anims_.get(), playing_state->delta_frame_.get());
		}
	default:
		break;
	}
	return false;
}

void FlagGate::reset_animation() {
	cycle_time_ = 0;
}

void FlagGate::draw(GraphicsManager* gfx, FPoint3 p) {
	if (down_) {
		glm::vec3 center = glm::vec3(center_.x, center_.y, 0.5f * (1.0f + 0.05f));
		glm::vec3 scale = glm::vec3(scale_.x, scale_.y, 0.05f);
		gfx->cube.push_instance(glm::vec3(p) + center, scale, BlockTexture::SolidEdgesDark, parent_->color());
		return;
	}
	switch (animation_state_) {
	case FlagGateAnimationState::Default:
	case FlagGateAnimationState::Charging:
	case FlagGateAnimationState::Fade:
	{
		gfx->cube.push_instance(glm::vec3(p) + center_, scale_, BlockTexture::SolidEdgesDark, parent_->color());
		ModelInstancer* sigil_model;
		switch (orientation_) {
		case 0:
		default:
			sigil_model = &gfx->square_0;
			break;
		case 1:
			sigil_model = &gfx->square_1;
			break;
		case 2:
			sigil_model = &gfx->square_2;
			break;
		case 3:
			sigil_model = &gfx->square_3;
			break;
		}
		for (auto& sigil : sigils_) {
			sigil.draw(sigil_model, p, orientation_, cycle_time_);
		}
	} break;
	case FlagGateAnimationState::Shrink:
		float shrink_factor = (float)animation_timer_ / (float)FLAG_GATE_SHRINK_TIME;
		glm::vec3 center = glm::vec3(center_.x, center_.y, 0.5f * (1.0f + shrink_factor));
		glm::vec3 scale = glm::vec3(scale_.x, scale_.y, shrink_factor);
		gfx->cube.push_instance(glm::vec3(p) + center, scale, BlockTexture::SolidEdgesDark, parent_->color());
	}
}

void FlagSigil::update(bool charging, int total) {
	if (charging && charge < FLAG_SIGIL_DELAY * total + MAX_FLAG_SIGIL_CHARGE) {
		++charge;
	} else if (!charging && charge > 0) {
		--charge;
	}
}

void FlagSigil::draw(ModelInstancer* model, FPoint3 p, int orientation, int time) {
	glm::vec3 pos(p);
	BlockTexture tex = BlockTexture::FlagSigil;
	double t = TWO_PI * (time + phase) / (double)period;
	switch (orientation) {
	case 0:
		pos += center + glm::vec3{ -radius*cos(t), 0.52, radius*sin(t) };
		break;
	case 1:
		pos += center + glm::vec3{ -0.52, -radius*cos(t), radius*sin(t) };
		break;
	case 2:
		pos += center + glm::vec3{ radius*cos(t), -0.52, radius*sin(t) };
		break;
	case 3:
		pos += center + glm::vec3{ 0.52, radius*cos(t), radius*sin(t) };
		break;
	}
	float value = std::min(std::max((charge - FLAG_SIGIL_DELAY * index), 0), MAX_FLAG_SIGIL_CHARGE) / (float)MAX_FLAG_SIGIL_CHARGE;
	glm::vec4 color = glm::vec4(lum, lum, lum, 1.0f) * glm::vec4(1 - value) + COLOR_VECTORS[GOLD] * glm::vec4(value);
	color.w = (float)opacity / (float)MAX_FLAG_SIGIL_OPACITY;
	model->push_instance(pos, glm::vec3(0.6f), tex, color);
}


FlagGateOpenDelta::FlagGateOpenDelta(FlagGate* fg) : Delta(), fg_{ fg } {}

FlagGateOpenDelta::FlagGateOpenDelta(FrozenObject fg) : Delta(), fg_{ fg } {}

FlagGateOpenDelta::~FlagGateOpenDelta() {}

void FlagGateOpenDelta::serialize(MapFileO& file) {
	fg_.serialize(file);
}

void FlagGateOpenDelta::revert(RoomMap* room_map) {
	auto* fg = static_cast<FlagGate*>(fg_.resolve_mod(room_map));
	fg->down_ = false;
	for (auto& sigil : fg->sigils_) {
		sigil.charge = 0;
		sigil.opacity = MAX_FLAG_SIGIL_OPACITY;
	}
}

DeltaCode FlagGateOpenDelta::code() {
	return DeltaCode::FlagGateOpenDelta;
}

std::unique_ptr<Delta> FlagGateOpenDelta::deserialize(MapFileI& file) {
	return std::make_unique<FlagGateOpenDelta>(file.read_frozen_obj());
}
