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
	inacc_obj_list_.push_back(nullptr);
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

void GameObjectArray::uncreate_object(GameObject* obj) {
	auto id = obj->id_;
	free_ids_.push_back(id);
	array_[id].reset(nullptr);
}

bool GameObjectArray::add_inacc_obj(GameObject* obj, RoomMap* room_map) {
	if (inacc_obj_map_.count(obj)) {
		std::cout << obj << " was already added." << std::endl;
		return false;
	}
	unsigned int id;
	if (free_inacc_ids_.empty()) {
		id = (unsigned int)inacc_obj_list_.size();
		std::cout << obj << " added at " << id << std::endl;
		inacc_obj_list_.push_back(obj);
	} else {
		id = free_inacc_ids_.back();
		free_inacc_ids_.pop_back();
		std::cout << obj << " inserted at " << id << std::endl;
		inacc_obj_list_[id] = obj;
	}
	obj->inacc_id_ = id;
	inacc_obj_map_[obj] = id;
	obj_room_map_[obj] = room_map;
	return true;
}

void GameObjectArray::remove_inacc_obj(GameObject* obj) {
	auto id = inacc_obj_map_[obj];
	std::cout << obj << " removed from " << id << std::endl;
	inacc_obj_map_.erase(obj);
	obj_room_map_.erase(obj);
	free_inacc_ids_.push_back(id);
	inacc_obj_list_[id] = nullptr;
	obj->inacc_id_ = 0;
}

void GameObjectArray::update_parent_map(GameObject* obj, RoomMap* room_map) {
	if (obj_room_map_.count(obj)) {
		obj_room_map_[obj] = room_map;
	}
}

void GameObjectArray::serialize_object_ref(GameObject* obj, MapFileO& file) {
	FrozenObject ref{ obj };
	file << ref.ref_;
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
	file << ObjRefCode::Inaccessible;
	file << inacc_obj_map_[obj];
}

void GameObjectArray::check_room_inaccessibles(RoomMap* room_map) {
	auto name = room_map->name_;
	if (frozen_inaccessible_map_.count(name)) {
		for (auto& fp : frozen_inaccessible_map_[name]) {
			auto key = fp->inacc_id_;
			auto* obj = fp->resolve(room_map);
			inacc_obj_list_[key] = obj;
			obj->inacc_id_ = key;
			inacc_obj_map_[obj] = key;
			obj_room_map_[obj] = room_map;
		}
		frozen_inaccessible_map_.erase(name);
	}
}

void GameObjectArray::serialize_inacc_objs(MapFileO& file) {
	file.write_uint32((unsigned int)inacc_obj_list_.size());
	file.write_uint32((unsigned int)inacc_obj_map_.size());
	for (auto& p : inacc_obj_map_) {
		file.write_uint32(p.second);
		auto* obj = p.first;
		FrozenObject f{ obj };
		file << f.ref_;
		if (obj_room_map_.count(nullptr)) {
			std::cout << "wow!" << std::endl;
		}
		if (f.ref_ == ObjRefCode::Inaccessible) {
			std::cout << "Serializing FIO" << std::endl;
			file << obj->obj_code();
			file << obj->pos_;
			p.first->serialize(file);
			if (ObjectModifier* mod = obj->modifier()) {
				file << mod->mod_code();
				mod->serialize(file);
				if (mod->relation_check()) {
					mod->relation_serialize_frozen(file);
				}
			} else {
				file << ModCode::NONE;
				file << MapCode::NONE;
			}
		} else {
			std::cout << "Serializing ACCESSIBLE" << std::endl;
			if (!obj_room_map_.count(obj)) {
				std::cout << "none!" << std::endl;
			}
			file << f.pos_;
			file << obj_room_map_[p.first]->name_;
		}
	}
	file.write_uint32((unsigned int)frozen_inaccessible_map_.size());
	for (auto& p : frozen_inaccessible_map_) {
		file << p.first;
		file.write_uint32((unsigned int)p.second.size());
		for (auto& fio : p.second) {
			fio->serialize(file);
		}
	}
}
