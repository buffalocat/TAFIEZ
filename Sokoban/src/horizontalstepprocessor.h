#ifndef HORIZONTALSTEPPROCESSOR_H
#define HORIZONTALSTEPPROCESSOR_H




#include "point.h"

#include "component.h"

class GameObject;
class SnakeBlock;
class Player;

class RoomMap;
class DeltaFrame;

class HorizontalStepProcessor {
public:
    HorizontalStepProcessor(RoomMap*, DeltaFrame*, Player*, Point3, std::vector<GameObject*>&, std::vector<GameObject*>&);
    ~HorizontalStepProcessor();

    void run();

private:
    void perform_horizontal_step();

    bool compute_push_component_tree(GameObject* block);
    bool compute_push_component(GameObject* block);

    void collect_moving_and_weak_links(PushComponent* comp, std::vector<GameObject*>& weak_links);

    std::vector<std::unique_ptr<PushComponent>> push_comps_unique_;

    std::vector<SnakeBlock*> moving_snakes_;
    std::vector<SnakeBlock*> snakes_to_recheck_;
    std::vector<GameObject*>& moving_blocks_;
    std::vector<GameObject*>& fall_check_;

    RoomMap* map_;
    DeltaFrame* delta_frame_;
	Player* player_;
    Point3 dir_;
};

#endif // HORIZONTALSTEPPROCESSOR_H