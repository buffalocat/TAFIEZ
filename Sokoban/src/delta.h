#ifndef DELTA_H
#define DELTA_H

#include "point.h"


class MapFileO;
class GameObject;
class ObjectModifier;
class RoomMap;
class PlayingState;
class GameObjectArray;

enum class ObjRefCode;

class FrozenObject {
public:
	FrozenObject(GameObject* obj);
	FrozenObject(ObjectModifier* mod);
	FrozenObject(Point3 pos, ObjRefCode ref);
	~FrozenObject();

	void init_from_obj();
	void serialize(MapFileO& file, GameObjectArray* arr);
	GameObject* resolve(RoomMap*);

	GameObject* obj_;
	ObjRefCode ref_;
	Point3 pos_;
};


class Delta {
public:
	virtual ~Delta();
	virtual void revert(RoomMap* room_map) = 0;
	virtual void serialize(MapFileO& file, GameObjectArray* arr) = 0;
};


class DeltaFrame {
public:
	DeltaFrame();
	~DeltaFrame();
	void revert(PlayingState* state);
	void push(std::unique_ptr<Delta>);
	bool trivial();

	void reset_changed();
	bool changed();

	void serialize(MapFileO& file, GameObjectArray* arr);

private:
	std::vector<std::unique_ptr<Delta>> deltas_{};
	bool changed_ = false;
};


class UndoStack {
public:
	UndoStack(PlayingState* state, unsigned int max_depth);
	~UndoStack();
	void push(std::unique_ptr<DeltaFrame>);
	bool non_empty();
	void pop();
	void reset();
	void serialize(MapFileO& file, GameObjectArray* arr);

private:
	PlayingState* state_;
	std::deque<std::unique_ptr<DeltaFrame>> frames_;
	unsigned int max_depth_;
	unsigned int size_;
};

#endif // DELTA_H
