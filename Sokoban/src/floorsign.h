#pragma once
#include "objectmodifier.h"
#include "delta.h"

class IndependentStringDrawer;
class TextRenderer;

class FloorSign : public ObjectModifier {
public:
	FloorSign(GameObject* parent, std::string content, bool showing);
	~FloorSign();

	void make_str(std::string&);
	ModCode mod_code();

	void serialize(MapFileO& file);
	static void deserialize(MapFileI& file, RoomMap*, GameObject* parent);
	bool relation_check();
	void relation_serialize(MapFileO& file);

	std::unique_ptr<ObjectModifier> duplicate(GameObject*, RoomMap*, DeltaFrame*);

	void map_callback(RoomMap*, DeltaFrame*, MoveProcessor*);

	void setup_on_put(RoomMap*, DeltaFrame*, bool real);
	void cleanup_on_take(RoomMap*, DeltaFrame*, bool real);

	void set_text_state(bool state, TextRenderer* text);
	void toggle_active(TextRenderer* text, DeltaFrame* delta_frame);

	void draw(GraphicsManager* gfx, FPoint3 pos);

	unsigned int learn_flag_ = 0;

private:
	std::string content_;
	IndependentStringDrawer* drawer_instance_{};
	bool active_;

	friend class SignToggleDelta;
	friend class ModifierTab;
};


class SignToggleDelta : public Delta {
public:
	SignToggleDelta(FloorSign* sign);
	~SignToggleDelta();
	void serialize(MapFileO&, GameObjectArray*);
	void revert(RoomMap*);

private:
	FrozenObject sign_;
};

class LearnFlagDelta : public Delta {
public:
	LearnFlagDelta(FloorSign* sign, unsigned int flag);
	~LearnFlagDelta();
	void serialize(MapFileO&, GameObjectArray*);
	void revert(RoomMap*);

private:
	FrozenObject sign_;
	unsigned int flag_;
};

