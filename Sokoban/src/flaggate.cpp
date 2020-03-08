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
#include "savefile.h"

FlagGate::FlagGate(GameObject* parent, int num_flags, int orientation, int count, bool active, bool walls_placed) :
	Switchable(parent, count, false, false, false, false),
	num_flags_{ num_flags }, orientation_{ orientation }, walls_placed_{ walls_placed } {
	init_draw_constants();
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
	file << num_flags_ << orientation_ << count_ << active_ << walls_placed_;
}

void FlagGate::deserialize(MapFileI& file, RoomMap* map, GameObject* parent) {
	unsigned char b[5];
	file.read(b, 5);
	auto fg = std::make_unique<FlagGate>(parent, b[0], b[1], b[2], b[3], b[4]);
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
	if (!down_) {
		spawn_sigils();
	}
}

void FlagGate::destroy(MoveProcessor* mp, CauseOfDeath) {
	signal_animation(mp->anims_, mp->delta_frame_);
}

void FlagGate::signal_animation(AnimationManager* anims, DeltaFrame* delta_frame) {
	if (parent_->tangible_ && !down_) {
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

void FlagGate::remove_walls(RoomMap* map, DeltaFrame* delta_frame) {
	Point3 a, b;
	get_gate_extremes(a, b);
	for (int x = a.x; x <= b.x; ++x) {
		for (int y = a.y; y <= b.y; ++y) {
			for (int z = a.z; z <= b.z; ++z) {
				map->take_from_map(map->view(Point3{ x, y, z }), true, false, delta_frame);
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
	case 16:
		*width = 5;
		*height = 5;
	case 32:
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
		scale_ = glm::vec3(width, 1, height);
		break;
	case 1:
		center_ = glm::vec3(0.0, (width - 1) / 2.0, 0.5 + height / 2.0);
		scale_ = glm::vec3(1, width, height);
		break;
	case 2:
		center_ = glm::vec3(- (width - 1) / 2.0, 0.0, 0.5 + height / 2.0);
		scale_ = glm::vec3(width, 1, height);
		break;
	case 3:
		center_ = glm::vec3(0.0, - (width - 1) / 2.0, 0.5 + height / 2.0);
		scale_ = glm::vec3(1, width, height);
		break;
	}
}

void FlagGate::spawn_sigils() {
	if (!sigils_.empty()) {
		return;
	}
	switch (num_flags_) {
	case 1:
		sigils_.push_back(FlagSigil{ center_, 0.0, 0, 1, 0, 1 });
		break;
	case 2:
		for (int i = 0; i < 2; ++i) {
			sigils_.push_back(FlagSigil{ center_, 0.5, 80 * i, 80 * 2, i, 2 });
		}
		break;
	case 4:
		for (int i = 0; i < 4; ++i) {
			sigils_.push_back(FlagSigil{ center_, 0.75, 60 * i, 60 * 4, i, 4 });
		}
		break;
	case 8:
		sigils_.push_back(FlagSigil{ center_, 0.0, 0, 1, 0, 8 });
		for (int i = 0; i < 7; ++i) {
			sigils_.push_back(FlagSigil{ center_, 1.0, 40 * i, 40 * 7, i+1, 8 });
		}
		break;
	case 32:
		for (int i = 0; i < 3; ++i) {
			sigils_.push_back(FlagSigil{ center_, 0.6, 80 * i, 80 * 3, i, 32 });
		}
		for (int i = 0; i < 11; ++i) {
			sigils_.push_back(FlagSigil{ center_, 1.5, 80 * i, 80 * 11, i+3, 32 });
		}
		for (int i = 0; i < 18; ++i) {
			sigils_.push_back(FlagSigil{ center_, 2.5, 80 * i, 80 * 18, i+14, 32 });
		}
		break;
	default:
		break;
	}
}

const int FLAG_GATE_MAX_TIME = 15840; // LCM of all relevant periods

bool FlagGate::update_animation(PlayingState* playing_state) {
	++animation_time_;
	if (animation_time_ == FLAG_GATE_MAX_TIME) {
		animation_time_ = 0;
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
	return false;
}

void FlagGate::reset_animation() {
	animation_time_ = 0;
}

void FlagGate::draw(GraphicsManager* gfx, FPoint3 p) {
	if (down_) {
		return;
	}
	gfx->cube.push_instance(glm::vec3(p) + center_, scale_, BlockTexture::Edges, parent_->color());
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
		sigil.draw(sigil_model, p, orientation_, animation_time_);
	}
}

const int MAX_FLAG_SIGIL_CHARGE = 30;
const int FLAG_SIGIL_DELAY = 8;

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
		pos += center + glm::vec3{ -radius*cos(t), 0.51, radius*sin(t) };
		break;
	case 1:
		pos += center + glm::vec3{ -0.51, -radius*cos(t), radius*sin(t) };
		break;
	case 2:
		pos += center + glm::vec3{ radius*cos(t), -0.51, radius*sin(t) };
		break;
	case 3:
		pos += center + glm::vec3{ 0.51, radius*cos(t), radius*sin(t) };
		break;
	}
	float value = std::min(std::max((charge - FLAG_SIGIL_DELAY * index), 0), MAX_FLAG_SIGIL_CHARGE) / (float)MAX_FLAG_SIGIL_CHARGE;
	glm::vec4 color = glm::vec4(0.6f, 0.6f, 0.6f, 1.0f) * glm::vec4(1 - value) + COLOR_VECTORS[GOLD] * glm::vec4(value);
	model->push_instance(pos, glm::vec3(0.6f), tex, color);
}
