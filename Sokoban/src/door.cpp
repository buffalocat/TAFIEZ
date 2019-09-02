#include "stdafx.h"
#include "door.h"

#include "mapfile.h"
#include "gameobject.h"
#include "graphicsmanager.h"

#include "texture_constants.h"

#include "roommap.h"
#include "moveprocessor.h"

DoorData::DoorData(Point3_S16 p, std::string start_room, std::string dest_room) : pos{ p }, start{ start_room }, dest{ dest_room } {}

Door::Door(GameObject* parent, int count, bool persistent, bool def, bool active):
	Switchable(parent, count, persistent, def, active, false), data_ {} {}

Door::~Door() {}

Door::Door(const Door& d): Switchable(d.parent_, d.count_, d.persistent_, d.default_, d.active_, d.waiting_), data_{} {}

void Door::make_str(std::string& str) {
	char buf[64];
	if (data_) {
		Point3_S16 p = data_->pos;
		snprintf(buf, 64, "Door:(%d,%d,%d):\"%s\"", p.x, p.y, p.z, data_->dest.c_str());
	} else {
		snprintf(buf, 64, "Door:NO DEST");
	}
	str += buf;
	Switchable::make_str(str);
}

ModCode Door::mod_code() {
    return ModCode::Door;
}

void Door::set_data(Point3_S16 pos, std::string start, std::string dest) {
    data_ = std::make_unique<DoorData>(pos, start, dest);
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
}

void Door::deserialize(MapFileI& file, RoomMap*, GameObject* parent) {
    unsigned char b[4];
    file.read(b, 4);
    parent->set_modifier(std::make_unique<Door>(parent, b[0], b[1], b[2], b[3]));
}

bool Door::relation_check() {
    return data_ != nullptr;
}

void Door::relation_serialize(MapFileO& file) {
    file << MapCode::DoorDest;
    file << parent_->pos_;
    file << data_->pos;
	if (data_->dest == data_->start) {
		file << std::string{};
	} else {
		file << data_->dest;
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

void Door::setup_on_put(RoomMap* map, bool real) {
	Switchable::setup_on_put(map, real);
    map->add_listener(this, pos_above());
    map->activate_listener_of(this);
}

void Door::cleanup_on_take(RoomMap* map, bool real) {
	Switchable::cleanup_on_take(map, real);
    map->remove_listener(this, pos_above());
}

void Door::draw(GraphicsManager* gfx, FPoint3 p) {
	int color = (data_ && state()) ? GREEN : DARK_RED;
	ModelInstancer& model = parent_->is_snake() ? gfx->top_diamond : gfx->top_cube;
	model.push_instance(glm::vec3(p.x, p.y, p.z + 0.5f), glm::vec3(0.9f, 0.9f, 0.1f), BlockTexture::Door, color);
}

std::unique_ptr<ObjectModifier> Door::duplicate(GameObject* parent, RoomMap*, DeltaFrame*) {
    auto dup = std::make_unique<Door>(*this);
    dup->parent_ = parent;
    dup->data_ = std::make_unique<DoorData>(*data_);
    return std::move(dup);
}
