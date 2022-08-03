#pragma once

#include "gamestate.h"

class MetaEditorState : public GameState {
public:
    MetaEditorState(GameState* parent);
    virtual ~MetaEditorState();

private:
};
