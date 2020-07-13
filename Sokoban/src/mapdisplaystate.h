#ifndef MAPDISPLAYSTATE_H
#define MAPDISPLAYSTATE_H

#include "gamestate.h"

enum class HubAccessFlag;
enum class HubCode;

class PlayingGlobalData;

class MapDisplayState : public GameState {
public:
	MapDisplayState(GameState* parent);
	~MapDisplayState();
	void main_loop();
	void draw_map();

	void generate_map();
	void draw_zone(char zone, int x, int y);
	void draw_hub(HubCode hub, int x, int y);
	void draw_warp(HubCode hub, char zone, int x, int y);

private:
	PlayingGlobalData* global;
};

#endif //MAPDISPLAYSTATE_H