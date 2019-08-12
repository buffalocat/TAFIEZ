#include "stdafx.h"
#include "pushblock.h"

#include "roommap.h"
#include "mapfile.h"
#include "graphicsmanager.h"
#include "texture_constants.h"

#include "objectmodifier.h"
#include "autoblock.h"
#include "car.h"

PushBlock::PushBlock(Point3 pos, int color, bool pushable, bool gravitable, Sticky sticky):
ColoredBlock(pos, color, pushable, gravitable), sticky_ {sticky} {}

PushBlock::~PushBlock() {}

std::string PushBlock::name() {
    return "PushBlock";
}

ObjCode PushBlock::obj_code() {
    return ObjCode::PushBlock;
}

void PushBlock::serialize(MapFileO& file) {
    file << color_ << pushable_ << gravitable_ << sticky_;
}

std::unique_ptr<GameObject> PushBlock::deserialize(MapFileI& file) {
    Point3 pos {file.read_point3()};
    unsigned char b[4];
    file.read(b, 4);
    return std::make_unique<PushBlock>(pos, b[0], b[1], b[2], static_cast<Sticky>(b[3]));
}

void PushBlock::collect_sticky_links(RoomMap* map, Sticky sticky_level, std::vector<GameObject*>& links) {
    Sticky sticky_condition = sticky_ & sticky_level;
    if (sticky_condition != Sticky::None) {
        for (Point3 d : DIRECTIONS) {
            PushBlock* adj = dynamic_cast<PushBlock*>(map->view(pos_ + d));
            if (adj && adj->color() == color() && ((adj->sticky_ & sticky_condition) != Sticky::None)) {
                links.push_back(adj);
            }
        }
    }
}

Sticky PushBlock::sticky() {
    return sticky_;
}

void PushBlock::draw(GraphicsManager* gfx) {
    FPoint3 p {real_pos()};
    BlockTexture tex {BlockTexture::Blank};
    switch (sticky_) {
    case Sticky::None:
        tex = BlockTexture::Edges;
        break;
    case Sticky::Weak:
        tex = BlockTexture::BrokenEdges;
        break;
    case Sticky::Strong:
        tex = BlockTexture::LightEdges;
        break;
    case Sticky::AllStick:
        tex = BlockTexture::Corners;
        break;
    }
	if (modifier_) {
		tex = tex | modifier_->texture();
	}
	gfx->cube.push_instance(glm::vec3(p.x, p.y, p.z), glm::vec3(1.0f, 1.0f, 1.0f), tex, color());
    draw_force_indicators(gfx, p, 1.1f);
	if (auto* car = dynamic_cast<Car*>(modifier())) {
		
	}
    if (modifier_) {
        modifier()->draw(gfx, p);
    }
}

void PushBlock::draw_force_indicators(GraphicsManager* gfx, FPoint3 p, double radius) {
	if (!pushable_) {
		gfx->cube.push_instance(glm::vec3(p.x, p.y, p.z - 0.2f), glm::vec3(radius, radius, 0.1f), BlockTexture::Blank, BLACK);
	}
	if (!gravitable_) {
		gfx->cube.push_instance(glm::vec3(p.x, p.y, p.z + 0.2f), glm::vec3(radius, radius, 0.1f), BlockTexture::Blank, WHITE);
	}
}
