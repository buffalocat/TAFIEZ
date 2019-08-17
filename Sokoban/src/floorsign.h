#pragma once
#include "objectmodifier.h"

class SignTextDrawer;
class TextRenderer;

class FloorSign : public ObjectModifier {
public:
	FloorSign(GameObject* parent, std::string content, bool showing);
	~FloorSign();

	std::string name();
	ModCode mod_code();

	void serialize(MapFileO& file);
	static void deserialize(MapFileI& file, RoomMap*, GameObject* parent);

	std::unique_ptr<ObjectModifier> duplicate(GameObject*, RoomMap*, DeltaFrame*);

	void map_callback(RoomMap*, DeltaFrame*, MoveProcessor*);

	void setup_on_put(RoomMap*, bool real);
	void cleanup_on_take(RoomMap*, bool real);

	void set_text_state(bool state, TextRenderer* text);
	void toggle_active(TextRenderer* text, DeltaFrame* delta_frame);

	void draw(GraphicsManager* gfx, FPoint3 pos);

private:
	std::string content_;
	SignTextDrawer* drawer_instance_{};
	bool active_;

	friend class SignToggleDelta;
	friend class ModifierTab;
};

