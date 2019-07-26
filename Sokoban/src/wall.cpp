#include "stdafx.h"
#include "wall.h"

#include "graphicsmanager.h"
#include "texture_constants.h"
#include "mapfile.h"
#include "objectmodifier.h"

Wall::Wall() : Block({ 0,0,0 }, false, false) {}

Wall::Wall(Point3 pos) : Block(pos, false, false) {}

Wall::~Wall() {}

std::unique_ptr<GameObject> Wall::deserialize(MapFileI& file) {
	Point3 pos{ file.read_point3() };
	return std::make_unique<Wall>(pos);
}

std::string Wall::name() {
    return "Wall";
}

ObjCode Wall::obj_code() {
    return ObjCode::Wall;
}

void Wall::draw(GraphicsManager* gfx) {
	Point3 p = pos_;
	gfx->cube.push_instance(glm::vec3(p.x, p.y, p.z), glm::vec3(1.0f, 1.0f, 1.0f), (BlockTexture)20, GREY_VECTORS[p.z % NUM_GREYS]);
	if (modifier_) {
		modifier()->draw(gfx, p);
	}
}

// TODO: Replace with a batch drawing mechanism!
void Wall::draw(GraphicsManager* gfx, Point3 p) {
	gfx->cube.push_instance(glm::vec3(p.x, p.y, p.z), glm::vec3(1.0f, 1.0f, 1.0f), (BlockTexture)20, GREY_VECTORS[p.z % NUM_GREYS]);
}
