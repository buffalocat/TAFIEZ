#include "stdafx.h"
#include "savefile.h"

#include "room.h"

SaveFile::SaveFile(const std::string& base) :
	base_ {std::filesystem::path("saves") / base},
	room_subdir_{} {
	std::filesystem::create_directory(base_);
}

SaveFile::~SaveFile() {}

std::filesystem::path SaveFile::get_path(std::string const& name) {
	if (room_subdir_.count(name)) {
		return base_ / room_subdir_[name];
	} else {
		return (std::filesystem::path("maps") / "main" / name).concat(".map");
	}
}

void SaveFile::make_sub_save(std::map<std::string, std::unique_ptr<PlayingRoom>>& loaded_rooms) {

}
