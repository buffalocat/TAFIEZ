#include "stdafx.h"
#include "savefile.h"

#include "common_constants.h"
#include "globalflagconstants.h"
#include "room.h"
#include "roommap.h"
#include "player.h"
#include "car.h"
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
	auto flag_file_path = path / "used_flags.sav";
	if (std::filesystem::exists(flag_file_path)) {
		auto flag_file = MapFileI(flag_file_path);
		unsigned int num_flags = flag_file.read_uint32();
		for (unsigned int i = 0; i < num_flags; ++i) {
			unsigned int flag = flag_file.read_uint32();
			flags_[flag] = flag_file.read_str();
		}
	} else {
		// We have no file to read; start off with hardcoded global flags
		flags_[0] = "";
		for (auto f : FLAG_COLLECT_FLAGS) {
			flags_[f] = "";
		}
		for (auto f : ZONE_ACCESSED_GLOBAL_FLAGS) {
			flags_[f] = "";
		}
		for (auto f : HUB_ACCESSED_GLOBAL_FLAGS) {
			flags_[f] = "";
		}
		for (auto f : HUB_ALT_ACCESSED_GLOBAL_FLAGS) {
			flags_[f] = "";
		}
		for (auto f : X_ALT_ACCESSED_GLOBAL_FLAGS) {
			flags_[f] = "";
		}
		for (auto f : FATE_SIGNALER_CHOICE) {
			flags_[f] = "";
		}
		for (auto f : MISC_GLOBAL_FLAGS) {
			flags_[f] = "";
		}
		for (auto f : UNUSED_FLAGS) {
			flags_[f] = "";
		}
	}
}

void EditorGlobalData::save_flags(std::filesystem::path path) {
	auto flag_file = MapFileO(path / "used_flags.sav");
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
		clear_flag_total_ = 0;
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

void PlayingGlobalData::add_flag_delta(unsigned int flag, DeltaFrame* delta_frame) {
	if (!flags_.count(flag)) {
		flags_.insert(flag);
		if (delta_frame) {
			delta_frame->push(std::make_unique<GlobalFlagDelta>(this, flag));
		}
	}
}

void PlayingGlobalData::remove_flag(unsigned int flag) {
	flags_.erase(flag);
}

bool PlayingGlobalData::has_flag(unsigned int flag) {
	return flags_.count(flag);
}

void PlayingGlobalData::collect_clear_flag(char zone, DeltaFrame* delta_frame) {
	auto flag = get_clear_flag_code(zone);
	if (!flags_.count(flag)) {
		if (delta_frame) {
			delta_frame->push(std::make_unique<GlobalFlagDelta>(this, flag));
			delta_frame->push(std::make_unique<FlagCountDelta>(this, clear_flag_total_));
		}
		flags_.insert(get_clear_flag_code(zone));
		++clear_flag_total_;
	}
}

void PlayingGlobalData::uncollect_clear_flag(char zone) {
	flags_.erase(get_clear_flag_code(zone));
	--clear_flag_total_;
}


SaveFile::SaveFile(std::string base) :
	base_ {std::filesystem::path("saves") / base},
	global_{ std::make_unique<PlayingGlobalData>() } {}

SaveFile::~SaveFile() {}

void SaveFile::create_save_dir() {
	if (!std::filesystem::exists(base_)) {
		std::filesystem::create_directory(base_);
	}
}

void SaveFile::make_subsave(PlayingState* state, SaveType type) {
	int* save_index;
	switch (type) {
	case SaveType::Current:
		save_index = &cur_subsave_;
		break;
	case SaveType::Auto:
		save_index = &auto_subsave_;
		state->delta_frame_->push(std::make_unique<AutosaveDelta>(this, auto_subsave_));
		break;
	default:
		return;
	}
	*save_index = next_subsave_;
	++next_subsave_;
	std::filesystem::path subsave_path = base_ / std::to_string(*save_index);
	std::filesystem::create_directory(subsave_path);
	auto& loaded_rooms = state->loaded_rooms_;
	for (auto& p : loaded_rooms) {
		auto* proom = p.second.get();
		if (proom->changed) {
			save_room(proom->room.get(), subsave_path);
			proom->changed = false;
			room_subsave_[proom->room->name()] = *save_index;
		}
	}
	auto cur_room_name = state->active_room()->name();
	loaded_rooms[cur_room_name]->changed = true;
	save_room_data(subsave_path, cur_room_name);
	global_->save_flags(subsave_path);
	save_meta();
}

void SaveFile::load_subsave(unsigned int subsave_index) {
	cur_subsave_ = subsave_index;
	auto subsave_path = base_ / std::to_string(subsave_index);
	global_->load_flags(subsave_path);
	load_room_data(subsave_path);
}

void SaveFile::load_most_recent_subsave() {
	load_subsave(cur_subsave_);
}

void SaveFile::load_last_autosave() {
	load_subsave(auto_subsave_);
}

void SaveFile::load_meta() {
	auto meta_path = base_ / "meta.sav";
	if (!std::filesystem::exists(meta_path)) {
		return;
	}
	std::ifstream meta_file{};
	meta_file.open(meta_path, std::ios::in);
	meta_file >> cur_subsave_ >> next_subsave_ >> auto_subsave_;
	meta_file.close();
	exists_ = true;
	return;
}

void SaveFile::save_meta() {
	auto meta_path = base_ / "meta.sav";
	std::ofstream meta_file{};
	meta_file.open(meta_path, std::ios::out);
	meta_file << cur_subsave_ << " " << next_subsave_ << " " << auto_subsave_;
	meta_file.close();
}

void SaveFile::load_room_data(std::filesystem::path subsave_path) {
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
	room_data_file >> cur_room_name_;
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
	cur_room_name_ = cur_room_name;
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
		if (active_player->tangible_) {
			file << active_player->pos_;
		} else {
			file << active_player->car_riding()->pos();
		}
	}
	file << MapCode::End;
}

void SaveFile::world_reset() {
	// "Forget" the locations of all room maps
	room_subsave_.clear();
}


GlobalFlagDelta::GlobalFlagDelta(PlayingGlobalData* global, unsigned int flag) :
	Delta(), global_{ global }, flag_{ flag } {}

GlobalFlagDelta::~GlobalFlagDelta() {}

void GlobalFlagDelta::revert() {
	global_->remove_flag(flag_);
}


FlagCountDelta::FlagCountDelta(PlayingGlobalData* global, unsigned int count) :
	Delta(), global_{ global }, count_{ count } {}

FlagCountDelta::~FlagCountDelta() {}

void FlagCountDelta::revert() {
	global_->clear_flag_total_--;
}

AutosaveDelta::AutosaveDelta(SaveFile* savefile, int index) : Delta(),
	savefile_{ savefile }, index_{ index } {}

AutosaveDelta::~AutosaveDelta() {}

void AutosaveDelta::revert() {
	savefile_->auto_subsave_ = index_;
}