#ifndef TESTPLAYINGSTATE_H
#define TESTPLAYINGSTATE_H



#include "playingstate.h"
#include "point.h"

class TestPlayingState: public PlayingState {
public:
    TestPlayingState(const std::string& name, Point3 pos);
    virtual ~TestPlayingState();
    void init_player(Point3);

private:
};

#endif // TESTPLAYINGSTATE_H
