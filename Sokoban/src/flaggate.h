#ifndef FLAGGATE_H
#define FLAGGATE_H

#include "switchable.h"
#include "delta.h"

const int MAX_FLAG_SIGIL_OPACITY = 60;

class ModelInstancer;

struct FlagSigil {
	glm::vec3 center;
	double radius;
	int phase, period;
	int index;
	int charge = 0;
	int opacity = MAX_FLAG_SIGIL_OPACITY;

	void update(bool signal, int total);
	void draw(ModelInstancer* model, FPoint3 p, int orientation, int time);
};


enum class FlagGateAnimationState {
	Default,
	Charging,
	Fade,
	Shrink,
};


class FlagGate : public Switchable {
public:
	FlagGate(GameObject* parent, int num_flags, int orientation, int count, bool active, bool walls_placed, bool down);
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
	void apply_state_change(RoomMap*, DeltaFrame*, MoveProcessor*);

	void destroy(MoveProcessor* mp, CauseOfDeath);
	void signal_animation(AnimationManager*, DeltaFrame*);
	void reset_animation();

	void get_gate_dims(int* width, int* height);
	void get_gate_extremes(Point3& a, Point3& b);

	int num_flags_;
	int orientation_;
	bool walls_placed_;
	bool down_;
	// Animation/Drawing members
	glm::vec3 center_;
	glm::vec3 scale_;
	std::vector<FlagSigil> sigils_{};
	int cycle_time_ = 0;
	FlagGateAnimationState animation_state_ = FlagGateAnimationState::Default;
	int animation_timer_ = 0;

private:
	DeltaFrame* stored_delta_frame_{};
};


class FlagGateOpenDelta: public Delta{
public:
	FlagGateOpenDelta(FlagGate* fg);
	~FlagGateOpenDelta();

	void revert();

private:
	FlagGate* fg_;
};


#endif //FLAGGATE_H