#ifndef TESTPLAYINGSTATE_H
#define TESTPLAYINGSTATE_H



#include "playingstate.h"
#include "point.h"

class TestPlayingState: public PlayingState {
public:
    TestPlayingState(const std::string& map_name);
    virtual ~TestPlayingState();

	bool load_room(const std::string&, bool use_default_player);

private:
};

#endif // TESTPLAYINGSTATE_H
