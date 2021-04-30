#include "stdafx.h"
#include "gameobjectarray.h"

#include "wall.h"
#include "objectmodifier.h"
#include "mapfile.h"
#include "room.h"
#include "roommap.h"
#include "gatebody.h"
#include "gate.h"
#include "player.h"
#include "car.h"

GameObjectArray::GameObjectArray() {
    array_.push_back(nullptr);
    array_.push_back(std::make_unique<Wall>());
    array_[1]->id_ = 1;
}

GameObjectArray::~GameObjectArray() {}

void GameObjectArray::push_object(std::unique_ptr<GameObject> obj) {
	if (free_ids_.empty()) {
		obj->id_ = (unsigned int)array_.size();
		array_.push_back(std::move(obj));
	} else {
		auto id = free_ids_.back();
		free_ids_.pop_back();
		obj->id_ = id;
		array_[id] = std::move(obj);
	}
}

GameObject* GameObjectArray::operator[](unsigned int id) const {
    return array_[id].get();
}

GameObject* GameObjectArray::safe_get(unsigned int id) const {
    if (id >= array_.size()) {
        return nullptr;
    } else {
        return array_[id].get();
    }
}

void GameObjectArray::schedule_uncreation(GameObject* obj) {
	to_uncreate_.push_back(obj);
}

void GameObjectArray::schedule_undeletion(GameObject* obj) {
	to_undelete_.push_back(obj);
}

void GameObjectArray::remove_uncreated_objects() {
	for (auto* obj : to_uncreate_) {
		auto id = obj->id_;
		free_ids_.push_back(id);
		array_[id].reset(nullptr);
	}
	to_uncreate_.clear();
}

void GameObjectArray::remove_undeleted_objects() {
	for (auto* obj : to_undelete_) {
		auto id = dead_obj_map_[obj];
		free_dead_ids_.push_back(id);
		dead_obj_list_[id] = nullptr;
	}
	to_undelete_.clear();
}

unsigned int GameObjectArray::add_dead_obj(GameObject* obj) {
	if (!obj) {
		free_dead_ids_.push_back((unsigned int)dead_obj_list_.size());
		dead_obj_list_.push_back(nullptr);
	}
	if (free_dead_ids_.empty()) {
		auto id = (unsigned int)dead_obj_list_.size();
		dead_obj_list_.push_back(obj);
		dead_obj_map_[obj] = id;
		return id;
	} else {
		auto id = free_ids_.back();
		free_ids_.pop_back();
		dead_obj_list_[id] = obj;
		dead_obj_map_[obj] = id;
		return id;
	}
}

void GameObjectArray::serialize_object_ref(GameObject* obj, MapFileO& file) {
	FrozenObject ref{ obj };
	file << ref.ref_;
	switch (ref.ref_) {
	case ObjRefCode::Tangible:
	{
		file << obj->pos_;
	}
	}
	if (obj->tangible_) {
		file << ObjRefCode::Tangible;
		file << obj->pos_;
		return;
	}
	if (auto* gate_body = dynamic_cast<GateBody*>(obj)) {
		if (auto* gate = gate_body->gate_) {
			if (gate->body_ == gate_body) {
				file << ObjRefCode::HeldGateBody;
				file << gate->pos();
				return;
			}
		}
	}
	if (auto* player = dynamic_cast<Player*>(obj)) {
		if (auto* car = player->car_) {
			if (car->player_ == player) {
				file << ObjRefCode::HeldPlayer;
				file << car->pos();
				return;
			}
		}
	}
	file << ObjRefCode::Dead;
	file << dead_obj_map_[obj];
}

void GameObjectArray::serialize_dead_objs(MapFileO& file) {
	for (auto* obj : dead_obj_list_) {
		if (obj) {
			obj->serialize(file);
		} else {
			file << ObjCode::Null;
		}
	}
	file << ObjCode::NONE;
}

void GameObjectArray::deserialize_dead_objs(MapFileI& file) {
	DeadObjectAdder fake_room{ *this };
	read_objects_free(file, &fake_room);
	for (auto* obj : dead_obj_list_) {
		if (obj->tangible_) {
			obj->tangible_ = false;
		}
	}
}
