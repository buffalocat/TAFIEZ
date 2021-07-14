#ifndef CAR_H
#define CAR_H

#include "objectmodifier.h"
#include "colorcycle.h"

class Player;

enum class CarType {
	Normal = 0,
	Locked = 1,
	Convertible = 2,
	Hover = 3,
	Binding = 4,
	GrappleWeak = 5,
	GrappleStrong = 6,
};

enum class CarAnimationState {
	None,
	Riding,
	Unriding,
};

const int MAX_CAR_ANIMATION_FRAMES = 4;

class Car: public ObjectModifier {
public:
    Car(GameObject* parent, CarType type, ColorCycle color_cycle);
    virtual ~Car();

	void make_str(std::string&);
    ModCode mod_code();
    void serialize(MapFileO& file);
	void serialize_with_ids(MapFileO& file);
    static void deserialize(MapFileI&, RoomMap*, GameObject*);
	static void deserialize_with_ids(MapFileI&, RoomMap*, GameObject*);

	bool valid_parent(GameObject*);

	GameObject* get_subordinate_object();

	virtual BlockTexture texture();

	bool is_multi_color();
    void cycle_color(bool undo);
	int next_color();

	void handle_movement(RoomMap*, DeltaFrame*, MoveProcessor*);
	void collect_special_links(std::vector<GameObject*>&);

	void cleanup_on_take(RoomMap*, DeltaFrame*, bool real);
	void setup_on_put(RoomMap*, DeltaFrame*, bool real);
	void destroy(MoveProcessor*, CauseOfDeath);

	bool update_animation(PlayingState*);
	void reset_animation();
	void draw(GraphicsManager* gfx, FPoint3 p);
	void draw_squished(GraphicsManager*, FPoint3 p, float scale);


    std::unique_ptr<ObjectModifier> duplicate(GameObject*, RoomMap*, DeltaFrame*);
	
	CarType type_;
    ColorCycle color_cycle_;

	Player* player_{};
	
	// The player currently in the car for animation purposes
	Player* animation_player_{};
	CarAnimationState animation_state_ = CarAnimationState::None;
	int animation_time_ = 0;
};

#endif // CAR_H
