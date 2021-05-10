#pragma once

#include <random>
#include "delta.h"

class PlayingState;
class RealPlayingState;
struct PlayingRoom;
class Room;
class MapFileI;
class MapFileO;

enum class SaveType {
	Emergency,
	Auto,
	Manual,
};

class GlobalData {
public:
	GlobalData();
	virtual ~GlobalData();

	virtual void load_flags(std::filesystem::path path) = 0;
	virtual void save_flags(std::filesystem::path path) = 0;
};

class EditorGlobalData : public GlobalData {
public:
	EditorGlobalData();
	~EditorGlobalData();

	void load_flags(std::filesystem::path path);
	void save_flags(std::filesystem::path path);
	unsigned int generate_flag();
	void assign_flag(unsigned int flag, std::string room_name);
	void destroy_flag(unsigned int flag);

private:
	std::map<unsigned int, std::string> flags_{};
	std::default_random_engine rand_engine_{};
};

class PlayingGlobalData : public GlobalData {
public:
	PlayingGlobalData();
	~PlayingGlobalData();

	void load_flags(std::filesystem::path path);
	void save_flags(std::filesystem::path path);
	void add_flag(unsigned int flag);
	bool has_flag(unsigned int flag);

	void collect_clear_flag(char zone);

	std::set<unsigned int> flags_{};
	int clear_flag_total_ = 0;
};


class SaveDependencyGraph {
public:
	SaveDependencyGraph();
	~SaveDependencyGraph();

	void add_set(std::set<unsigned int>& dep);
	void remove_set(std::set<unsigned int>& dep);
	std::vector<unsigned int> clear_unused_saves();

private:
	std::map<unsigned int, unsigned int> dep_count_{};
};


extern const int NUM_SAVE_ROWS;
extern const int NUM_SAVE_COLUMNS;
extern const int NUM_AUTO_SAVES;
extern const int NUM_MANUAL_SAVES;

class AutosavePanel;

class SubSave {
public:
	SubSave();
	virtual ~SubSave();

	void save_meta(MapFileO& file);
	void load_meta(MapFileI& file, SaveDependencyGraph* dep);

	unsigned int index_;
	std::set<unsigned int> dependent_{};
	std::string time_stamp_{};
	char zone_{};
};


class SubSaveAuto : public SubSave {
public:
	SubSaveAuto(AutosavePanel* panel);
	~SubSaveAuto();

	AutosavePanel* panel_{};
};


class LoadedSubSave {
public:
	LoadedSubSave();
	~LoadedSubSave();

	std::set<unsigned int> dependent_{};
	std::map<std::string, unsigned int> room_subsave_{};
};


class SaveProfile {
public:
	SaveProfile(std::string profile_name);
	~SaveProfile();

	unsigned int get_save_index();
	void make_subsave(SubSave*, RealPlayingState*);
	void load_subsave(SubSave*, RealPlayingState*);
	void load_global();
	void remove_save(SubSave*);
	void delete_unused_saves();
	void create_save_dir();

	bool overwrite_current_autosave(AutosavePanel*);
	void make_emergency_save(RealPlayingState*);
	void make_auto_save(AutosavePanel*, RealPlayingState*);
	void make_manual_save(unsigned int index, RealPlayingState*);
	bool load_subsave_dispatch(SaveType type, unsigned int index, RealPlayingState* state);

	void load_room_data(LoadedSubSave* loaded, std::filesystem::path subsave_path);
	void save_room_data(RealPlayingState* state, std::filesystem::path subsave_path, std::string cur_room_name);
	std::filesystem::path get_path(RealPlayingState* state, std::string name, bool* from_main);
	void save_room(Room* room, std::filesystem::path path);

	void init_state(RealPlayingState* state);
	void unload_state(RealPlayingState* state);

	void load_meta();
	void save_meta();
	void save_meta_single(MapFileO& file, SubSave* subsave);

	std::unique_ptr<PlayingGlobalData> global_;
	std::string cur_room_name_{};
	bool exists_ = false;

	std::map<RealPlayingState*, LoadedSubSave> loaded_subsave_map_{};

	std::unique_ptr<SubSave> emergency_save_{};
	std::deque<std::unique_ptr<SubSaveAuto>> auto_saves_{};
	std::vector<std::unique_ptr<SubSave>> manual_saves_{};
	SaveDependencyGraph dep_graph_{};

private:
	unsigned int next_index_{ 0 };
	std::vector<unsigned int> unused_subsave_indices_{};
	std::filesystem::path base_path_;
};