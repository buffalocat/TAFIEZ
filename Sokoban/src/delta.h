#ifndef DELTA_H
#define DELTA_H

#include "point.h"


class MapFileIwithObjs;
class MapFileO;
class GameObject;
class ObjectModifier;
class RoomMap;
class PlayingState;
class GameObjectArray;

enum class ObjRefCode;

enum class DeltaCode {
	AnimationSignalDelta = 1,
	AutosavePanelDelta = 2,
	ClearFlagToggleDelta = 3,
	FlagGateOpenDelta = 4,
	SignToggleDelta = 5,
	LearnFlagDelta = 6,
	DestructionDelta = 7,
	AbstractShiftDelta = 8,
	AbstractPutDelta = 9,
	RoomChangeDelta = 10,
	ToggleGravitableDelta = 11,
	ColorChangeDelta = 12,
	ModDestructionDelta = 13,
	PlayerStateDelta =14,
	PutDelta = 15,
	TakeDelta = 16,
	WallDestructionDelta = 17,
	ObjArrayPushDelta = 18,
	ObjArrayDeletedPushDelta = 19,
	MotionDelta = 20,
	BatchMotionDelta = 21,
	ClearFlagCollectionDelta = 22,
	AddPlayerDelta = 23,
	RemovePlayerDelta = 24,
	CyclePlayerDelta = 25,
	GlobalFlagDelta = 26,
	FlagCountDelta = 27,
	AutosaveDelta = 28,
	SignalerCountDelta = 29,
	AddLinkDelta = 30,
	RemoveLinkDelta = 31,
	RemoveLinkOneWayDelta = 32,
	SwitchToggleDelta = 33,
	SwitchableDelta = 34,
};

class FrozenObject {
public:
	FrozenObject();
	FrozenObject(GameObject* obj);
	FrozenObject(ObjectModifier* mod);
	FrozenObject(Point3 pos, ObjRefCode ref);
	~FrozenObject();

	static FrozenObject create_dead_obj(GameObject* obj);
	void init_from_obj();
	void serialize(MapFileO& file, GameObjectArray* arr);
	GameObject* resolve(RoomMap*);
	ObjectModifier* resolve_mod(RoomMap*);

	GameObject* obj_;
	ObjRefCode ref_;
	Point3 pos_;
};


class Delta {
public:
	virtual ~Delta();
	virtual void revert(RoomMap* room_map) = 0;
	virtual void serialize(MapFileO& file, GameObjectArray* arr) = 0;
	virtual DeltaCode code() = 0;
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

	void deserialize(MapFileIwithObjs& file);
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
	void deserialize(MapFileIwithObjs& file);
	void serialize(MapFileO& file, GameObjectArray* arr);

private:
	PlayingState* state_;
	std::deque<std::unique_ptr<DeltaFrame>> frames_;
	unsigned int max_depth_;
	unsigned int size_;
};

#endif // DELTA_H
