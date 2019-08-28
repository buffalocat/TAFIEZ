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
};

class Car: public ObjectModifier {
public:
    Car(GameObject* parent, CarType type, ColorCycle color_cycle);
    virtual ~Car();

	void make_str(std::string&);
    ModCode mod_code();
    void serialize(MapFileO& file);
    static void deserialize(MapFileI&, RoomMap*, GameObject*);

	bool valid_parent(GameObject*);

	virtual BlockTexture texture();

	void shift_internal_pos(Point3 d);

    bool cycle_color(bool undo);
	int next_color();

	void map_callback(RoomMap*, DeltaFrame*, MoveProcessor*);
	void collect_sticky_links(RoomMap*, Sticky, std::vector<GameObject*>&);

	void cleanup_on_take(RoomMap* map, bool real);
	void setup_on_put(RoomMap* map, bool real);
	void destroy(DeltaFrame*, CauseOfDeath);

	void draw(GraphicsManager* gfx, FPoint3 p);

    std::unique_ptr<ObjectModifier> duplicate(GameObject*, RoomMap*, DeltaFrame*);
	
	CarType type_;
    ColorCycle color_cycle_;

	Player* player_{};
};

#endif // CAR_H
