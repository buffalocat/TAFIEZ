#ifndef REALPLAYINGSTATE_H
#define REALPLAYINGSTATE_H

#include "playingstate.h"

class PlayingGlobalData;
class SaveFile;

class RealPlayingState: public PlayingState {
public:
    RealPlayingState(std::unique_ptr<SaveFile> save);
    virtual ~RealPlayingState();

	void start_from_map(std::string starting_map);
	bool load_room(std::string name, bool use_default_player);
	void make_subsave();
	void load_most_recent_subsave();

	void world_reset();

private:
	std::unique_ptr<SaveFile> savefile_;
};


#endif // REALPLAYINGSTATE_H
