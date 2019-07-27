#pragma once

#include <filesystem>
#include <map>

struct PlayingRoom;
class Room;
class GlobalData {};

class SaveFile {
public:
	SaveFile(const std::string& base);
	~SaveFile();

	bool create();
	bool load_meta();
	std::filesystem::path get_path(std::string const&, bool* from_main);
	void make_subsave(std::map<std::string, std::unique_ptr<PlayingRoom>>& loaded_rooms, std::string const& cur_room_name);
	void load_most_recent_subsave(std::string* cur_room_name);

private:
	void save_meta();
	void load_room_data(std::filesystem::path const& subsave_path, std::string* cur_room_name);
	void save_room_data(std::filesystem::path const& subsave_path, std::string const& cur_room_name);
	void save_room(Room* room, std::filesystem::path path);

	const std::filesystem::path base_;
	std::map<std::string, unsigned int> room_subsave_;
	std::unique_ptr<GlobalData> global_;
	unsigned int cur_subsave_;
	unsigned int next_subsave_;
};