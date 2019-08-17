#ifndef CAR_H
#define CAR_H

#include "objectmodifier.h"
#include "colorcycle.h"

class Car: public ObjectModifier {
public:
    Car(GameObject* parent, ColorCycle color_cycle);
    virtual ~Car();

	void make_str(std::string&);
    ModCode mod_code();
    void serialize(MapFileO& file);
    static void deserialize(MapFileI&, RoomMap*, GameObject*);

	bool valid_parent(GameObject*);

	virtual BlockTexture texture();

    void collect_sticky_links(RoomMap*, Sticky, std::vector<GameObject*>&);

    bool cycle_color(bool undo);
	int next_color();

	void draw(GraphicsManager* gfx, FPoint3 p);

    std::unique_ptr<ObjectModifier> duplicate(GameObject*, RoomMap*, DeltaFrame*);

    ColorCycle color_cycle_;
};

#endif // CAR_H
