#ifndef DELTA_H
#define DELTA_H

#include "point.h"

class Room;
class RoomMap;
class PlayingGlobalData;
class GameObject;
class PushBlock;
class SnakeBlock;
class Switchable;
class Switch;
class Signaler;
class Player;
class PlayingState;
class Car;
class GateBody;
class ClearFlag;
class WorldResetKey;
class FloorSign;
class TextRenderer;

enum class PlayerState;

class Delta {
public:
	virtual ~Delta();
	virtual void revert() = 0;
};


class DeltaFrame {
public:
	DeltaFrame();
	~DeltaFrame();
	void revert();
	void push(std::unique_ptr<Delta>);
	bool trivial();

	void reset_changed();
	bool changed();

private:
	std::vector<std::unique_ptr<Delta>> deltas_{};
	bool changed_ = false;
};


class UndoStack {
public:
	UndoStack(unsigned int max_depth);
	~UndoStack();
	void push(std::unique_ptr<DeltaFrame>);
	bool non_empty();
	void pop();
	void reset();

private:
	std::deque<std::unique_ptr<DeltaFrame>> frames_;
	unsigned int max_depth_;
	unsigned int size_;
};


class ObjArrayPushDelta : public Delta {
public:
	ObjArrayPushDelta(GameObject* obj, RoomMap* map);
	~ObjArrayPushDelta();
	void revert();

private:
	GameObject* obj_;
	RoomMap* map_;
};


class PutDelta : public Delta {
public:
	PutDelta(GameObject* obj, RoomMap* map);
	~PutDelta();
	void revert();

private:
	GameObject* obj_;
	RoomMap* map_;
};


class TakeDelta : public Delta {
public:
	TakeDelta(GameObject* obj, RoomMap* map);
	~TakeDelta();
	void revert();

private:
	GameObject* obj_;
	RoomMap* map_;
};


class MotionDelta : public Delta {
public:
	MotionDelta(GameObject* obj, Point3 dpos, RoomMap* map);
	~MotionDelta();
	void revert();

private:
	GameObject* obj_;
	Point3 dpos_;
	RoomMap* map_;
};


class BatchMotionDelta : public Delta {
public:
	BatchMotionDelta(std::vector<GameObject*> objs, Point3 dpos, RoomMap* map);
	~BatchMotionDelta();
	void revert();

private:
	std::vector<GameObject*> objs_;
	Point3 dpos_;
	RoomMap* map_;
};

// Object motion outside of the map
class AbstractShiftDelta : public Delta {
public:
	AbstractShiftDelta(GameObject* obj, Point3 dpos);
	~AbstractShiftDelta();
	void revert();

private:
	GameObject* obj_;
	Point3 dpos_;
};


class AbstractPutDelta : public Delta {
public:
	AbstractPutDelta(GameObject* obj, Point3 pos);
	~AbstractPutDelta();
	void revert();

private:
	GameObject* obj_;
	Point3 pos_;
};


class AddLinkDelta : public Delta {
public:
	AddLinkDelta(SnakeBlock* a, SnakeBlock* b);
	~AddLinkDelta();
	void revert();

private:
	SnakeBlock* a_;
	SnakeBlock* b_;
};


class RemoveLinkDelta : public Delta {
public:
	RemoveLinkDelta(SnakeBlock* a, SnakeBlock* b);
	~RemoveLinkDelta();
	void revert();

private:
	SnakeBlock* a_;
	SnakeBlock* b_;
};


class RoomChangeDelta : public Delta {
public:
	RoomChangeDelta(PlayingState* state, Room* room);
	~RoomChangeDelta();
	void revert();

private:
	PlayingState* state_;
	Room* room_;
};


class SwitchableDelta : public Delta {
public:
	SwitchableDelta(Switchable* obj, int count, bool active, bool waiting);
	~SwitchableDelta();
	void revert();

private:
	Switchable* obj_;
	int count_;
	bool active_;
	bool waiting_;
};


class SwitchToggleDelta : public Delta {
public:
	SwitchToggleDelta(Switch* obj);
	~SwitchToggleDelta();
	void revert();

private:
	Switch* obj_;
};

class SignalerCountDelta : public Delta {
public:
	SignalerCountDelta(Signaler*, int count);
	~SignalerCountDelta();
	void revert();

private:
	Signaler* sig_;
	int count_;
};


class PlayerStateDelta : public Delta {
public:
	PlayerStateDelta(Player* player, Car* car, PlayerState state);
	~PlayerStateDelta();
	void revert();

private:
	Player* player_;
	Car* car_;
	PlayerState state_;
};


class ColorChangeDelta : public Delta {
public:
	ColorChangeDelta(Car* car, bool undo);
	~ColorChangeDelta();
	void revert();

private:
	Car* car_;
	bool undo_;
};

class GatePosDelta : public Delta {
public:
	GatePosDelta(GateBody* gate_body, Point3 dpos);
	~GatePosDelta();
	void revert();

private:
	GateBody* gate_body_;
	Point3 dpos_;
};

class ClearFlagToggleDelta : public Delta {
public:
	ClearFlagToggleDelta(ClearFlag* flag, RoomMap* map);
	~ClearFlagToggleDelta();
	void revert();

private:
	ClearFlag* flag_;
	RoomMap* map_;
};

class ClearFlagCollectionDelta : public Delta {
public:
	ClearFlagCollectionDelta(RoomMap* map, int req);
	~ClearFlagCollectionDelta();
	void revert();

private:
	RoomMap* map_;
	int req_;
};

class KeyCollectDelta : public Delta {
public:
	KeyCollectDelta(WorldResetKey* key);
	~KeyCollectDelta();
	void revert();

private:
	WorldResetKey* key_;
};

class GlobalFlagDelta : public Delta {
public:
	GlobalFlagDelta(PlayingGlobalData* global, unsigned int flag);
	~GlobalFlagDelta();
	void revert();

private:
	PlayingGlobalData* global_;
	unsigned int flag_;
};

class SignToggleDelta : public Delta {
public:
	SignToggleDelta(FloorSign* sign, TextRenderer* text);
	~SignToggleDelta();
	void revert();

private:
	FloorSign* sign_;
	TextRenderer* text_;
};

class MapInitDelta : public Delta {
public:
	MapInitDelta(RoomMap* map);
	~MapInitDelta();

	void revert();
private:
	RoomMap* map_;
};

#endif // DELTA_H
