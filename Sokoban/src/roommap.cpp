#include "stdafx.h"
#include "roommap.h"

#include "playingstate.h"
#include "gameobjectarray.h"
#include "gameobject.h"
#include "graphicsmanager.h"
#include "animationmanager.h"
#include "soundmanager.h"
#include "wall.h"
#include "delta.h"
#include "snakeblock.h"
#include "player.h"
#include "switch.h"
#include "signaler.h"
#include "mapfile.h"
#include "objectmodifier.h"
#include "maplayer.h"
#include "effects.h"
#include "moveprocessor.h"
#include "common_constants.h"
#include "clearflag.h"
#include "door.h"
#include "car.h"
#include "incinerator.h"
#include "flaggate.h"
#include "savefile.h"
#include "globalflagconstants.h"
#include "gate.h"
#include "gatebody.h"

RoomMap::RoomMap(GameObjectArray& obj_array, GameState* state,
	int width, int height, int depth) :
	obj_array_{ obj_array }, state_{ state },
	width_{ width }, height_{ height }, depth_{ depth },
	player_cycle_{ std::make_unique<PlayerCycle>() } {
	if (auto* ps = dynamic_cast<PlayingState*>(state)) {
		global_ = ps->global_;
	}
	for (int z = 0; z < depth; ++z) {
		layers_.push_back(MapLayer(this, width_, height_, z));
	}
	door_groups_[0].clear();
}

RoomMap::~RoomMap() {}

bool RoomMap::valid(Point3 pos) {
	return (0 <= pos.x) && (pos.x < width_) && (0 <= pos.y) && (pos.y < height_) && (0 <= pos.z) && (pos.z < depth_);
}

struct ObjectSerializationHandler {
	void operator()(int);

	GameObjectArray& obj_array;
	MapFileO& file;
	std::vector<GameObject*>& rel_check_objs;
	std::vector<ObjectModifier*>& rel_check_mods;
};

void ObjectSerializationHandler::operator()(int id) {
	if (id == GENERIC_WALL_ID) {
		return;
	}
	GameObject* obj = obj_array[id];
	// NOTE: a MapLayer should never pass an invalid ( = 0) id here.
	// And a nonzero id in the map should correspond to a "real" object
	// (i.e., it should exist, and not be in a destroyed state)
	if (obj->skip_serialization()) {
		return;
	}
	file << obj->obj_code();
	file << obj->pos_;
	obj->serialize(file);
	if (ObjectModifier* mod = obj->modifier()) {
		file << mod->mod_code();
		mod->serialize(file);
		if (mod->relation_check()) {
			rel_check_mods.push_back(mod);
		}
	} else {
		file << ModCode::NONE;
	}
	if (obj->relation_check()) {
		rel_check_objs.push_back(obj);
	}
}

void RoomMap::serialize(MapFileO& file) {
	// Metadata
	if (inited_) {
		file << MapCode::InitFlag;
	}

	file << MapCode::Zone;
	file << zone_;
	// Serialize layer types
	std::vector<GameObject*> rel_check_objs{};
	std::vector<ObjectModifier*> rel_check_mods{};
	GameObjIDFunc ser_handler = ObjectSerializationHandler{ obj_array_, file, rel_check_objs, rel_check_mods };
	// Serialize raw object data
	file << MapCode::Objects;
	for (auto& layer : layers_) {
		layer.apply_to_rect(MapRect{ 0,0,width_,height_ }, ser_handler);
	}
	file << ObjCode::NONE;
	// Serialize relational data
	for (auto obj : rel_check_objs) {
		obj->relation_serialize(file);
	}
	for (auto mod : rel_check_mods) {
		mod->relation_serialize(file);
	}
	// Serialize Signalers
	for (auto& signaler : signalers_) {
		signaler->serialize(file);
	}
	file << MapCode::WallRuns;
	for (auto& layer : layers_) {
		layer.serialize_wall_runs(file);
	}

	if (clear_flag_req_ > 0) {
		file << MapCode::ClearFlagRequirement;
		file << clear_flag_req_;
		file.write_uint32(clear_id_);
	}
}

int& RoomMap::at(Point3 pos) {
	return layers_[pos.z].at(pos.h());
}

// Pretend that every out-of-bounds "object" is a Wall, unless it's below the map
GameObject* RoomMap::view(Point3 pos) {
	if (pos.z < 0) {
		return nullptr;
	} else if (valid(pos)) {
		return obj_array_[layers_[pos.z].at(pos.h())];
	} else {
		return obj_array_[GENERIC_WALL_ID];
	}
}

void RoomMap::push_to_object_array(std::unique_ptr<GameObject> obj_unique, DeltaFrame* delta_frame) {
	GameObject* obj = obj_unique.get();
	obj_array_.push_object(std::move(obj_unique));
	if (delta_frame) {
		delta_frame->push(std::make_unique<ObjArrayPushDelta>(obj));
	}
}

void RoomMap::create_in_map(std::unique_ptr<GameObject> obj_unique, bool activate_listeners, DeltaFrame* delta_frame) {
	GameObject* obj = obj_unique.get();
	push_to_object_array(std::move(obj_unique), delta_frame);
	put_in_map(obj, true, activate_listeners, delta_frame);
}

GameObject* RoomMap::deref_object(ObjRefCode ref_code, Point3 pos) {
	auto* base_obj = view(pos);
	switch (ref_code) {
	case ObjRefCode::Tangible:
		return base_obj;
	case ObjRefCode::HeldPlayer:
		return dynamic_cast<Car*>(base_obj->modifier())->player_;
	case ObjRefCode::HeldGateBody:
		return dynamic_cast<Gate*>(base_obj->modifier())->body_;
	default:
		return nullptr;
	}
}

void RoomMap::remove_from_object_array(GameObject* obj) {
	obj_array_.schedule_uncreation(obj);
}

void RoomMap::push_to_object_array_deleted(GameObject* obj, DeltaFrame* delta_frame) {
	obj_array_.add_dead_obj(obj);
	if (delta_frame) {
		delta_frame->push(std::make_unique<ObjArrayDeletedPushDelta>(obj));
	}
}

void RoomMap::remove_from_object_array_deleted(GameObject* obj) {
	obj_array_.schedule_undeletion(obj);
}

void RoomMap::put_in_map(GameObject* obj, bool real, bool activate_listeners, DeltaFrame* delta_frame) {
	at(obj->pos_) += obj->id_;
	obj->tangible_ = true;
	obj->setup_on_put(this, delta_frame, real);
	if (activate_listeners) {
		activate_listeners_at(obj->pos_);
	}
	if (real && delta_frame) {
		delta_frame->push(std::make_unique<PutDelta>(obj));
	}
}

void RoomMap::take_from_map(GameObject* obj, bool real, bool activate_listeners, DeltaFrame* delta_frame) {
	if (activate_listeners) {
		activate_listeners_at(obj->pos_);
	}
	obj->cleanup_on_take(this, delta_frame, real);
	obj->tangible_ = false;
	at(obj->pos_) -= obj->id_;
	if (real && delta_frame) {
		delta_frame->push(std::make_unique<TakeDelta>(obj));
	}
}

void RoomMap::create_wall(Point3 pos) {
	at(pos) = GENERIC_WALL_ID;
}

void RoomMap::clear(Point3 pos) {
	at(pos) = 0;
}

void RoomMap::shift(GameObject* obj, Point3 dpos, bool activate_listeners, DeltaFrame* delta_frame) {
	take_from_map(obj, false, activate_listeners, nullptr);
	obj->abstract_shift(dpos);
	put_in_map(obj, false, activate_listeners, nullptr);
	if (delta_frame) {
		delta_frame->push(std::make_unique<MotionDelta>(obj, dpos));
	}
}

void RoomMap::batch_shift(std::vector<GameObject*> objs, Point3 dpos, bool activate_listeners, DeltaFrame* delta_frame) {
	for (auto obj : objs) {
		take_from_map(obj, false, activate_listeners, nullptr);
		obj->abstract_shift(dpos);
		put_in_map(obj, false, activate_listeners, nullptr);
	}
	if (delta_frame) {
		delta_frame->push(std::make_unique<BatchMotionDelta>(std::move(objs), dpos));
	}
}

void RoomMap::batch_shift_frozen(std::vector<FrozenObject> objs, Point3 dpos) {
	for (auto& f_obj : objs) {
		auto* obj = f_obj.resolve(this);
		take_from_map(obj, false, false, nullptr);
		obj->abstract_shift(dpos);
		put_in_map(obj, false, false, nullptr);
	}
}

void RoomMap::add_door(Door* door) {
	unsigned int id = door->door_id_;
	if (id > 0) {
		door_groups_[id].push_back(door);
	}
}

void RoomMap::remove_door(Door* door) {
	unsigned int id = door->door_id_;
	if (id > 0) {
		auto& group = door_groups_[door->door_id_];
		if (group.size() <= 1) {
			door_groups_.erase(id);
		} else {
			group.erase(std::remove(group.begin(), group.end(), door), group.end());
		}
	}
}

unsigned int RoomMap::get_unused_door_id() {
	unsigned int i = (unsigned int)door_groups_.size();
	while (door_groups_.count(i)) {
		--i;
	}
	return i;
}

// TODO: consider making these sets instead of vectors
void RoomMap::remove_auto(AutoBlock* obj) {
	autos_.erase(std::remove(autos_.begin(), autos_.end(), obj), autos_.end());
}

void RoomMap::remove_puppet(PuppetBlock* obj) {
	puppets_.erase(std::remove(puppets_.begin(), puppets_.end(), obj), puppets_.end());
}

void RoomMap::remove_clear_flag(ClearFlag* obj) {
	clear_flags_.erase(std::remove(clear_flags_.begin(), clear_flags_.end(), obj), clear_flags_.end());
}

void RoomMap::add_listener(ObjectModifier* obj, Point3 pos) {
	listeners_[pos].push_back(obj);
}

void RoomMap::remove_listener(ObjectModifier* obj, Point3 pos) {
	auto& cur_lis = listeners_[pos];
	cur_lis.erase(std::remove(cur_lis.begin(), cur_lis.end(), obj), cur_lis.end());
	if (cur_lis.size() == 0) {
		listeners_.erase(pos);
	}
}

void RoomMap::activate_listener_of(ObjectModifier* obj) {
	activated_listeners_.insert(obj);
}

void RoomMap::activate_listeners_at(Point3 pos) {
	auto& cur_lis = listeners_[pos];
	if (!cur_lis.empty()) {
		activated_listeners_.insert(cur_lis.begin(), cur_lis.end());
	}
}

void RoomMap::alert_activated_listeners(DeltaFrame* delta_frame, MoveProcessor* mp) {
	for (ObjectModifier* obj : activated_listeners_) {
		obj->map_callback(this, delta_frame, mp);
	}
}

void RoomMap::handle_moved_cars(MoveProcessor* mp) {
	for (Car* car : moved_cars_) {
		car->handle_movement(this, mp->delta_frame_, mp);
	}
	moved_cars_.clear();
}

PlayingState* RoomMap::playing_state() {
	return static_cast<PlayingState*>(state_);
}

struct ObjectDrawer {
	void operator()(unsigned int, Point3);

	GameObjectArray& obj_array;
	GraphicsManager* gfx;
};

void ObjectDrawer::operator()(unsigned int id, Point3 pos) {
	if (id > GENERIC_WALL_ID) {
		obj_array[id]->draw(gfx);
	} else {
		Wall::draw(gfx, pos);
	}
}

void RoomMap::draw(GraphicsManager* gfx, double angle) {
	GameObjIDPosFunc drawer = ObjectDrawer{ obj_array_, gfx };
	// TODO: use a smarter map rectangle!
	for (auto it = layers_.rbegin(); it != layers_.rend(); ++it) {
		it->apply_to_rect_with_pos(MapRect{ 0, 0, width_, height_ }, drawer);
	}
}

void RoomMap::draw_layer(GraphicsManager* gfx, int z) {
	GameObjIDPosFunc drawer = ObjectDrawer{ obj_array_, gfx };
	layers_[z].apply_to_rect_with_pos(MapRect{ 0,0,width_,height_ }, drawer);
}

struct ObjectShifter {
	void operator()(unsigned int);

	GameObjectArray& obj_array;
	RoomMap* map;
	Point3 dpos;
};

void ObjectShifter::operator()(unsigned int id) {
	if (id > GENERIC_WALL_ID) {
		obj_array[id]->abstract_shift(dpos);
	}
}

void RoomMap::shift_all_objects(Point3 d) {
	GameObjIDFunc shifter = ObjectShifter{ obj_array_, this, d };
	for (auto& layer : layers_) {
		layer.apply_to_rect(MapRect{ 0,0,width_,height_ }, shifter);
	}
}

struct ObjectDestroyer {
	void operator()(unsigned int);

	GameObjectArray& obj_array;
	RoomMap* map;
};

void ObjectDestroyer::operator()(unsigned int id) {
	if (id > GENERIC_WALL_ID) {
		auto* obj = obj_array[id];
		map->take_from_map(obj, true, false, nullptr);
		map->remove_from_object_array(obj);
	}
}

void RoomMap::extend_by(Point3 d) {
	GameObjIDFunc destroyer = ObjectDestroyer{ obj_array_, this };
	if (d.z < 0) {
		for (int i = (int)layers_.size() - 1; i >= layers_.size() + d.z; --i) {
			layers_[i].apply_to_rect(MapRect{ 0,0,width_,height_ }, destroyer);
		}
		layers_.erase(layers_.end() + d.z, layers_.end());
		for (auto& layer : layers_) {
			layer.z_ += d.z;
		}
	}
	if (d.y < 0) {
		for (auto& layer : layers_) {
			layer.apply_to_rect(MapRect{ 0, height_ + d.y, width_, -d.y }, destroyer);
		}
	}
	if (d.x < 0) {
		for (auto& layer : layers_) {
			layer.apply_to_rect(MapRect{ width_ + d.x, 0, -d.x, height_ }, destroyer);
		}
	}
	height_ += d.y;
	width_ += d.x;
	for (auto& layer : layers_) {
		layer.extend_by(d.x, d.y);
	}
	for (int i = 0; i < d.z; ++i) {
		layers_.insert(layers_.end(), MapLayer(this, width_, height_, depth_ + i));
	}
	depth_ += d.z;
}

void RoomMap::shift_by(Point3 d) {
	GameObjIDFunc destroyer = ObjectDestroyer{ obj_array_, this };
	// First clean up objects if necessary, then actually shift the map
	if (d.z < 0) {
		for (int i = 0; i < -d.z; ++i) {
			layers_[i].apply_to_rect(MapRect{ 0,0,width_,height_ }, destroyer);
		}
		layers_.erase(layers_.begin(), layers_.begin() - d.z);
	}
	if (d.y < 0) {
		for (auto& layer : layers_) {
			layer.apply_to_rect(MapRect{ 0,0,width_,-d.y }, destroyer);
		}
	}
	if (d.x < 0) {
		for (auto& layer : layers_) {
			layer.apply_to_rect(MapRect{ 0,0,-d.x,height_ }, destroyer);
		}
	}
	width_ += d.x;
	height_ += d.y;
	depth_ += d.z;
	for (auto& layer : layers_) {
		layer.shift_by(d.x, d.y, d.z);
	}
	for (int i = d.z - 1; i >= 0; --i) {
		layers_.insert(layers_.begin(), MapLayer(this, width_, height_, i));
	}
	shift_all_objects(d);
}

struct RoomStateInitializer {
	void operator()(int id);

	GameObjectArray& obj_array;
	MoveProcessor* mp;
	RoomMap* map;
	DeltaFrame* delta_frame;
};

void RoomStateInitializer::operator()(int id) {
	GameObject* obj = obj_array[id];
	if (obj->gravitable_) {
		mp->add_to_fall_check(obj);
	}
	if (SnakeBlock* sb = dynamic_cast<SnakeBlock*>(obj)) {
		sb->check_add_local_links(map, delta_frame);
	} else if (Player* player = dynamic_cast<Player*>(obj)) {
		player->validate_state(map, delta_frame);
	}
	if (ObjectModifier* mod = obj->modifier()) {
		map->activate_listener_of(mod);
	}
}

void RoomMap::set_initial_state(PlayingState* playing_state) {
	if (!inited_) {
		DeltaFrame df{};
		MoveProcessor mp = MoveProcessor(playing_state, this, &df, nullptr, false);
		GameObjIDFunc state_initializer = RoomStateInitializer{ obj_array_, &mp, this, &df };
		for (auto& layer : layers_) {
			layer.apply_to_rect(MapRect{ 0,0,width_,height_ }, state_initializer);
		}
		// In editor mode, don't check switches or gravity.
		if (!playing_state) {
			return;
		}
		for (auto& sig : signalers_) {
			sig->check_send_initial(this, &df, &mp);
		}
		mp.perform_switch_checks(false);
		inited_ = true;
		while (!mp.update());
	}
	if (playing_state) {
		initialize_animation(playing_state->anims_.get());
		playing_state->anims_->sounds_->flush_sounds();
	}
}

struct AnimationInitializer {
	void operator()(int id);

	GameObjectArray& obj_array;
	RoomMap* map;
	AnimationManager* anims;
};

void AnimationInitializer::operator()(int id) {
	GameObject* obj = obj_array[id];
	if (auto* mod = obj->modifier()) {
		mod->signal_animation(anims, nullptr);
	}
}

void RoomMap::initialize_animation(AnimationManager* anims) {
	GameObjIDFunc state_initializer = AnimationInitializer{ obj_array_, this, anims };
	for (auto& layer : layers_) {
		layer.apply_to_rect(MapRect{ 0,0,width_,height_ }, state_initializer);
	}
	anims->reset_temp();
}

struct SnakeInitializer {
	void operator()(int id);

	GameObjectArray& obj_array;
	RoomMap* map;
	DeltaFrame* delta_frame;
};

void SnakeInitializer::operator()(int id) {
	if (SnakeBlock* sb = dynamic_cast<SnakeBlock*>(obj_array[id])) {
		sb->check_add_local_links(map, delta_frame);
	}
}

// This function does just one of the things that set_initial_state does
// But it's useful for making the SnakeTab convenient!
void RoomMap::initialize_automatic_snake_links() {
	DeltaFrame dummy_df{};
	GameObjIDFunc snake_initializer = SnakeInitializer{ obj_array_, this, &dummy_df };
	for (auto& layer : layers_) {
		layer.apply_to_rect(MapRect{ 0,0,width_,height_ }, snake_initializer);
	}
}

void RoomMap::reset_local_state() {
	activated_listeners_.clear();
}

void RoomMap::push_signaler(std::unique_ptr<Signaler> signaler) {
	signalers_.push_back(std::move(signaler));
}

// It's probably more efficient to just check all signalers than to keep track
void RoomMap::check_signalers(DeltaFrame* delta_frame, MoveProcessor* mp) {
	for (auto& signaler : signalers_) {
		signaler->check_send_signal(this, delta_frame, mp);
	}
}

void RoomMap::remove_signaler(Signaler* rem) {
	signalers_.erase(std::remove_if(signalers_.begin(), signalers_.end(),
		[rem](std::unique_ptr<Signaler>& sig) {return sig.get() == rem; }), signalers_.end());
}

void RoomMap::check_clear_flag_collected(DeltaFrame* delta_frame) {
	if (!global_->has_flag(get_clear_flag_code(zone_)) && clear_flags_changed_) {
		int total = 0;
		for (auto* flag : clear_flags_) {
			total += flag->active_;
		}
		if (total >= clear_flag_req_) {
			collect_flag(true, delta_frame);
			if (global_) {
				global_->collect_clear_flag(zone_, delta_frame);
				playing_state()->anims_->start_flag_cutscene(global_, zone_);
			}
		}
	}
}

void RoomMap::collect_flag(bool real, DeltaFrame* delta_frame) {
	if (delta_frame) {
		delta_frame->push(std::make_unique<ClearFlagCollectionDelta>());
	}
	if (real) {
		playing_state()->anims_->sounds_->queue_sound(SoundName::FlagGet);
	}
	for (auto& flag : clear_flags_) {
		flag->collected_ = true;
	}
}

void RoomMap::uncollect_flag() {
	for (auto& flag : clear_flags_) {
		flag->collected_ = false;
	}
}

void RoomMap::validate_players(DeltaFrame* delta_frame) {
	for (auto* player : player_cycle_->players_) {
		player->validate_state(this, delta_frame);
	}
}

PlayerCycle* RoomMap::player_cycle() {
	return player_cycle_.get();
}

TextRenderer* RoomMap::text_renderer() {
	return state_->text_.get();
}

std::vector<Door*>& RoomMap::door_group(unsigned int id) {
	if (door_groups_.count(id)) {
		return door_groups_[id];
	} else {
		return door_groups_[0];
	}
}

std::vector<Player*>& RoomMap::player_list() {
	return player_cycle_->players_;
}


PutDelta::PutDelta(GameObject* obj) :
	obj_{ obj } {}

PutDelta::PutDelta(FrozenObject obj) :
	obj_{ obj } {}

PutDelta::~PutDelta() {}

void PutDelta::serialize(MapFileO& file, GameObjectArray* arr) {
	obj_.serialize(file, arr);
}

void PutDelta::revert(RoomMap* room_map) {
	room_map->take_from_map(obj_.resolve(room_map), true, false, nullptr);
}

DeltaCode PutDelta::code() {
	return DeltaCode::PutDelta;
}

std::unique_ptr<Delta> PutDelta::deserialize(MapFileIwithObjs& file) {
	return std::make_unique<PutDelta>(file.read_frozen_obj());
}


TakeDelta::TakeDelta(GameObject* obj) :
	obj_{ obj } {}

TakeDelta::TakeDelta(FrozenObject obj) :
	obj_{ obj } {}

TakeDelta::~TakeDelta() {}

void TakeDelta::serialize(MapFileO& file, GameObjectArray* arr) {
	obj_.serialize(file, arr);
}

void TakeDelta::revert(RoomMap* room_map) {
	room_map->put_in_map(obj_.resolve(room_map), true, false, nullptr);
}

DeltaCode TakeDelta::code() {
	return DeltaCode::TakeDelta;
}

std::unique_ptr<Delta> TakeDelta::deserialize(MapFileIwithObjs& file) {
	return std::make_unique<TakeDelta>(file.read_frozen_obj());
}


ObjArrayPushDelta::ObjArrayPushDelta(GameObject* obj) : obj_{ obj } {}

ObjArrayPushDelta::ObjArrayPushDelta(FrozenObject obj) : obj_{ obj } {}

ObjArrayPushDelta::~ObjArrayPushDelta() {}

void ObjArrayPushDelta::serialize(MapFileO& file, GameObjectArray* arr) {
	obj_.serialize(file, arr);
}

void ObjArrayPushDelta::revert(RoomMap* room_map) {
	room_map->remove_from_object_array(obj_.resolve(room_map));
}

DeltaCode ObjArrayPushDelta::code() {
	return DeltaCode::ObjArrayPushDelta;
}

std::unique_ptr<Delta> ObjArrayPushDelta::deserialize(MapFileIwithObjs& file) {
	return std::make_unique<ObjArrayPushDelta>(file.read_frozen_obj());
}


ObjArrayDeletedPushDelta::ObjArrayDeletedPushDelta(GameObject* obj) : obj_{ obj } {}

ObjArrayDeletedPushDelta::ObjArrayDeletedPushDelta(FrozenObject obj) : obj_{ obj } {}

ObjArrayDeletedPushDelta::~ObjArrayDeletedPushDelta() {}

void ObjArrayDeletedPushDelta::serialize(MapFileO& file, GameObjectArray* arr) {
	obj_.serialize(file, arr);
}

void ObjArrayDeletedPushDelta::revert(RoomMap* room_map) {
	room_map->remove_from_object_array_deleted(obj_.resolve(room_map));
}

DeltaCode ObjArrayDeletedPushDelta::code() {
	return DeltaCode::ObjArrayDeletedPushDelta;
}

std::unique_ptr<Delta> ObjArrayDeletedPushDelta::deserialize(MapFileIwithObjs& file) {
	return std::make_unique<ObjArrayDeletedPushDelta>(file.read_frozen_obj());
}


MotionDelta::MotionDelta(GameObject* obj, Point3 dpos) :
	obj_{ obj }, dpos_{ dpos } {}

MotionDelta::MotionDelta(FrozenObject obj, Point3 dpos) :
	obj_{ obj }, dpos_{ dpos } {}

MotionDelta::~MotionDelta() {}

void MotionDelta::serialize(MapFileO& file, GameObjectArray* arr) {
	obj_.serialize(file, arr);
	file.write_spoint3(dpos_);
}

void MotionDelta::revert(RoomMap* room_map) {
	room_map->shift(obj_.resolve(room_map), -dpos_, false, nullptr);
}

DeltaCode MotionDelta::code() {
	return DeltaCode::MotionDelta;
}

std::unique_ptr<Delta> MotionDelta::deserialize(MapFileIwithObjs& file) {
	auto obj = file.read_frozen_obj();
	auto dpos = file.read_spoint3();
	return std::make_unique<MotionDelta>(obj, dpos);
}


BatchMotionDelta::BatchMotionDelta(std::vector<GameObject*> objs, Point3 dpos) :
	objs_{}, dpos_{ dpos } {
	for (auto* obj : objs) {
		objs_.push_back({ obj });
	}
}

BatchMotionDelta::BatchMotionDelta(std::vector<FrozenObject>&& objs, Point3 dpos) :
	objs_{ std::move(objs) }, dpos_{ dpos } {}

BatchMotionDelta::~BatchMotionDelta() {}

void BatchMotionDelta::serialize(MapFileO& file, GameObjectArray* arr) {
	file.write_uint32((unsigned int)objs_.size());
	for (auto& obj : objs_) {
		obj.serialize(file, arr);
	}
	file.write_spoint3(dpos_);
}

void BatchMotionDelta::revert(RoomMap* room_map) {
	room_map->batch_shift_frozen(std::move(objs_), -dpos_);
}

DeltaCode BatchMotionDelta::code() {
	return DeltaCode::BatchMotionDelta;
}

std::unique_ptr<Delta> BatchMotionDelta::deserialize(MapFileIwithObjs& file) {
	std::vector<FrozenObject> objs;
	auto n_objs = file.read_uint32();
	for (unsigned int i = 0; i < n_objs; ++i) {
		objs.push_back(file.read_frozen_obj());
	}
	auto dpos = file.read_spoint3();
	return std::make_unique<BatchMotionDelta>(std::move(objs), dpos);
}


ClearFlagCollectionDelta::ClearFlagCollectionDelta() {}

ClearFlagCollectionDelta::~ClearFlagCollectionDelta() {}

void ClearFlagCollectionDelta::serialize(MapFileO& file, GameObjectArray* arr) {}

void ClearFlagCollectionDelta::revert(RoomMap* room_map) {
	room_map->uncollect_flag();
}

DeltaCode ClearFlagCollectionDelta::code() {
	return DeltaCode::ClearFlagCollectionDelta;
}

std::unique_ptr<Delta> ClearFlagCollectionDelta::deserialize(MapFileIwithObjs& file) {
	return std::make_unique<ClearFlagCollectionDelta>();
}


ActivePlayerGuard::ActivePlayerGuard(PlayerCycle* cycle) : cycle_{ cycle } {
	if (cycle_->index_ >= 0) {
		cycle_->players_[cycle_->index_]->active_ = false;
	}
}

ActivePlayerGuard::~ActivePlayerGuard() {
	if (cycle_->index_ >= 0) {
		cycle_->players_[cycle_->index_]->active_ = true;
	}
}


PlayerCycle::PlayerCycle() {}

PlayerCycle::~PlayerCycle() {}

void PlayerCycle::add_player(Player* player, DeltaFrame* delta_frame, bool init) {
	ActivePlayerGuard guard{ this };
	if (delta_frame) {
		delta_frame->push(std::make_unique<AddPlayerDelta>(index_));
	}
	if (init) {
		index_ = (int)players_.size();
	}
	players_.push_back(player);
}

void PlayerCycle::set_active_player(Player* player) {
	ActivePlayerGuard guard{ this };
	auto active_it = std::find(players_.begin(), players_.end(), player);
	if (active_it == players_.end()) {
		add_player(player, nullptr, true);
	} else {
		index_ = (int)std::distance(players_.begin(), active_it);
	}
}

void PlayerCycle::add_player_at_pos(Player* player, int index) {
	if (index < dead_index_) {
		++dead_index_;
	}
	players_.insert(players_.begin() + index, player);
}

void PlayerCycle::remove_player(Player* player, DeltaFrame* delta_frame) {
	ActivePlayerGuard guard{ this };
	auto rem_it = std::find(players_.begin(), players_.end(), player);
	if (rem_it == players_.end()) {
		return;
	}
	int rem_index = (int)std::distance(players_.begin(), rem_it);
	if (delta_frame) {
		delta_frame->push(std::make_unique<RemovePlayerDelta>(player, dead_player_, index_, dead_index_, rem_index));
	}
	if (index_ == rem_index) {
		dead_player_ = player;
		dead_index_ = index_;
		index_ = -1;
	}
	if (rem_index < dead_index_) {
		--dead_index_;
	}
	if (rem_index < index_) {
		--index_;
	}
	players_.erase(rem_it);
}

bool PlayerCycle::cycle_player(DeltaFrame* delta_frame) {
	ActivePlayerGuard guard{ this };
	auto delta = std::make_unique<CyclePlayerDelta>(dead_player_, index_, dead_index_);
	if (index_ == -1) {
		if (players_.size() == 0) {
			return false;
		} else {
			dead_player_ = nullptr;
			if (dead_index_ < players_.size()) {
				index_ = dead_index_;
			} else {
				index_ = 0;
			}
			dead_index_ = -1;
		}
	} else {
		if (players_.size() ==  1) {
			return false;
		} else if (index_ == players_.size() - 1) {
			index_ = 0;
		} else {
			++index_;
		}
	}
	delta_frame->push(std::move(delta));
	return true;
}

Player* PlayerCycle::current_player() {
	if (index_ >= 0) {
		return players_[index_];
	} else {
		return nullptr;
	}
}

Player* PlayerCycle::dead_player() {
	return dead_player_;
}

bool PlayerCycle::any_player_alive() {
	return players_.size() > 0;
}

void PlayerCycle::serialize_permutation(MapFileO& file) {
	file << MapCode::PlayerCycle;
	for (auto* player : players_) {
		if (player->tangible_) {
			file << ObjRefCode::Tangible;
			file << player->pos_;
		} else {
			file << ObjRefCode::HeldPlayer;
			file << player->car_riding()->pos();
		}
	}
}

void PlayerCycle::deserialize_permutation(MapFileI& file, RoomMap* room_map) {
	for (int i = 0; i < players_.size(); ++i) {
		auto ref_code = static_cast<ObjRefCode>(file.read_byte());
		Player* cur_player = nullptr;
		auto pos = file.read_point3();
		switch (ref_code) {
		case ObjRefCode::Tangible:
		{
			cur_player = dynamic_cast<Player*>(room_map->view(pos));
			break;
		}
		case ObjRefCode::HeldPlayer:
		{
			cur_player = dynamic_cast<Car*>(room_map->view(pos)->modifier())->player_;
			break;
		}
		}
		players_[i] = cur_player;
	}
}


AddPlayerDelta::AddPlayerDelta(int index) :
	index_{ index } {}

AddPlayerDelta::~AddPlayerDelta() {}

void AddPlayerDelta::serialize(MapFileO& file, GameObjectArray* arr) {
	file << index_;
}

void AddPlayerDelta::revert(RoomMap* room_map) {
	auto* cycle = room_map->player_cycle();
	ActivePlayerGuard guard{ cycle };
	cycle->players_.pop_back();
	cycle->index_ = index_;
}

DeltaCode AddPlayerDelta::code() {
	return DeltaCode::AddPlayerDelta;
}

std::unique_ptr<Delta> AddPlayerDelta::deserialize(MapFileIwithObjs& file) {
	return std::make_unique<AddPlayerDelta>(file.read_byte());
}


RemovePlayerDelta::RemovePlayerDelta(Player* player, Player* dead_player, int index, int dead_index, int rem) :
	player_{ player }, dead_player_{ dead_player }, index_{ index }, dead_index_{ dead_index }, rem_{ rem } {}

RemovePlayerDelta::RemovePlayerDelta(FrozenObject player, FrozenObject dead_player, int index, int dead_index, int rem) :
	player_{ player }, dead_player_{ dead_player }, index_{ index }, dead_index_{ dead_index }, rem_{ rem } {}

RemovePlayerDelta::~RemovePlayerDelta() {}

void RemovePlayerDelta::serialize(MapFileO& file, GameObjectArray* arr) {
	player_.serialize(file, arr);
	dead_player_.serialize(file, arr);
	file << index_ << dead_index_ << rem_;
}

void RemovePlayerDelta::revert(RoomMap* room_map) {
	auto* cycle = room_map->player_cycle();
	ActivePlayerGuard guard{ cycle };
	cycle->dead_player_ = static_cast<Player*>(dead_player_.resolve(room_map));
	cycle->index_ = index_;
	cycle->add_player_at_pos(static_cast<Player*>(player_.resolve(room_map)), rem_);
}

DeltaCode RemovePlayerDelta::code() {
	return DeltaCode::RemovePlayerDelta;
}

std::unique_ptr<Delta> RemovePlayerDelta::deserialize(MapFileIwithObjs& file) {
	auto player = file.read_frozen_obj();
	auto dead_player = file.read_frozen_obj();
	unsigned char b[3];
	file.read(b, 3);
	return std::make_unique<RemovePlayerDelta>(player, dead_player, b[0], b[1], b[2]);
}


CyclePlayerDelta::CyclePlayerDelta(Player* dead_player, int index, int dead_index) :
	dead_player_{ dead_player }, index_{ index }, dead_index_{ dead_index } {}

CyclePlayerDelta::CyclePlayerDelta(FrozenObject dead_player, int index, int dead_index) :
	dead_player_{ dead_player }, index_{ index }, dead_index_{ dead_index } {}

CyclePlayerDelta::~CyclePlayerDelta() {}

void CyclePlayerDelta::serialize(MapFileO& file, GameObjectArray* arr) {
	dead_player_.serialize(file, arr);
	file << index_ << dead_index_;
}

void CyclePlayerDelta::revert(RoomMap* room_map) {
	auto* cycle = room_map->player_cycle();
	ActivePlayerGuard guard{ cycle };
	cycle->dead_player_ = static_cast<Player*>(dead_player_.resolve(room_map));
	cycle->index_ = index_;
	cycle->dead_index_ = dead_index_;
}

DeltaCode CyclePlayerDelta::code() {
	return DeltaCode::CyclePlayerDelta;
}

std::unique_ptr<Delta> CyclePlayerDelta::deserialize(MapFileIwithObjs& file) {
	auto dead_player = file.read_frozen_obj();
	unsigned char b[2];
	file.read(b, 2);
	return std::make_unique<CyclePlayerDelta>(dead_player, b[0], b[1]);
}


WallDestructionDelta::WallDestructionDelta(Point3 pos) : Delta(), pos_{ pos } {}

WallDestructionDelta::~WallDestructionDelta() {}

void WallDestructionDelta::serialize(MapFileO& file, GameObjectArray* arr) {
	file << pos_;
}

void WallDestructionDelta::revert(RoomMap* room_map) {
	room_map->at(pos_) = GENERIC_WALL_ID;
}

DeltaCode WallDestructionDelta::code() {
	return DeltaCode::WallDestructionDelta;
}

std::unique_ptr<Delta> WallDestructionDelta::deserialize(MapFileIwithObjs& file) {
	return std::make_unique<WallDestructionDelta>(file.read_point3());
}


DeadObjectAdder::DeadObjectAdder(GameObjectArray & obj_array) :
	RoomMap(obj_array, nullptr, 0, 0, 0) {}

DeadObjectAdder::~DeadObjectAdder() {}

void DeadObjectAdder::push_to_object_array(std::unique_ptr<GameObject> obj, DeltaFrame*) {
	auto* obj_raw = obj.get();
	obj_array_.push_object(std::move(obj));
	obj_array_.add_dead_obj(obj_raw);
}

void DeadObjectAdder::create_in_map(std::unique_ptr<GameObject> obj_unique, bool, DeltaFrame*) {
	push_to_object_array(std::move(obj_unique), nullptr);
}
