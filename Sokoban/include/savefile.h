#pragma once

#include <filesystem>
#include <map>

struct PlayingRoom;

class SaveFile {
public:
	SaveFile(const std::string& base);
	~SaveFile();
	std::filesystem::path get_path(std::string const&);
	void make_sub_save(std::map<std::string, std::unique_ptr<PlayingRoom>>& loaded_rooms);

private:
	const std::filesystem::path base_;
	std::map<std::string, std::string> room_subdir_;
};