#include "stdafx.h"
#include "room.h"

#include "roommap.h"
#include "camera.h"

#include "graphicsmanager.h"
#include "gameobject.h"

#include "pushblock.h"
#include "snakeblock.h"
#include "gatebody.h"
#include "wall.h"
#include "player.h"

#include "car.h"
#include "door.h"
#include "gate.h"
#include "pressswitch.h"
#include "autoblock.h"
#include "puppetblock.h"

#include "switch.h"
#include "switchable.h"
#include "signaler.h"
#include "mapfile.h"

Room::Room(const std::string& name): name_ {name},
map_ {}, camera_ {}, offset_pos_ {0,0,0} {}

Room::~Room() {}

std::string const Room::name() {
    return name_;
}

void Room::initialize(GameObjectArray& objs, int w, int h, int d) {
    map_ = std::make_unique<RoomMap>(objs, w, h, d);
    camera_ = std::make_unique<Camera>(w, h);
}

void Room::set_cam_pos(Point3 vpos, FPoint3 rpos) {
	camera_->set_target(vpos, rpos);
    camera_->set_current_to_target();
}

bool Room::valid(Point3 pos) {
    return (map_ && map_->valid(pos));
}

RoomMap* Room::map() {
    return map_.get();
}

Camera* Room::camera() {
	return camera_.get();
}

#include <ctime>

void Room::draw(GraphicsManager* gfx, Point3 cam_pos, bool ortho, bool one_layer) {
    update_view(gfx, cam_pos, cam_pos, ortho);
    if (one_layer) {
        map_->draw_layer(gfx, cam_pos.z);
    } else {
        map_->draw(gfx, camera_->get_rotation());
    }
	gfx->draw();
}

void Room::draw(GraphicsManager* gfx, Player* player, bool ortho, bool one_layer) {
    update_view(gfx, player->pos_, player->cam_pos(), ortho);
    if (one_layer) {
        map_->draw_layer(gfx, player->pos_.z);
    } else {
        map_->draw(gfx, camera_->get_rotation());
    }
	gfx->draw();
}

void Room::update_view(GraphicsManager* gfx, Point3 vpos, FPoint3 rpos, bool ortho) {
    glm::mat4 model, view, projection;
    if (ortho) {
        camera_->set_target(vpos, vpos);
		camera_->set_current_to_target();
        view = glm::lookAt(glm::vec3(-rpos.x, rpos.y, rpos.z),
                           glm::vec3(-rpos.x, rpos.y, rpos.z - 1.0),
                           glm::vec3(0.0, -1.0, 0.0));
        projection = glm::ortho(-ORTHO_WIDTH/2.0, ORTHO_WIDTH/2.0, -ORTHO_HEIGHT/2.0, ORTHO_HEIGHT/2.0, -2.5, 2.5);
    } else {
        camera_->set_target(vpos, rpos);
        camera_->update();

        double cam_radius = camera_->get_radius();
		FPoint3 target_pos = camera_->get_pos() + FPoint3{ 0, 0, 0.5 };

        double cam_tilt = camera_->get_tilt();
        double cam_rotation = camera_->get_rotation();
        double cam_x = sin(cam_tilt) * sin(cam_rotation) * cam_radius;
		double cam_y = sin(cam_tilt) * cos(cam_rotation) * cam_radius;
        double cam_z = cos(cam_tilt) * cam_radius;
		
        view = glm::lookAt(glm::vec3(cam_x - target_pos.x, cam_y + target_pos.y, cam_z + target_pos.z),
                           glm::vec3(-target_pos.x, target_pos.y, target_pos.z),
                           glm::vec3(0.0, -1.0, 1.0));
        projection = glm::perspective(FOV_VERTICAL, ASPECT_RATIO, 0.1, 100.0);
    }
	view = glm::scale(view, glm::vec3(-1.0, 1.0, 1.0));
    gfx->set_PV(projection, view);
}

void Room::extend_by(Point3 d) {
    map_->extend_by(d);
}

void Room::shift_by(Point3 d) {
    map_->shift_by(d);
    // TODO: shift camera rects also?
}

void Room::write_to_file(MapFileO& file) {
    file << MapCode::Dimensions;
    file << map_->width_;
    file << map_->height_;
    file << map_->depth_;

    file << MapCode::OffsetPos;
    file << offset_pos_;

    map_->serialize(file);

    camera_->serialize(file);

    file << MapCode::End;
}

void Room::load_from_file(GameObjectArray& objs, MapFileI& file, Player** player_ptr) {
    unsigned char b[8];
    bool reading_file = true;
    while (reading_file) {
        file.read(b, 1);
        switch (static_cast<MapCode>(b[0])) {
        case MapCode::Dimensions:
            file.read(b, 3);
            initialize(objs, b[0], b[1], b[2]);
            break;
        case MapCode::FullLayer: // These codes are useless for now!
            //map_->push_full();
            break;
        case MapCode::SparseLayer:
            //map_->push_sparse();
            break;
        case MapCode::DefaultPos_DEFUNCT:
            file.read(b, 3);
            break;
        case MapCode::OffsetPos:
            file >> offset_pos_;
            break;
        case MapCode::Objects:
            read_objects(file, player_ptr);
            break;
        case MapCode::CameraRects:
            read_camera_rects(file);
            break;
        case MapCode::SnakeLink:
            read_snake_link(file);
            break;
        case MapCode::DoorDest:
            read_door_dest(file);
            break;
        case MapCode::Signaler:
            read_signaler(file);
            break;
        case MapCode::Walls:
            read_walls(file);
            break;
        case MapCode::End:
            reading_file = false;
            break;
        default :
            std::cout << "unknown state code! " << (int)b[0] << std::endl;
            //throw std::runtime_error("Unknown State code encountered in .map file (it's probably corrupt/an old version)");
            break;
        }
    }
}

#define CASE_OBJCODE(CLASS)\
case ObjCode::CLASS:\
    obj = CLASS::deserialize(file);\
    break;

#define CASE_MODCODE(CLASS)\
case ModCode::CLASS:\
    CLASS::deserialize(file, map_.get(), obj.get());\
    break;


void Room::read_objects(MapFileI& file, Player** player_ptr) {
    unsigned char b;
    std::unique_ptr<GameObject> obj {};
    while (true) {
        obj = nullptr;
        file.read(&b, 1);
        switch (static_cast<ObjCode>(b)) {
        CASE_OBJCODE(PushBlock)
        CASE_OBJCODE(SnakeBlock)
        CASE_OBJCODE(GateBody)
		CASE_OBJCODE(Wall)
        // Some Object types should never actually be serialized (as "Objects")
        case ObjCode::Player:
		{
			obj = Player::deserialize(file);
			if (player_ptr) {
				*player_ptr = static_cast<Player*>(obj.get());
			} else {
				// TODO: make this less fragile?
				file.read(&b, 1);
				continue;
			}
			break;
		}
        case ObjCode::NONE:
            return;
        default:
            throw std::runtime_error("Unknown Object code encountered in .map file (it's probably corrupt/an old version)");
			break;
        }
        file.read(&b, 1);
        switch (static_cast<ModCode>(b)) {
        CASE_MODCODE(Car)
        CASE_MODCODE(Door)
        CASE_MODCODE(Gate)
        CASE_MODCODE(PressSwitch)
        CASE_MODCODE(AutoBlock)
		CASE_MODCODE(PuppetBlock)
        case ModCode::NONE:
            break;
        default:
            throw std::runtime_error("Unknown Modifier code encountered in .map file (it's probably corrupt/an old version)");
            break;
        }
		map_->create(std::move(obj), nullptr);
    }
}

#undef CASE_OBJCODE

#undef CASE_MODCODE

#define CASE_CAMCODE(CLASS)\
case CameraCode::CLASS:\
    camera_->push_context(std::unique_ptr<CameraContext>(CLASS ## CameraContext::deserialize(file)));\
    break;

void Room::read_camera_rects(MapFileI& file) {
    unsigned char b[1];
    while (true) {
        file.read(b, 1);
        CameraCode code = static_cast<CameraCode>(b[0]);
        switch (code) {
        CASE_CAMCODE(Clamped)
        CASE_CAMCODE(Null)
        case CameraCode::NONE:
            return;
        default :
            throw std::runtime_error("Unknown Camera code encountered in .map file (it's probably corrupt/an old version)");
            return;
        }
    }
}

#undef CASE_CAMCODE

void Room::read_snake_link(MapFileI& file) {
    unsigned char b[4];
    file.read(b, 4);
    SnakeBlock* sb = static_cast<SnakeBlock*>(map_->view({b[0], b[1], b[2]}));
    // Linked right
    if (b[3] & 1) {
        sb->add_link_quiet(static_cast<SnakeBlock*>(map_->view({b[0]+1, b[1], b[2]})));
    }
    // Linked down
    if (b[3] & 2) {
        sb->add_link_quiet(static_cast<SnakeBlock*>(map_->view({b[0], b[1]+1, b[2]})));
    }
}

void Room::read_door_dest(MapFileI& file) {
    Point3 pos {file.read_point3()};
    Point3_S16 exit_pos;
    file >> exit_pos;
    auto door = static_cast<Door*>(map_->view(pos)->modifier());
	std::string exit_room = file.read_str();
	if (exit_room.empty()) {
		exit_room = name_;
	}
    door->set_data(exit_pos, name_, exit_room);
}

void Room::read_signaler(MapFileI& file) {
    unsigned char b[6];
    std::string label = file.read_str();
    // All signalers should have some sort of mnemonic
    // This forces the user of the editor to come up with names
    if (label.empty()) {
        label = "UNNAMED";
    }
    file.read(b, 6);
    auto signaler = std::make_unique<Signaler>(label, b[0], b[1], b[2], b[3]);
    for (int i = 0; i < b[4]; ++i) {
        signaler->push_switch_mutual(dynamic_cast<Switch*>(map_->view(file.read_point3())->modifier()));
    }
    for (int i = 0; i < b[5]; ++i) {
        signaler->push_switchable_mutual(dynamic_cast<Switchable*>(map_->view(file.read_point3())->modifier()));
    }
    map_->push_signaler(std::move(signaler));
}

void Room::read_walls(MapFileI& file) {
	unsigned int wall_count = file.read_uint32();
    Point3 pos;
    for (unsigned int i = 0; i < wall_count; ++i) {
        file >> pos;
		map_->create_wall(pos);
    }
}