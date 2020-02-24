#pragma once

#include "point.h"
#include "component.h"

class GameObject;
class SnakeBlock;
class Player;
class AnimationManager;

class RoomMap;
class DeltaFrame;

class JumpStepProcessor {
public:
	JumpStepProcessor(RoomMap*, DeltaFrame*, Player*, AnimationManager*, std::vector<GameObject*>&, std::vector<GameObject*>&);
	~JumpStepProcessor();

	void run();

private:
	void perform_jump();

	bool compute_push_component_tree(GameObject* block);
	bool compute_push_component(GameObject* block);
	void collect_moving_and_weak_links(PushComponent* comp, std::vector<GameObject*>& weak_links);

	std::vector<std::unique_ptr<PushComponent>> push_comps_unique_{};
	std::vector<SnakeBlock*> moving_snakes_{};
	std::vector<GameObject*> broken_weak_links_{};
	std::vector<GameObject*>& moving_blocks_;
	std::vector<GameObject*>& fall_check_;

	RoomMap* map_;
	DeltaFrame* delta_frame_;
	Player* player_;
	AnimationManager* anims_;
};