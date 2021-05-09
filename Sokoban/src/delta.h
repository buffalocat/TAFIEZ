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
	SignalerCountDelta = 28,
	AddLinkDelta = 29,
	RemoveLinkDelta = 30,
	GateUnlinkDelta = 31,
	SwitchToggleDelta = 32,
	SwitchableDelta = 33,
};

class FrozenObject {
public:
	FrozenObject();
	FrozenObject(GameObject* obj);
	FrozenObject(ObjectModifier* mod);
	FrozenObject(Point3 pos, ObjRefCode ref, unsigned int inacc_id);
	~FrozenObject();

	void init_from_obj();
	void serialize(MapFileO& file);
	GameObject* resolve(RoomMap*);
	ObjectModifier* resolve_mod(RoomMap*);

	void print();

	GameObject* obj_;
	ObjRefCode ref_;
	Point3 pos_;
	unsigned int inacc_id_;
};


class Delta {
public:
	virtual ~Delta();
	virtual void revert(RoomMap* room_map) = 0;
	virtual void serialize(MapFileO& file) = 0;
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
	void serialize(MapFileO& file);

	std::vector<std::unique_ptr<Delta>> deltas_{};
private:
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
	void deserialize(std::filesystem::path base_path, unsigned int subsave_index, GameObjectArray* arr);
	void serialize(std::filesystem::path subsave_path, unsigned int subsave_index, GameObjectArray* arr);
	std::vector<unsigned int> dependent_subsaves();

private:
	PlayingState* state_;
	std::deque<std::pair<unsigned int, unsigned int>> cache_map_{};
	unsigned int num_new_frames_{ 0 };
	unsigned int skip_frames_{ 0 };
	std::deque<std::unique_ptr<DeltaFrame>> frames_{};
	unsigned int max_depth_;
	unsigned int size_{ 0 };
};

#endif // DELTA_H
