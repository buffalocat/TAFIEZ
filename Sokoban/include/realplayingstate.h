#ifndef REALPLAYINGSTATE_H
#define REALPLAYINGSTATE_H

#include "playingstate.h"

class SaveFile;

class RealPlayingState: public PlayingState {
public:
    RealPlayingState(const std::string& savefile_dir, const std::string& starting_map);
    virtual ~RealPlayingState();
	bool load_room(const std::string& path, bool use_default_player);
    void create_new_savefile();
    void load_savefile();

private:
	std::unique_ptr<SaveFile> savefile_;
};


#endif // REALPLAYINGSTATE_H
