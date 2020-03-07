#include "stdafx.h"
#include "savefile.h"

#include "common_constants.h"
#include "globalflagconstants.h"
#include "room.h"
#include "roommap.h"
#include "player.h"
#include "playingstate.h"
#include "mapfile.h"


GlobalData::GlobalData() {}

GlobalData::~GlobalData() {}


EditorGlobalData::EditorGlobalData() : GlobalData() {
	unsigned int seed = (unsigned int)std::chrono::system_clock::now().time_since_epoch().count();
	rand_engine_.seed(seed);
}

EditorGlobalData::~EditorGlobalData() {}

void EditorGlobalData::load_flags(std::filesystem::path path) {
	auto flag_file_path = path / "global.sav";
	if (std::filesystem::exists(flag_file_path)) {
		auto flag_file = MapFileI(flag_file_path);
		unsigned int num_flags = flag_file.read_uint32();
		for (unsigned int i = 0; i < num_flags; ++i) {
			unsigned int flag = flag_file.read_uint32();
			flags_[flag] = flag_file.read_str();
		}
	} else {
		// We have no file to read; start off with the *truly* global flags
		// These aren't bound to any particular room, and never change (this list should not get very long)
		flags_[0] = "";
		flags_[WORLD_RESET_GLOBAL_ID] = "";
	}
}

void EditorGlobalData::save_flags(std::filesystem::path path) {
	auto flag_file = MapFileO(path / "global.sav");
	flag_file.write_uint32((unsigned int)flags_.size());
	for (auto pair : flags_) {
		flag_file.write_uint32(pair.first);
		flag_file << pair.second;
	}
}

unsigned int EditorGlobalData::generate_flag() {
	unsigned int flag;
	do {
		flag = rand_engine_();
	} while (flags_.count(flag));
	return flag;
}

void EditorGlobalData::assign_flag(unsigned int flag, std::string room_name) {
	flags_[flag] = room_name;
}

void EditorGlobalData::destroy_flag(unsigned int flag) {
	flags_.erase(flag);
}


PlayingGlobalData::PlayingGlobalData() : GlobalData() {}

PlayingGlobalData::~PlayingGlobalData() {}

void PlayingGlobalData::load_flags(std::filesystem::path path) {
	auto flag_file_path = path / "global.sav";
	if (std::filesystem::exists(flag_file_path)) {
		flags_.clear();
		auto flag_file = MapFileI(flag_file_path);
		unsigned int num_flags = flag_file.read_uint32();
		for (unsigned int i = 0; i < num_flags; ++i) {
			flags_.insert(flag_file.read_uint32());
		}
		for (auto f : FLAG_COLLECT_FLAGS) {
			if (flags_.count(f)) {
				++clear_flag_total_;
			}
		}
	}
}

void PlayingGlobalData::save_flags(std::filesystem::path path) {
	auto flag_file = MapFileO(path / "global.sav");
	flag_file.write_uint32((unsigned int)flags_.size());
	for (unsigned int flag : flags_) {
		flag_file.write_uint32(flag);
	}
}

void PlayingGlobalData::add_flag(unsigned int flag) {
	flags_.insert(flag);
}

void PlayingGlobalData::remove_flag(unsigned int flag) {
	flags_.erase(flag);
}

bool PlayingGlobalData::has_flag(unsigned int flag) {
	return flags_.count(flag);
}

void PlayingGlobalData::collect_clear_flag(char zone) {
	flags_.insert(get_clear_flag_code(zone));
	++clear_flag_total_;
}

void PlayingGlobalData::uncollect_clear_flag(char zone) {
	flags_.erase(get_clear_flag_code(zone));
	--clear_flag_total_;
}


SaveFile::SaveFile(std::string base) :
	base_ {std::filesystem::path("saves") / base} {}

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
	global_->save_flags(subsave_path);
	save_meta();
}

void SaveFile::load_subsave(unsigned int subsave_index, std::string* cur_room_name) {
	cur_subsave_ = subsave_index;
	auto subsave_path = base_ / std::to_string(subsave_index);
	global_->load_flags(subsave_path);
	load_room_data(subsave_path, cur_room_name);
}

void SaveFile::load_most_recent_subsave(std::string* cur_room_name) {
	load_subsave(cur_subsave_, cur_room_name);
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

void SaveFile::load_room_data(std::filesystem::path subsave_path, std::string* cur_room_name) {
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

void SaveFile::save_room_data(std::filesystem::path subsave_path, std::string cur_room_name) {
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

std::filesystem::path SaveFile::get_path(std::string name, bool* from_main) {
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
	if (Player* active_player = room->map()->player_cycle_->current_player()) {
		file << MapCode::ActivePlayerPos;
		file << active_player->pos_;
	}
	file << MapCode::End;
}

void SaveFile::world_reset() {
	// "Forget" the locations of all room maps
	room_subsave_.clear();
}