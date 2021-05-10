#include "stdafx.h"
#include "savefile.h"

#include <ctime>
#include <iomanip>
#include <sstream>

#include "common_constants.h"
#include "globalflagconstants.h"
#include "room.h"
#include "roommap.h"
#include "player.h"
#include "car.h"
#include "realplayingstate.h"
#include "mapfile.h"
#include "gameobjectarray.h"


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

bool PlayingGlobalData::has_flag(unsigned int flag) {
	return flags_.count(flag);
}

void PlayingGlobalData::collect_clear_flag(char zone) {
	auto flag = get_clear_flag_code(zone);
	if (!flags_.count(flag)) {
		flags_.insert(get_clear_flag_code(zone));
		++clear_flag_total_;
	}
}


const int NUM_SAVE_ROWS = 5;
const int NUM_SAVE_COLUMNS = 5;
const int NUM_AUTO_SAVES = NUM_SAVE_ROWS - 1;
const int NUM_MANUAL_SAVES = (NUM_SAVE_COLUMNS - 1) * (NUM_SAVE_ROWS - 1);


std::string get_current_timestamp() {
	auto t = std::time(nullptr);
	tm time_struct;
	localtime_s(&time_struct, &t);
	std::ostringstream oss;
	oss << std::put_time(&time_struct, "%H:%M:%S\n%b %d %Y");
	return oss.str();
}


SubSave::SubSave() {
	time_stamp_ = get_current_timestamp();
}

SubSave::~SubSave() {}

void SubSave::save_meta(MapFileO& file) {
	file.write_uint32(index_);
	file << zone_;
	file << time_stamp_;
	file.write_uint32((unsigned int)dependent_.size());
	for (auto i : dependent_) {
		file.write_uint32(i);
	}
}

void SubSave::load_meta(MapFileI& file, SaveDependencyGraph* dep) {
	index_ = file.read_uint32();
	zone_ = file.read_byte();
	time_stamp_ = file.read_str();
	auto n = file.read_uint32();
	for (unsigned int i = 0; i < n; ++i) {
		dependent_.insert(file.read_uint32());
	}
	dep->add_set(dependent_);
}


SubSaveAuto::SubSaveAuto(AutosavePanel* panel) : panel_{ panel } {}

SubSaveAuto::~SubSaveAuto() {}


LoadedSubSave::LoadedSubSave() {}

LoadedSubSave::~LoadedSubSave() {}


SaveDependencyGraph::SaveDependencyGraph() {}

SaveDependencyGraph::~SaveDependencyGraph() {}

void SaveDependencyGraph::add_set(std::set<unsigned int>& dep) {
	for (auto& idx : dep) {
		if (dep_count_.count(idx)) {
			dep_count_[idx] += 1;
		} else {
			dep_count_[idx] = 1;
		}
	}
}

void SaveDependencyGraph::remove_set(std::set<unsigned int>& dep) {
	for (auto idx : dep) {
		dep_count_[idx] -= 1;
	}
}

std::vector<unsigned int> SaveDependencyGraph::clear_unused_saves() {
	std::vector<unsigned int> unused{};
	for (auto& p : dep_count_) {
		if (p.second == 0) {
			unused.push_back(p.first);
		}
	}
	for (auto i : unused) {
		dep_count_.erase(i);
	}
	return unused;
}


SaveProfile::SaveProfile(std::string profile_name) :
	base_path_{ std::filesystem::path("saves") / profile_name },
	global_{ std::make_unique<PlayingGlobalData>() } {
	for (int i = 0; i < NUM_AUTO_SAVES; ++i) {
		auto_saves_.push_back(nullptr);
	}
	for (int i = 0; i < NUM_MANUAL_SAVES; ++i) {
		manual_saves_.push_back(nullptr);
	}
}

SaveProfile::~SaveProfile() {}

bool SaveProfile::overwrite_current_autosave(AutosavePanel* panel) {
	if (auto* cur_auto_save = auto_saves_[0].get()) {
		return cur_auto_save->panel_ == panel;
	}
	return false;
}

void SaveProfile::make_emergency_save(RealPlayingState* state) {
	remove_save(emergency_save_.get());
	emergency_save_ = std::make_unique<SubSave>();
	make_subsave(emergency_save_.get(), state);
}

void SaveProfile::make_auto_save(AutosavePanel* panel, RealPlayingState* state) {
	if (overwrite_current_autosave(panel)) {
		remove_save(auto_saves_.front().get());
		auto_saves_.pop_front();
	} else {
		remove_save(auto_saves_.back().get());
		auto_saves_.pop_back();
	}
	auto new_save_unique = std::make_unique<SubSaveAuto>(panel);
	auto* new_save = new_save_unique.get();
	auto_saves_.push_front(std::move(new_save_unique));
	make_subsave(new_save, state);
}

void SaveProfile::make_manual_save(unsigned int index, RealPlayingState* state) {
	if (auto* old_save = manual_saves_[index].get()) {
		remove_save(old_save);
	}
	auto new_save_unique = std::make_unique<SubSave>();
	auto* new_save = new_save_unique.get();
	manual_saves_[index] = std::move(new_save_unique);
	make_subsave(new_save, state);
}

unsigned int SaveProfile::get_save_index() {
	unsigned int index;
	if (unused_subsave_indices_.empty()) {
		index = next_index_;
		++next_index_;
	} else {
		index = unused_subsave_indices_.back();
		unused_subsave_indices_.pop_back();
	}
	return index;
}


void SaveProfile::make_subsave(SubSave* subsave, RealPlayingState* state) {
	unsigned int index = get_save_index();
	subsave->index_ = index;
	std::filesystem::path subsave_path = base_path_ / std::to_string(index);
	std::filesystem::create_directory(subsave_path);
	auto& loaded_rooms = state->loaded_rooms_;
	auto& room_subsave = loaded_subsave_map_[state].room_subsave_;
	for (auto& p : loaded_rooms) {
		auto* proom = p.second.get();
		if (proom->changed) {
			save_room(proom->room.get(), subsave_path);
			proom->changed = false;
			room_subsave[proom->room->name()] = index;
		}
	}
	for (auto& p : room_subsave) {
		subsave->dependent_.insert(p.second);
	}
	subsave->zone_ = state->active_room()->map()->zone_;
	auto cur_room_name = state->active_room()->name();
	loaded_rooms[cur_room_name]->changed = true;
	save_room_data(state, subsave_path, cur_room_name);
	global_->save_flags(subsave_path);
	MapFileO objs_file{ subsave_path / "objs.sav" };
	state->objs_->serialize(objs_file);
	state->undo_stack_->serialize(subsave_path, index);
	for (auto i : state->undo_stack_->dependent_subsaves()) {
		subsave->dependent_.insert(i);
	}
	save_meta();
	global_->save_flags(base_path_);
	dep_graph_.add_set(subsave->dependent_);
}

void SaveProfile::create_save_dir() {
	if (!std::filesystem::exists(base_path_)) {
		std::filesystem::create_directory(base_path_);
	}
}

bool SaveProfile::load_subsave_dispatch(SaveType type, unsigned int index, RealPlayingState* state) {
	SubSave* subsave{};
	switch (type) {
	case SaveType::Emergency:
		subsave = emergency_save_.get();
		break;
	case SaveType::Auto:
		subsave = auto_saves_[index].get();
		break;
	case SaveType::Manual:
		subsave = manual_saves_[index].get();
		break;
	}
	if (subsave) {
		load_subsave(subsave, state);
		return true;
	} else {
		return false;
	}
}

void SaveProfile::load_subsave(SubSave* subsave, RealPlayingState* state) {
	unsigned int subsave_index = subsave->index_;
	auto subsave_path = base_path_ / std::to_string(subsave_index);
	auto objs_path = subsave_path / "objs.sav";
	MapFileI objs_file{ objs_path };
	state->objs_->deserialize(objs_file);
	state->undo_stack_->deserialize(base_path_, subsave_index);
	auto* loaded = &loaded_subsave_map_[state];
	load_room_data(loaded, subsave_path);
	loaded->dependent_ = subsave->dependent_;
	dep_graph_.add_set(loaded->dependent_);
}

void SaveProfile::load_global() {
	global_->load_flags(base_path_);
}

void SaveProfile::remove_save(SubSave* subsave) {
	if (subsave) {
		dep_graph_.remove_set(subsave->dependent_);
	}
}

void SaveProfile::delete_unused_saves() {
	return;
	for (auto index : dep_graph_.clear_unused_saves()) {
		unused_subsave_indices_.push_back(index);
		std::filesystem::remove_all(base_path_ / std::to_string(index));
	}
}


void SaveProfile::load_room_data(LoadedSubSave* loaded, std::filesystem::path subsave_path) {
	auto room_data_path = subsave_path / "rooms.sav";
	std::ifstream room_data_file{};
	room_data_file.open(room_data_path, std::ios::in);
	unsigned int number_pairs;
	room_data_file >> number_pairs;
	auto& room_subsave = loaded->room_subsave_;
	for (unsigned int i = 0; i < number_pairs; ++i) {
		std::string room_name;
		unsigned int subsave_index;
		room_data_file >> room_name >> subsave_index;
		room_subsave[room_name] = subsave_index;
		loaded->dependent_.insert(subsave_index);
	}
	room_data_file >> cur_room_name_;
	room_data_file.close();
}

void SaveProfile::save_room_data(RealPlayingState* state, std::filesystem::path subsave_path, std::string cur_room_name) {
	auto room_data_path = subsave_path / "rooms.sav";
	std::ofstream room_data_file{};
	room_data_file.open(room_data_path, std::ios::out);
	auto room_subsave = loaded_subsave_map_[state].room_subsave_;
	room_data_file << (unsigned int)room_subsave.size() << " ";
	for (auto& p : room_subsave) {
		room_data_file << p.first << " " << p.second << " ";
	}
	cur_room_name_ = cur_room_name;
	room_data_file << cur_room_name;
	room_data_file.close();
}

std::filesystem::path SaveProfile::get_path(RealPlayingState* state, std::string name, bool* from_main) {
	auto room_subsave = loaded_subsave_map_[state].room_subsave_;
	if (room_subsave.count(name)) {
		*from_main = false;
		return (base_path_ / std::to_string(room_subsave[name]) / name).concat(".map");
	} else {
		*from_main = true;
		return (std::filesystem::path("maps") / "main" / name).concat(".map");
	}
}

void SaveProfile::save_room(Room* room, std::filesystem::path path) {
	MapFileO file{ (path / room->name()).concat(".map") };
	room->write_to_file(file, true);
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

void SaveProfile::init_state(RealPlayingState* state) {
	loaded_subsave_map_[state] = LoadedSubSave{};
}

void SaveProfile::unload_state(RealPlayingState* state) {
	dep_graph_.remove_set(loaded_subsave_map_[state].dependent_);
	loaded_subsave_map_.erase(state);
	for (unsigned int i : dep_graph_.clear_unused_saves()) {
		std::error_code e{};
		auto subsave_path = base_path_ / std::to_string(i);
		std::filesystem::remove_all(subsave_path, e);
	}
}


void SaveProfile::load_meta() {
	auto meta_path = base_path_ / "meta.sav";
	if (!std::filesystem::exists(meta_path)) {
		return;
	}
	MapFileI meta_file{ meta_path };
	if (meta_file.read_byte()) {
		emergency_save_ = std::make_unique<SubSave>();
		emergency_save_->load_meta(meta_file, &dep_graph_);
	}
	for (auto& auto_save : auto_saves_) {
		if (meta_file.read_byte()) {
			auto_save = std::make_unique<SubSaveAuto>(nullptr);
			auto_save->load_meta(meta_file, &dep_graph_);
		}
	}
	for (auto& manual_save : manual_saves_) {
		if (meta_file.read_byte()) {
			manual_save = std::make_unique<SubSave>();
			manual_save->load_meta(meta_file, &dep_graph_);
		}
	}
	next_index_ = meta_file.read_uint32();
	unsigned int num_unused = meta_file.read_uint32();
	for (unsigned int i = 0; i < num_unused; ++i) {
		unused_subsave_indices_.push_back(meta_file.read_uint32());
	}
	exists_ = true;
	return;
}


void SaveProfile::save_meta() {
	auto meta_path = base_path_ / "meta.sav";
	MapFileO meta_file{ meta_path };
	save_meta_single(meta_file, emergency_save_.get());
	for (auto& auto_save : auto_saves_) {
		save_meta_single(meta_file, auto_save.get());
	}
	for (auto& manual_save : manual_saves_) {
		save_meta_single(meta_file, manual_save.get());
	}
	meta_file.write_uint32(next_index_);
	meta_file.write_uint32((unsigned int)unused_subsave_indices_.size());
	for (auto i : unused_subsave_indices_) {
		meta_file.write_uint32(i);
	}
}

void SaveProfile::save_meta_single(MapFileO& file, SubSave* subsave) {
	if (subsave) {
		file << 1;
		subsave->save_meta(file);
	} else {
		file << 0;
	}
}
