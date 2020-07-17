#pragma once

#include <random>
#include "delta.h"

struct PlayingRoom;
class Room;

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
	void add_flag_delta(unsigned int flag, DeltaFrame*);
	void remove_flag(unsigned int flag);
	bool has_flag(unsigned int flag);

	void collect_clear_flag(char zone);
	void uncollect_clear_flag(char zone);

	std::set<unsigned int> flags_{};
	int clear_flag_total_ = 0;
};

class SaveFile {
public:
	SaveFile(std::string base);
	~SaveFile();

	void create_save_dir();
	void load_meta();
	std::filesystem::path get_path(std::string, bool* from_main);
	void make_subsave(std::map<std::string, std::unique_ptr<PlayingRoom>>& loaded_rooms, std::string const& cur_room_name);
	void load_subsave(unsigned int subsave_index);
	void load_most_recent_subsave();

	void world_reset();

	std::unique_ptr<PlayingGlobalData> global_{};
	bool exists_ = false;
	std::string cur_room_name_{};

private:
	void save_meta();
	void load_room_data(std::filesystem::path subsave_path);
	void save_room_data(std::filesystem::path subsave_path, std::string cur_room_name);
	void save_room(Room* room, std::filesystem::path path);

	const std::filesystem::path base_;
	std::map<std::string, unsigned int> room_subsave_{};
	unsigned int cur_subsave_ = 0;
	unsigned int next_subsave_ = 1;
};

class GlobalFlagDelta : public Delta {
public:
	GlobalFlagDelta(PlayingGlobalData* global, unsigned int flag);
	~GlobalFlagDelta();
	void revert();

private:
	PlayingGlobalData* global_;
	unsigned int flag_;
};
