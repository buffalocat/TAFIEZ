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
	static std::unique_ptr<GameObject> deserialize(MapFileI& file);
	bool relation_check();
	void relation_serialize(MapFileO& file);

	Point3 gate_pos();
	void set_gate(Gate*);
	Point3 update_gate_pos(DeltaFrame*);

	void destroy(MoveProcessor*, CauseOfDeath, bool collect_links);
	void undestroy();

	void collect_special_links(RoomMap*, std::vector<GameObject*>&);

	void draw(GraphicsManager*);

private:
	Gate* gate_;
	Point3 gate_pos_;
	bool snake_;
	bool persistent_;
	bool corrupt_;

	friend class GatePosDelta;
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

#endif // GATEBODY_H
