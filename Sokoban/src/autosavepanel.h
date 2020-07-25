#pragma once

#include "objectmodifier.h"

class AutosavePanel : public ObjectModifier {
public:
	AutosavePanel(GameObject* parent, std::string label, bool active);
	~AutosavePanel();

	void make_str(std::string&);
	ModCode mod_code();
	void serialize(MapFileO& file);
	static void deserialize(MapFileI&, RoomMap*, GameObject*);

	void map_callback(RoomMap*, DeltaFrame*, MoveProcessor*);

	void setup_on_put(RoomMap*, DeltaFrame*, bool real);
	void cleanup_on_take(RoomMap*, DeltaFrame*, bool real);

	void draw(GraphicsManager*, FPoint3);

	std::unique_ptr<ObjectModifier> duplicate(GameObject*, RoomMap*, DeltaFrame*);

	std::string label_;
	bool active_;
};

class AutosavePanelDelta : public Delta {
public:
	AutosavePanelDelta(AutosavePanel* panel);
	~AutosavePanelDelta();
	void revert();

private:
	AutosavePanel* panel_;
};

