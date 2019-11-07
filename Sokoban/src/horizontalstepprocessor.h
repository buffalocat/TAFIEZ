#ifndef HORIZONTALSTEPPROCESSOR_H
#define HORIZONTALSTEPPROCESSOR_H

#include "point.h"
#include "component.h"

class GameObject;
class SnakeBlock;
class Player;

class RoomMap;
class DeltaFrame;
class MoveProcessor;

class HorizontalStepProcessor {
public:
    HorizontalStepProcessor(MoveProcessor* mp, RoomMap*, DeltaFrame*, Player*, Point3, std::vector<GameObject*>&, std::vector<GameObject*>&);
    ~HorizontalStepProcessor();

    void run();

private:
	void move_free();
	void move_bound();
	void move_riding();

    void perform_horizontal_step();

    bool compute_push_component_tree(GameObject* block, bool dragged);
    bool compute_push_component(GameObject* block, bool dragged, std::vector<GameObject*>& weak_links);
	bool snake_drag_check(SnakeBlock* sb, std::vector<GameObject*>& weak_links);

    void collect_moving_and_weak_links(PushComponent* comp, std::vector<GameObject*>& weak_links);

	std::vector<std::unique_ptr<PushComponent>> push_comps_unique_{};
	std::vector<PushComponent*> orphaned_moving_comps_{};

	std::vector<SnakeBlock*> moving_snakes_{};
	std::vector<SnakeBlock*> strong_drags_{};
	std::vector<GameObject*> broken_weak_links_{};
    std::vector<GameObject*>& moving_blocks_;
    std::vector<GameObject*>& fall_check_;

	MoveProcessor* move_processor_;
    RoomMap* map_;
    DeltaFrame* delta_frame_;
	Player* player_;
    Point3 dir_;
};

#endif // HORIZONTALSTEPPROCESSOR_H
