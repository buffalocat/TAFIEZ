#ifndef GATEBODY_H
#define GATEBODY_H

#include "pushblock.h"
#include "delta.h"

class Gate;
class MoveProcessor;

// The part of a Gate that comes up above the ground
// It inherits the color, pushability, and gravitability of its corresponding Gate object
class GateBody : public PushBlock {
public:
	GateBody(Gate* gate, Point3 pos);
	GateBody(Point3 pos, int color, bool pushable, bool gravitable, bool snake, bool persistent, bool corrupt);
	~GateBody();

	std::string name();
	ObjCode obj_code();
	void serialize(MapFileO& file);
	static std::unique_ptr<GameObject> deserialize(MapFileI& file, Point3 pos);
	bool relation_check();
	void relation_serialize(MapFileO& file);

	Point3 gate_pos();
	void set_gate(Gate*);

	void destroy(MoveProcessor*, CauseOfDeath);

	void collect_special_links(std::vector<GameObject*>&);

	void draw(GraphicsManager*);
	Gate* gate_;

private:
	bool snake_;
	bool persistent_;
	bool corrupt_;
};

class GateUnlinkDelta : public Delta {
public:
	GateUnlinkDelta(GateBody* body, Gate* gate);
	GateUnlinkDelta(FrozenObject body, FrozenObject gate);
	~GateUnlinkDelta();
	void serialize(MapFileO&);
	void revert(RoomMap*);
	DeltaCode code();
	static std::unique_ptr<Delta> deserialize(MapFileI& file);

private:
	FrozenObject body_;
	FrozenObject gate_;
};


#endif // GATEBODY_H
