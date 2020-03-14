#ifndef REALPLAYINGSTATE_H
#define REALPLAYINGSTATE_H

#include "playingstate.h"

class PlayingGlobalData;
class SaveFile;
class FontManager;

class RealPlayingState: public PlayingState {
public:
    RealPlayingState(SaveFile*, GameState* parent);
    ~RealPlayingState();

	void play_from_map(std::string starting_map);
	bool load_room(std::string name, bool use_default_player);
	void make_subsave();
	void play_from_loaded_subsave();

	void world_reset();

private:
	SaveFile* savefile_;
};


#endif // REALPLAYINGSTATE_H
