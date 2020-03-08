#ifndef FALLSTEPPROCESSOR_H
#define FALLSTEPPROCESSOR_H

#include "component.h"

class RoomMap;
class DeltaFrame;
class SnakeBlock;
class MoveProcessor;
class AnimationManager;

class FallStepProcessor {
public:
    FallStepProcessor(MoveProcessor* mp, RoomMap* map, DeltaFrame* delta_frame, std::vector<GameObject*>&& fall_check);
    ~FallStepProcessor();

    bool run(bool test);
    void check_land_first(FallComponent* comp);
    void collect_above(FallComponent* comp, std::vector<GameObject*>& above_list);
    bool drop_check(FallComponent* comp);
    void check_land_sticky(FallComponent* comp);
    void handle_fallen_blocks(FallComponent* comp);
    void settle(FallComponent* comp);

private:
	std::vector<std::unique_ptr<FallComponent>> fall_comps_unique_{};
    std::vector<GameObject*> fall_check_;

	MoveProcessor* move_processor_;
    RoomMap* map_;
    DeltaFrame* delta_frame_;
	AnimationManager* anims_;
    int layers_fallen_ = 0;
};

#endif // FALLSTEPPROCESSOR_H
