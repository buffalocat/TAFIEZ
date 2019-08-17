#pragma once
#include "pressswitch.h"

class EditorGlobalData;

class PermanentSwitch :	public PressSwitch {
public:
	PermanentSwitch(GameObject* parent, int color, bool active, unsigned int global_id);
	~PermanentSwitch();

	void make_str(std::string&);
	ModCode mod_code();

	void serialize(MapFileO& file);
	static void deserialize(MapFileI&, RoomMap*, GameObject*);
	bool relation_check();
	void relation_serialize(MapFileO& file);

	void check_send_signal(RoomMap* map, DeltaFrame* delta_frame);

	void setup_on_editor_creation(EditorGlobalData* global, Room* room);
	void cleanup_on_editor_destruction(EditorGlobalData* global);

	void draw(GraphicsManager*, FPoint3);

	std::unique_ptr<ObjectModifier> duplicate(GameObject*, RoomMap*, DeltaFrame*);

private:
	unsigned int global_id_;
};

