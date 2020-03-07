#ifndef FLAGGATE_H
#define FLAGGATE_H

#include "switchable.h"

class DynamicInstancer;

struct FlagSigil {
	glm::vec3 center;
	double radius;
	int phase, period;
	int index;
	int charge = 0;

	void update(bool signal, int total);
	void draw(DynamicInstancer* model, FPoint3 p, int orientation, int time);
};


class FlagGate : public Switchable {
public:
	FlagGate(GameObject* parent, int num_flags, int orientation, int count, bool active, bool walls_placed);
	~FlagGate();

	void make_str(std::string&);
	ModCode mod_code();

	void serialize(MapFileO& file);
	static void deserialize(MapFileI& file, RoomMap* map, GameObject* parent);
	std::unique_ptr<ObjectModifier> duplicate(GameObject*, RoomMap*, DeltaFrame*);

	bool can_set_state(bool state, RoomMap*);

	void init_draw_constants();
	bool update_animation(PlayingState*);
	void draw(GraphicsManager*, FPoint3);

	void place_walls(RoomMap*);
	void remove_walls(RoomMap*, DeltaFrame*);
	void spawn_sigils();

	void map_callback(RoomMap*, DeltaFrame*, MoveProcessor*);
	void destroy(MoveProcessor* mp, CauseOfDeath);
	void signal_animation(AnimationManager*, DeltaFrame*);
	void reset_animation();

	void get_gate_dims(int* width, int* height);
	void get_gate_extremes(Point3& a, Point3& b);

	int num_flags_;
	int orientation_;
	bool walls_placed_;
	bool down_ = false;
	// Animation/Drawing members
	glm::vec3 center_;
	glm::vec3 scale_;
	std::vector<FlagSigil> sigils_{};
	int animation_time_ = 0;
};


#endif //FLAGGATE_H