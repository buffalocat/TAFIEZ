#include "stdafx.h"
#include "room.h"

#include "roommap.h"
#include "maplayer.h"
#include "camera.h"

#include "graphicsmanager.h"
#include "fontmanager.h"
#include "stringdrawer.h"

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
#include "clearflag.h"
#include "worldresetkey.h"
#include "permanentswitch.h"
#include "floorsign.h"

#include "switch.h"
#include "switchable.h"
#include "signaler.h"
#include "mapfile.h"

#include "savefile.h"

Room::Room(GraphicsManager* gfx, std::string name) : gfx_{ gfx }, name_ { name } {}

Room::~Room() {}

std::string const Room::name() {
	return name_;
}

void Room::initialize(GameObjectArray& objs, PlayingGlobalData* global, int w, int h, int d) {
	map_ = std::make_unique<RoomMap>(objs, global, gfx_, w, h, d);
	camera_ = std::make_unique<Camera>(w, h);
}

void Room::set_cam_pos(Point3 vpos, FPoint3 rpos, bool display_labels, bool snap) {
	if (camera_->update_context(vpos) && display_labels) {
		if (camera_->update_label()) {
			context_label_ = std::make_unique<RoomLabelDrawer>(
				gfx_->fonts_->get_font(Fonts::KALAM_BOLD, 72), COLOR_VECTORS[DARK_BLUE],
				camera_->active_label_, 0.65f);
			gfx_->toggle_string_drawer(context_label_.get(), true);
		}
	}
	camera_->set_target(rpos);
	if (snap) {
		camera_->set_current_to_target();
	} else {
		camera_->update();
	}
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

void Room::draw_at_pos(Point3 pos, bool display_labels, bool ortho, bool one_layer) {
	draw(pos, pos, display_labels, ortho, one_layer);
}

void Room::draw_at_player(Player* player, bool display_labels, bool ortho, bool one_layer) {
	draw(player->pos_, player->cam_pos(), display_labels, ortho, one_layer);
}

void Room::draw(Point3 vpos, FPoint3 rpos, bool display_labels, bool ortho, bool one_layer) {
	gfx_->prepare_object_rendering();
	update_view(vpos, rpos, display_labels, ortho);
	if (one_layer) {
		map_->draw_layer(gfx_, vpos.z);
	} else {
		map_->draw(gfx_, camera_->get_rotation());
	}
	gfx_->draw_world();
	gfx_->draw_text();
}

void Room::update_view(Point3 vpos, FPoint3 rpos, bool display_labels, bool ortho) {
	glm::mat4 model, view, projection;
	if (ortho) {
		set_cam_pos(vpos, vpos, display_labels, true);
		gfx_->set_light_source(glm::vec3(rpos.x, rpos.y, rpos.z + 100));
		view = glm::lookAt(glm::vec3(-rpos.x, rpos.y, rpos.z),
			glm::vec3(-rpos.x, rpos.y, rpos.z - 1.0),
			glm::vec3(0.0, -1.0, 0.0));
		projection = glm::ortho(-ORTHO_WIDTH / 2.0, ORTHO_WIDTH / 2.0, -ORTHO_HEIGHT / 2.0, ORTHO_HEIGHT / 2.0, -2.5, 2.5);
	} else {
		set_cam_pos(vpos, rpos, display_labels, false);

		double cam_radius = camera_->get_radius();
		FPoint3 target_pos = camera_->get_pos() + FPoint3{ 0, 0, 0.5 };

		double cam_tilt = camera_->get_tilt();
		double cam_rotation = camera_->get_rotation();
		double cam_x = sin(cam_tilt) * sin(cam_rotation) * cam_radius;
		double cam_y = sin(cam_tilt) * cos(cam_rotation) * cam_radius;
		double cam_z = cos(cam_tilt) * cam_radius;

		gfx_->set_light_source(glm::vec3(cam_x + target_pos.x, cam_y + target_pos.y, cam_z + target_pos.z));

		view = glm::lookAt(glm::vec3(cam_x - target_pos.x, cam_y + target_pos.y, cam_z + target_pos.z),
			glm::vec3(-target_pos.x, target_pos.y, target_pos.z),
			glm::vec3(0.0, -1.0, 1.0));
		projection = glm::perspective(FOV_VERTICAL, ASPECT_RATIO, 0.1, 100.0);
	}
	view = glm::scale(view, glm::vec3(-1.0, 1.0, 1.0));
	gfx_->set_PV(projection * view);
}

void Room::shift_by(Point3 d) {
	map_->shift_by(d);
	camera_->shift_by(d);
}

void Room::extend_by(Point3 d) {
	map_->extend_by(d);
	camera_->extend_by(d);
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

void Room::load_from_file(GameObjectArray& objs, MapFileI& file, PlayingGlobalData* global, Player** player_ptr) {
	unsigned char b[8];
	bool reading_file = true;
	while (reading_file) {
		file.read(b, 1);
		switch (static_cast<MapCode>(b[0])) {
		case MapCode::Dimensions:
			file.read(b, 3);
			initialize(objs, global, b[0], b[1], b[2]);
			break;
		case MapCode::InitFlag:
			map_->inited_ = true;
			break;
		case MapCode::Zone:
			map_->zone_ = file.read_byte();
			zone_label_ = std::make_unique<RoomLabelDrawer>(
				gfx_->fonts_->get_font(Fonts::KALAM_BOLD, 108), COLOR_VECTORS[BRIGHT_PURPLE],
				std::string("Zone ") + map_->zone_, 0.8f);
			break;
		case MapCode::ClearFlagRequirement:
			map_->clear_flag_req_ = file.read_byte();
			map_->clear_id_ = file.read_uint32();
			if (global && global->has_flag(map_->clear_id_)) {
				map_->collect_flag();
			}
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
		case MapCode::ThresholdSignaler:
			read_threshold_signaler(file);
			break;
		case MapCode::ParitySignaler:
			read_parity_signaler(file);
			break;
		case MapCode::WallRuns:
			read_wall_runs(file);
			break;
		case MapCode::WallPositions:
			read_wall_positions(file);
			break;
		case MapCode::End:
			reading_file = false;
			break;
		default:
			std::cout << "unknown state code! " << (int)b[0] << std::endl;
			break;
		}
	}
}

#define CASE_OBJCODE(CLASS)\
case ObjCode::CLASS:\
    obj = CLASS::deserialize(file);\
    break

#define CASE_MODCODE(CLASS)\
case ModCode::CLASS:\
    CLASS::deserialize(file, map_.get(), obj.get());\
    break


void Room::read_objects(MapFileI& file, Player** player_ptr) {
	unsigned char b;
	std::unique_ptr<GameObject> obj{};
	while (true) {
		obj = nullptr;
		file.read(&b, 1);
		switch (static_cast<ObjCode>(b)) {
			CASE_OBJCODE(PushBlock);
			CASE_OBJCODE(SnakeBlock);
			CASE_OBJCODE(GateBody);
			CASE_OBJCODE(Wall);
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
			CASE_MODCODE(Car);
			CASE_MODCODE(Door);
			CASE_MODCODE(Gate);
			CASE_MODCODE(PressSwitch);
			CASE_MODCODE(AutoBlock);
			CASE_MODCODE(PuppetBlock);
			CASE_MODCODE(ClearFlag);
			CASE_MODCODE(WorldResetKey);
			CASE_MODCODE(PermanentSwitch);
			CASE_MODCODE(FloorSign);
		case ModCode::NONE:
			break;
		default:
			throw std::runtime_error("Unknown Modifier code encountered in .map file (it's probably corrupt/an old version)");
			break;
		}
		auto* obj_raw = obj.get();
		map_->create_in_map(std::move(obj), false, nullptr);
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
		default:
			throw std::runtime_error("Unknown Camera code encountered in .map file (it's probably corrupt/an old version)");
			return;
		}
	}
}

#undef CASE_CAMCODE

void Room::read_snake_link(MapFileI& file) {
	unsigned char b[4];
	file.read(b, 4);
	SnakeBlock* sb = static_cast<SnakeBlock*>(map_->view({ b[0], b[1], b[2] }));
	// Linked right
	if (b[3] & 1) {
		sb->add_link_quiet(static_cast<SnakeBlock*>(map_->view({ b[0] + 1, b[1], b[2] })));
	}
	// Linked down
	if (b[3] & 2) {
		sb->add_link_quiet(static_cast<SnakeBlock*>(map_->view({ b[0], b[1] + 1, b[2] })));
	}
}

void Room::read_door_dest(MapFileI& file) {
	Point3 pos{ file.read_point3() };
	Point3_S16 exit_pos;
	file >> exit_pos;
	auto door = static_cast<Door*>(map_->view(pos)->modifier());
	std::string exit_room = file.read_str();
	if (exit_room.empty()) {
		exit_room = name_;
	}
	door->set_data(exit_pos, name_, exit_room);
}

void Room::read_threshold_signaler(MapFileI& file) {
	unsigned char b[4];
	std::string label = file.read_str();
	// All signalers should have some sort of mnemonic
	// This forces the user of the editor to come up with names
	if (label.empty()) {
		label = "UNNAMED";
	}
	file.read(b, 4);
	auto signaler = std::make_unique<ThresholdSignaler>(label, b[0], b[1]);
	for (int i = 0; i < b[2]; ++i) {
		signaler->push_switch(dynamic_cast<Switch*>(map_->view(file.read_point3())->modifier()), true);
	}
	for (int i = 0; i < b[3]; ++i) {
		signaler->push_switchable(dynamic_cast<Switchable*>(map_->view(file.read_point3())->modifier()), true, 0);
	}
	map_->push_signaler(std::move(signaler));
}

void Room::read_parity_signaler(MapFileI& file) {
	unsigned char b[4];
	std::string label = file.read_str();
	// All signalers should have some sort of mnemonic
	// This forces the user of the editor to come up with names
	if (label.empty()) {
		label = "UNNAMED";
	}
	file.read(b, 4);
	int parity_level = b[1];
	auto signaler = std::make_unique<ParitySignaler>(label, b[0], b[1], b[2]);
	for (int i = 0; i < b[3]; ++i) {
		signaler->push_switch(dynamic_cast<Switch*>(map_->view(file.read_point3())->modifier()), true);
	}
	for (int i = 0; i < parity_level; ++i) {
		int n_swbles = file.read_byte();
		for (int j = 0; j < n_swbles; ++j) {
			signaler->push_switchable(dynamic_cast<Switchable*>(map_->view(file.read_point3())->modifier()), true, i);
		}
	}
	map_->push_signaler(std::move(signaler));
}

void Room::read_wall_positions(MapFileI& file) {
	unsigned int wall_count = file.read_uint32();
	Point3 pos;
	for (unsigned int i = 0; i < wall_count; ++i) {
		file >> pos;
		map_->create_wall(pos);
	}
}

void Room::read_wall_runs(MapFileI& file) {
	for (auto& layer : map_->layers_) {
		layer.deserialize_wall_runs(file);
	}
}