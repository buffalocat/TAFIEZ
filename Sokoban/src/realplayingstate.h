#ifndef REALPLAYINGSTATE_H
#define REALPLAYINGSTATE_H

#include "playingstate.h"

class PlayingGlobalData;
class SaveProfile;
class AutosavePanel;
class FontManager;

class RealPlayingState: public PlayingState {
public:
    RealPlayingState(SaveProfile*, GameState* parent);
	RealPlayingState(RealPlayingState* rps);
    ~RealPlayingState();

	void main_loop();
	void play_from_map(std::string starting_map);
	bool load_room(std::string name, bool use_default_player);
	void make_subsave(SaveType type, unsigned int save_index = 0, AutosavePanel* panel = nullptr);
	bool load_subsave_dispatch(SaveType type, unsigned int index);
	void play_from_loaded_subsave();

	void world_reset();
	void ensure_safe_delta_state();

	SaveProfile* savefile_;
};


#endif // REALPLAYINGSTATE_H
