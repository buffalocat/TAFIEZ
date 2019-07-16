#ifndef REALPLAYINGSTATE_H
#define REALPLAYINGSTATE_H

#include "playingstate.h"

class RealPlayingState: public PlayingState {
public:
    RealPlayingState(const std::string& savefile_dir);
    virtual ~RealPlayingState();
    void create_new_savefile();
    void load_savefile();

private:
    std::string savefile_dir_;
};


#endif // REALPLAYINGSTATE_H
