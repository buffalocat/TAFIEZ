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

enum class PushResult {
	Blocked,
	CanMove,
	Defer,
};

using PushCompPair = std::pair<PushComponent*, PushComponent*>;

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
	PushResult compute_push_component(GameObject* block, bool dragged);
	bool snake_drag(SnakeBlock* sb);

    void collect_moving_and_weak_links(PushComponent* comp);

	std::vector<std::unique_ptr<PushComponent>> push_comps_unique_{};
	std::vector<PushComponent*> orphaned_moving_comps_{};
	std::vector<GameObject*> weak_links_{};
	std::vector<PushCompPair> deferred_{};

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
