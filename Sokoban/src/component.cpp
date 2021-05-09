#include "stdafx.h"
#include "component.h"

#include "gameobject.h"
#include "roommap.h"

Component::~Component() {
    for (GameObject* block : blocks_) {
        block->comp_ = nullptr;
    }
}


void PushComponent::add_pushing(Component* comp) {
    pushing_.push_back(static_cast<PushComponent*>(comp));
}


void FallComponent::add_above(Component* comp) {
    above_.push_back(static_cast<FallComponent*>(comp));
}

void FallComponent::settle_first() {
    settled_ = true;
    for (FallComponent* comp : above_) {
        if (!comp->settled_) {
            comp->settle_first();
        }
    }
}

void FallComponent::take_falling(RoomMap* map) {
    for (GameObject* block : blocks_) {
        map->take_from_map(block, false, false, true, nullptr);
    }
}
