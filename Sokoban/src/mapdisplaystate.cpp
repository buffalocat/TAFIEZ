#include "stdafx.h"
#include "mapdisplaystate.h"

#include "savefile.h"
#include "playingstate.h"
#include "globalflagconstants.h"


MapDisplayState::MapDisplayState(GameState* parent) {
	PlayingState* ps = dynamic_cast<PlayingState*>(parent->parent_.get());
	global = ps->global_;
	generate_map();
}

MapDisplayState::~MapDisplayState() {}

void MapDisplayState::main_loop() {

}

void MapDisplayState::draw_map() {

}

void MapDisplayState::generate_map() {
	if (global->has_flag(HUB_ACCESSED_GLOBAL_FLAGS[(int)HubCode::Alpha])) {
		draw_hub(HubCode::Alpha, 0, 0);
	}
}

void MapDisplayState::draw_zone(char zone, int x, int y) {
	if (!global->has_flag(get_zone_access_code(zone))) {
		// Draw question mark
		return;
	}
	if (global->has_flag(get_clear_flag_code(zone))) {
		// Draw gold square
		// Draw neighbors
	} else {
		// Draw white? square
	}
	// Draw letter "zone" at (x,y)
}

void MapDisplayState::draw_hub(HubCode hub, int x, int y) {
	if (global->has_flag(HUB_ACCESSED_GLOBAL_FLAGS[static_cast<int>(hub)])) {
		// Draw color coded? square (red, green, blue, purple)
		// Draw (greek) letter at (x,y)
	}
}

void MapDisplayState::draw_warp(HubCode hub, char zone, int x, int y) {
	bool should_draw;
	if (zone == 'H') {
		should_draw = global->has_flag(HUB_ALT_ACCESSED_GLOBAL_FLAGS[static_cast<int>(hub)]);
	} else { // zone == 'X'
		should_draw = global->has_flag(HUB_ALT_ACCESSED_GLOBAL_FLAGS[static_cast<int>(hub)]);
	}
	if (should_draw) {
		// Draw (greek) letter at (x,y)
	}
}
