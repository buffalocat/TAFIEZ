#include "stdafx.h"
#include "optionmenustate.h"

#include "menu.h"
#include "graphicsmanager.h"
#include "fontmanager.h"
#include "common_constants.h"

OptionMenuState::OptionMenuState(GameState* parent) : GameState(parent),
menu_{ std::make_unique<Menu>(this, gfx_->fonts_->get_font(Fonts::ABEEZEE, 72)) } {
	menu_->push_entry("Toggle Fullscreen", [this]() { toggle_fullscreen(); });
}

OptionMenuState::~OptionMenuState() {}

