#include "stdafx.h"
#include "roommap.h"

#include "gameobjectarray.h"
#include "gameobject.h"
#include "wall.h"
#include "delta.h"
#include "snakeblock.h"
#include "switch.h"
#include "signaler.h"
#include "mapfile.h"
#include "objectmodifier.h"
#include "maplayer.h"
#include "effects.h"
#include "moveprocessor.h"
#include "common_constants.h"

RoomMap::RoomMap(GameObjectArray& obj_array, int width, int height, int depth) :
	obj_array_{ obj_array },
	width_{ width }, height_{ height }, depth_{ depth } {
	for (int z = 0; z < depth; ++z) {
		layers_.push_back(MapLayer(this, width_, height_, z));
	}
}

/*
void RoomMap::print_listeners() {
	std::cout << "\nPrinting listeners post-move!" << std::endl;
	for (auto& p : listeners_) {
		for (ObjectModifier* mod : p.second) {
			std::cout << mod->name() << " at " << p.first << std::endl;
		}
	}
}
*/

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
	if (id == GLOBAL_WALL_ID) {
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
	file << MapCode::Zone;
	file << zone_;
	file << MapCode::ClearFlagRequirement;
	file << clear_flag_req_;
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
		return obj_array_[GLOBAL_WALL_ID];
	}
}

void RoomMap::just_take(GameObject* obj) {
	obj->cleanup_on_take(this);
	obj->tangible_ = false;
	at(obj->pos_) -= obj->id_;
}

void RoomMap::just_put(GameObject* obj) { 
	at(obj->pos_) += obj->id_;
	obj->setup_on_put(this);
	obj->tangible_ = true;
}

void RoomMap::take(GameObject* obj) {
	activate_listeners_at(obj->pos_);
	just_take(obj);
}

void RoomMap::put(GameObject* obj) {
	just_put(obj);
	activate_listeners_at(obj->pos_);
}

void RoomMap::take_real(GameObject* obj, DeltaFrame* delta_frame) {
	if (delta_frame) {
		delta_frame->push(std::make_unique<TakeDelta>(obj, this));
	}
	take(obj);
}

void RoomMap::put_real(GameObject* obj, DeltaFrame* delta_frame) {
	if (delta_frame) {
		delta_frame->push(std::make_unique<PutDelta>(obj, this));
	}
	put(obj);
}

void RoomMap::shift(GameObject* obj, Point3 dpos, DeltaFrame* delta_frame) {
	take(obj);
	obj->pos_ += dpos;
	put(obj);
	obj->set_linear_animation(dpos);
	delta_frame->push(std::make_unique<MotionDelta>(obj, dpos, this));
}

void RoomMap::batch_shift(std::vector<GameObject*> objs, Point3 dpos, DeltaFrame* delta_frame) {
	for (auto obj : objs) {
		obj->set_linear_animation(dpos);
		take(obj);
		obj->pos_ += dpos;
		put(obj);
	}
	delta_frame->push(std::make_unique<BatchMotionDelta>(std::move(objs), dpos, this));
}

// "just" means "don't do any checks, animations, deltas"
void RoomMap::just_shift(GameObject* obj, Point3 dpos) {
	just_take(obj);
	obj->pos_ += dpos;
	just_put(obj);
}

void RoomMap::just_batch_shift(std::vector<GameObject*> objs, Point3 dpos) {
	for (auto obj : objs) {
		just_take(obj);
		obj->pos_ += dpos;
		just_put(obj);
	}
}

void RoomMap::create(std::unique_ptr<GameObject> obj_unique, DeltaFrame* delta_frame) {
	GameObject* obj = obj_unique.get();
	// Need to push it into the GameObjectArray first to give it an ID
	obj_array_.push_object(std::move(obj_unique));
	put(obj);
	if (delta_frame) {
		delta_frame->push(std::make_unique<CreationDelta>(obj, this));
	}
}

void RoomMap::create_abstract(std::unique_ptr<GameObject> obj_unique, DeltaFrame* delta_frame) {
	if (delta_frame) {
		delta_frame->push(std::make_unique<AbstractCreationDelta>(obj_unique.get(), this));
	}
	obj_array_.push_object(std::move(obj_unique));
}

void RoomMap::create_wall(Point3 pos) {
	at(pos) = GLOBAL_WALL_ID;
}

void RoomMap::clear(Point3 pos) {
	at(pos) = 0;
}

void RoomMap::uncreate(GameObject* obj) {
	just_take(obj);
	obj->cleanup_on_destruction(this);
	obj_array_.destroy(obj);
}

void RoomMap::uncreate_abstract(GameObject* obj) {
	obj->cleanup_on_destruction(this);
	obj_array_.destroy(obj);
}

void RoomMap::destroy(GameObject* obj, DeltaFrame* delta_frame) {
	obj->cleanup_on_destruction(this);
	take(obj);
	if (delta_frame) {
		delta_frame->push(std::make_unique<DeletionDelta>(obj, this));
	}
}

void RoomMap::undestroy(GameObject* obj) {
	just_put(obj);
	obj->setup_on_undestruction(this);
}

void RoomMap::remove_auto(AutoBlock* obj) {
	autos_.erase(std::remove(autos_.begin(), autos_.end(), obj), autos_.end());
}

void RoomMap::remove_puppet(PuppetBlock* obj) {
	puppets_.erase(std::remove(puppets_.begin(), puppets_.end(), obj), puppets_.end());
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

struct ObjectDrawer {
	void operator()(int, Point3);

	GameObjectArray& obj_array;
	GraphicsManager* gfx;
};

void ObjectDrawer::operator()(int id, Point3 pos) {
	if (id > GLOBAL_WALL_ID) {
		obj_array[id]->draw(gfx);
	} else {
		Wall::draw(gfx, pos);
	}
}

void RoomMap::draw(GraphicsManager* gfx, double angle) {
	GameObjIDPosFunc drawer = ObjectDrawer{ obj_array_, gfx };
	// TODO: use a smarter map rectangle!
	for (auto it = layers_.rbegin(); it != layers_.rend(); ++it) {
		it->apply_to_rect_with_pos(MapRect{ 0,0,width_,height_ }, drawer);
	}
	// TODO: draw walls!
	effects_->sort_by_distance(angle);
	effects_->update();
	effects_->draw(gfx);
}

void RoomMap::draw_layer(GraphicsManager* gfx, int z) {
	GameObjIDPosFunc drawer = ObjectDrawer{ obj_array_, gfx };
	layers_[z].apply_to_rect_with_pos(MapRect{ 0,0,width_,height_ }, drawer);
}

struct ObjectShifter {
	void operator()(int);

	GameObjectArray& obj_array;
	RoomMap* room_map;
	Point3 dpos;
};

void ObjectShifter::operator()(int id) {
	if (id > GLOBAL_WALL_ID) {
		obj_array[id]->shift_internal_pos(dpos);
	}
}

void RoomMap::shift_all_objects(Point3 d) {
	GameObjIDFunc shifter = ObjectShifter{ obj_array_, this, d };
	for (auto& layer : layers_) {
		layer.apply_to_rect(MapRect{ 0,0,width_,height_ }, shifter);
	}
}

struct ObjectDestroyer {
	void operator()(int);

	GameObjectArray& obj_array;
	RoomMap* room_map;
};

void ObjectDestroyer::operator()(int id) {
	if (id > GLOBAL_WALL_ID) {
		room_map->destroy(obj_array[id], nullptr);
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
	height_ += d.y;
	if (d.x < 0) {
		for (auto& layer : layers_) {
			layer.apply_to_rect(MapRect{ width_ + d.x, 0, -d.x, height_ }, destroyer);
		}
	}
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
	RoomMap* room_map;
	DeltaFrame* delta_frame;
};

void RoomStateInitializer::operator()(int id) {
	GameObject* obj = obj_array[id];
	if (obj->gravitable_) {
		mp->add_to_fall_check(obj);
	}
	if (SnakeBlock* sb = dynamic_cast<SnakeBlock*>(obj)) {
		sb->check_add_local_links(room_map, delta_frame);
	}
	if (ObjectModifier* mod = obj->modifier()) {
		room_map->activate_listener_of(mod);
	}
}

void RoomMap::set_initial_state(bool editor_mode) {
	DeltaFrame dummy_df{};
	MoveProcessor mp = MoveProcessor(nullptr, this, &dummy_df, nullptr, false);
	GameObjIDFunc state_initializer = RoomStateInitializer{ obj_array_, &mp, this, &dummy_df };
	for (auto& layer : layers_) {
		layer.apply_to_rect(MapRect{ 0,0,width_,height_ }, state_initializer);
	}
	// In editor mode, don't check switches or gravity.
	if (editor_mode) {
		return;
	}
	for (auto& sig : signalers_) {
		if (auto* p_sig = dynamic_cast<ParitySignaler*>(sig.get())) {
			p_sig->check_send_initial(this, &dummy_df, &mp);
		}
	}
	mp.perform_switch_checks(true);
	mp.try_fall_step();
}

struct SnakeInitializer {
	void operator()(int id);

	GameObjectArray& obj_array;
	RoomMap* room_map;
	DeltaFrame* delta_frame;
};

void SnakeInitializer::operator()(int id) {
	if (SnakeBlock* sb = dynamic_cast<SnakeBlock*>(obj_array[id])) {
		sb->check_add_local_links(room_map, delta_frame);
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

// The room keeps track of some things which must be forgotten after a move or undo
void RoomMap::reset_local_state() {
	activated_listeners_ = {};
}

void RoomMap::push_signaler(std::unique_ptr<Signaler> signaler) {
	signalers_.push_back(std::move(signaler));
}

// TODO: make this *not* violate locality (i.e., keep a set of "maybe activated" signalers)
void RoomMap::check_signalers(DeltaFrame* delta_frame, MoveProcessor* mp) {
	for (auto& signaler : signalers_) {
		signaler->check_send_signal(this, delta_frame, mp);
	}
}

void RoomMap::remove_signaler(Signaler* rem) {
	signalers_.erase(std::remove_if(signalers_.begin(), signalers_.end(),
		[rem](std::unique_ptr<Signaler>& sig) {return sig.get() == rem; }), signalers_.end());
}


void RoomMap::make_fall_trail(GameObject* block, int height, int drop) {
	effects_->push_trail(block, height, drop);
}
