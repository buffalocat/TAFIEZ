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
	void uncreate_object(GameObject* obj);

	void add_dead_obj(GameObject* obj);
	void remove_dead_obj(GameObject* obj);

	void serialize(MapFileO& file);
	void deserialize(MapFileI& file);

	std::vector<std::unique_ptr<GameObject>> array_{};
	std::vector<unsigned int> free_ids_{};

	std::vector<GameObject*> dead_obj_list_{};
};

#endif // GAMEOBJECTARRAY_H
