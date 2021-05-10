#include "stdafx.h"
#include "gameobjectarray.h"

#include "wall.h"
#include "objectmodifier.h"
#include "mapfile.h"
#include "room.h"
#include "roommap.h"

GameObjectArray::GameObjectArray() {
	array_.push_back(nullptr);
	array_.push_back(std::make_unique<Wall>());
	array_[1]->id_ = 1;
}

GameObjectArray::~GameObjectArray() {}

void GameObjectArray::push_object(std::unique_ptr<GameObject> obj) {
	if (auto id = obj->id_) {
		array_[id] = std::move(obj);
	} else {
		if (free_ids_.empty()) {
			obj->id_ = (unsigned int)array_.size();
			array_.push_back(std::move(obj));
		} else {
			id = free_ids_.back();
			free_ids_.pop_back();
			obj->id_ = id;
			array_[id] = std::move(obj);
		}
	}
}

GameObject* GameObjectArray::operator[](unsigned int id) const {
	return array_[id].get();
}

void GameObjectArray::uncreate_object(GameObject* obj) {
	auto id = obj->id_;
	free_ids_.push_back(id);
	array_[id].reset(nullptr);
}

void GameObjectArray::add_dead_obj(GameObject* obj) {
	dead_obj_list_.push_back(obj);
}

void GameObjectArray::remove_dead_obj(GameObject* obj) {
	dead_obj_list_.pop_back();
}

void GameObjectArray::serialize(MapFileO& file) {
	file.write_uint32((unsigned int)array_.size());
	file.write_uint32((unsigned int)free_ids_.size());
	for (auto i : free_ids_) {
		file.write_uint32(i);
	}

	file.write_uint32((unsigned int)dead_obj_list_.size());
	for (auto* obj : dead_obj_list_) {
		file << obj->obj_code();
		file.write_uint32(obj->id_);
		file.write_spoint3(obj->pos_);
		obj->serialize(file);
		if (ObjectModifier* mod = obj->modifier()) {
			file << mod->mod_code();
			mod->serialize(file);
			mod->relation_serialize_frozen(file);
		} else {
			file << ModCode::NONE;
			file << MapCode::NONE;
		}
	}
}


void GameObjectArray::deserialize(MapFileI& file) {
	auto size = file.read_uint32();
	array_.resize(size);
	auto num_free = file.read_uint32();
	for (unsigned int i = 0; i < num_free; ++i) {
		free_ids_.push_back(file.read_uint32());
	}
	deserialize_dead_objects(file, this);
}