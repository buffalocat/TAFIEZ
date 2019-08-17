#ifndef TESTPLAYINGSTATE_H
#define TESTPLAYINGSTATE_H

#include "playingstate.h"
#include "point.h"

class FontManager;

class TestPlayingState: public PlayingState {
public:
    TestPlayingState(GameState* parent);
    virtual ~TestPlayingState();

	void init(std::string map_name);

	bool load_room_from_temp(std::string name, bool use_default_player);
	bool load_room(std::string, bool use_default_player);

private:
};

#endif // TESTPLAYINGSTATE_H
