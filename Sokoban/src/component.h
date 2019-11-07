#ifndef COMPONENT_H
#define COMPONENT_H



class GameObject;
class RoomMap;

struct Component {
    virtual ~Component();

	std::vector<GameObject*> blocks_{};
};

struct PushComponent: public Component {
    void add_pushing(Component* comp);

	std::vector<PushComponent*> pushing_{};
    bool blocked_ = false;
    bool moving_ = false;
};

struct FallComponent: Component {
    void add_above(Component* comp);

    void settle_first();
    void take_falling(RoomMap* map);

	std::vector<FallComponent*> above_{};
    bool settled_ = false;
};

#endif // COMPONENT_H
