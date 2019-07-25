#include "stdafx.h"
#include "savefile.h"

#include "room.h"
#include "playingstate.h"
#include <fstream>
#include "mapfile.h"

SaveFile::SaveFile(const std::string& base) :
	base_ {std::filesystem::path("saves") / base},
	room_subsave_{},
	global_{},
	cur_subsave_{ 0 },
	next_subsave_{ 1 } {}

SaveFile::~SaveFile() {}

bool SaveFile::create() {
	if (std::filesystem::exists(base_)) {
		return false;
	} else {
		std::filesystem::create_directory(base_);
		return true;
	}
}

void SaveFile::make_subsave(std::map<std::string, std::unique_ptr<PlayingRoom>>& loaded_rooms, std::string const& cur_room_name) {
	cur_subsave_ = next_subsave_;
	++next_subsave_;
	std::filesystem::path subsave_path = base_ / std::to_string(cur_subsave_);
	std::filesystem::create_directory(subsave_path);
	for (auto& p : loaded_rooms) {
		auto* proom = p.second.get();
		if (proom->changed) {
			save_room(proom->room.get(), subsave_path);
			proom->changed = false;
			room_subsave_[proom->room->name()] = cur_subsave_;
		}
	}
	loaded_rooms[cur_room_name]->changed = true;
	save_room_data(subsave_path, cur_room_name);
	save_meta();
}

void SaveFile::load_most_recent_subsave(std::string* cur_room_name) {
	load_room_data(base_ / std::to_string(cur_subsave_), cur_room_name);
}


bool SaveFile::load_meta() {
	auto meta_path = base_ / "meta.sav";
	if (!std::filesystem::exists(meta_path)) {
		return false;
	}
	std::ifstream meta_file{};
	meta_file.open(meta_path, std::ios::in);
	meta_file >> cur_subsave_ >> next_subsave_;
	meta_file.close();
	return true;
}

void SaveFile::save_meta() {
	auto meta_path = base_ / "meta.sav";
	std::ofstream meta_file{};
	meta_file.open(meta_path, std::ios::out);
	meta_file << cur_subsave_ << " " << next_subsave_;
	meta_file.close();
}

void SaveFile::load_room_data(std::filesystem::path const& subsave_path, std::string* cur_room_name) {
	auto room_data_path = subsave_path / "rooms.sav";
	std::ifstream room_data_file{};
	room_data_file.open(room_data_path, std::ios::in);
	unsigned int number_pairs;
	room_data_file >> number_pairs;
	for (unsigned int i = 0; i < number_pairs; ++i) {
		std::string room_name;
		unsigned int subsave_index;
		room_data_file >> room_name >> subsave_index;
		room_subsave_[room_name] = subsave_index;
	}
	room_data_file >> *cur_room_name;
	room_data_file.close();
}

void SaveFile::save_room_data(std::filesystem::path const& subsave_path, std::string const& cur_room_name) {
	auto room_data_path = subsave_path / "rooms.sav";
	std::ofstream room_data_file{};
	room_data_file.open(room_data_path, std::ios::out);
	room_data_file << (unsigned int)room_subsave_.size() << " ";
	for (auto& p : room_subsave_) {
		room_data_file << p.first << " " << p.second << " ";
	}
	room_data_file << cur_room_name;
	room_data_file.close();
}

std::filesystem::path SaveFile::get_path(std::string const& name, bool* from_main) {
	if (room_subsave_.count(name)) {
		*from_main = false;
		return (base_ / std::to_string(room_subsave_[name]) / name).concat(".map");
	} else {
		*from_main = true;
		return (std::filesystem::path("maps") / "main" / name).concat(".map");
	}
}

void SaveFile::save_room(Room* room, std::filesystem::path path) {
	MapFileO file{ (path / room->name()).concat(".map") };
	room->write_to_file(file);
}