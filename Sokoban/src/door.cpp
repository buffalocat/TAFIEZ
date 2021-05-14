#include "stdafx.h"
#include "door.h"

#include "mapfile.h"
#include "gameobject.h"
#include "graphicsmanager.h"
#include "animationmanager.h"

#include "texture_constants.h"

#include "roommap.h"
#include "moveprocessor.h"
#include "savefile.h"

DoorData::DoorData(std::string start_room, std::string dest_room, unsigned int door_id) :
	start{ start_room }, dest{ dest_room }, id{ door_id } {}

Door::Door(GameObject* parent, int count, bool persistent, bool def, bool active, unsigned int door_id):
	Switchable(parent, count, persistent, def, active, false), door_id_{ door_id } {}

Door::~Door() {}

Door::Door(const Door& d) : Switchable(d.parent_, d.count_, d.persistent_, d.default_, d.active_, d.waiting_), data_{}, door_id_{ d.door_id_ } {}

void Door::make_str(std::string& str) {
	char buf[64];
	if (data_) {
		snprintf(buf, 64, "Door:(%d)\"%s\"[%d]", door_id_, data_->dest.c_str(), data_->id);
	} else {
		snprintf(buf, 64, "Door:(%d)[NONE]", door_id_);
	}
	str += buf;
	Switchable::make_str(str);
}

ModCode Door::mod_code() {
    return ModCode::Door;
}

void Door::set_data(unsigned int door_id, std::string start, std::string dest) {
    data_ = std::make_unique<DoorData>(start, dest, door_id);
}

void Door::reset_data() {
	data_.reset(nullptr);
}

DoorData* Door::data() {
    return data_.get();
}

bool Door::usable() {
	return state() && data_;
}

void Door::serialize(MapFileO& file) {
    file << count_ << persistent_ << default_ << active_;
	file.write_uint32(door_id_);
}

void Door::deserialize(MapFileI& file, RoomMap*, GameObject* parent) {
    unsigned char b[4];
    file.read(b, 4);
	unsigned int id = file.read_uint32();
    parent->set_modifier(std::make_unique<Door>(parent, b[0], b[1], b[2], b[3], id));
}

bool Door::relation_check() {
	return true;
}

void Door::relation_serialize(MapFileO& file) {
	if ((data_ != nullptr) && (data_->id > 0)) {
		file << MapCode::DoorDest;
		file << pos();
		file.write_uint32(data_->id);
		if (data_->dest == data_->start) {
			file << std::string{};
		} else {
			file << data_->dest;
		}
	}
	if (map_flag_ != 0) {
		file << MapCode::DoorFlag;
		file << pos();
		file.write_uint32(map_flag_);
	}
}

void Door::relation_serialize_frozen(MapFileO& file) {
	file << MapCode::DoorRelationsFrozen;
	if ((data_ != nullptr) && (data_->id > 0)) {
		file << 1;
		file.write_uint32(data_->id);
		file << data_->start;
		file << data_->dest;
	} else {
		file << 0;
	}
	if (map_flag_ != 0) {
		file << 1;
		file.write_uint32(map_flag_);
	} else {
		file << 0;
	}
}

bool Door::can_set_state(bool state, RoomMap* map) {
    return true;
}

void Door::map_callback(RoomMap*, DeltaFrame*, MoveProcessor* mp) {
	if (parent_->tangible_) {
		mp->plan_door_move(this);
	}
}

void Door::apply_state_change(RoomMap*, DeltaFrame* delta_frame, MoveProcessor* mp) {
	signal_animation(mp->anims_, mp->delta_frame_);
}

void Door::setup_on_put(RoomMap* map, DeltaFrame* delta_frame, bool real) {
	Switchable::setup_on_put(map, delta_frame, real);
    map->add_listener(this, pos_above());
    map->activate_listener_of(this);
	if (real) {
		map->add_door(this);
	}
}

void Door::cleanup_on_take(RoomMap* map, DeltaFrame* delta_frame, bool real) {
	Switchable::cleanup_on_take(map, delta_frame, real);
    map->remove_listener(this, pos_above());
	if (real) {
		map->remove_door(this);
	}
}

void Door::destroy(MoveProcessor* mp, CauseOfDeath) {
	signal_animation(mp->anims_, mp->delta_frame_);
}

void Door::signal_animation(AnimationManager* anims, DeltaFrame* delta_frame) {
	if (parent_->tangible_ && state() && data_) {
		anims->receive_signal(AnimationSignal::DoorOn, parent_, delta_frame);
	} else {
		anims->receive_signal(AnimationSignal::DoorOff, parent_, delta_frame);
	}
}

void Door::draw(GraphicsManager* gfx, FPoint3 p) {
	int color = data_ ? (state() ? GREEN : RED) : WHITE;
	ModelInstancer& model = parent_->is_snake() ? gfx->top_diamond : gfx->top_cube;
	model.push_instance(glm::vec3(p.x, p.y, p.z + 0.5f), glm::vec3(0.7f, 0.7f, 0.1f), BlockTexture::Door, color);
}

std::unique_ptr<ObjectModifier> Door::duplicate(GameObject* parent, RoomMap*, DeltaFrame*) {
    auto dup = std::make_unique<Door>(*this);
    dup->parent_ = parent;
	if (data_) {
		dup->data_ = std::make_unique<DoorData>(*data_);
	}
    return std::move(dup);
}

void Door::acquire_map_flag(PlayingGlobalData* global) {
	if (map_flag_) {
		global->add_flag(map_flag_);
	}
}