#ifndef GAMEOBJECTARRAY_H
#define GAMEOBJECTARRAY_H

class GameObject;
class MapFileI;
class MapFileO;
class RoomMap;
class FrozenObject;

class GameObjectArray {
public:
	GameObjectArray();
	~GameObjectArray();
	void push_object(std::unique_ptr<GameObject> obj);
	GameObject* operator[](unsigned int id) const;
	GameObject* safe_get(unsigned int id) const;
	void uncreate_object(GameObject* obj);

	bool add_inacc_obj(GameObject* obj, RoomMap* room_map);
	void remove_inacc_obj(GameObject* obj);
	void update_parent_map(GameObject* obj, RoomMap* room_map);
	void serialize_object_ref(GameObject* obj, MapFileO& file);
	void serialize_inacc_objs(MapFileO& file);
	void check_room_inaccessibles(RoomMap* room);

	std::vector<std::unique_ptr<GameObject>> array_{};
	std::vector<unsigned int> free_ids_{};

	std::vector<GameObject*> inacc_obj_list_{};
	std::map<GameObject*, unsigned int> inacc_obj_map_{};
	std::vector<unsigned int> free_inacc_ids_{};

	std::map<GameObject*, RoomMap*> obj_room_map_{};
	std::map<std::string, std::vector<std::unique_ptr<FrozenObject>>> frozen_inaccessible_map_{};
};

#endif // GAMEOBJECTARRAY_H
