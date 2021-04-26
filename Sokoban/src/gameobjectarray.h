#ifndef GAMEOBJECTARRAY_H
#define GAMEOBJECTARRAY_H


class GameObject;
class MapFileI;
class MapFileO;

class GameObjectArray {
public:
	GameObjectArray();
	~GameObjectArray();
	void push_object(std::unique_ptr<GameObject> obj);
	GameObject* operator[](unsigned int id) const;
	GameObject* safe_get(unsigned int id) const;
	void schedule_uncreation(GameObject* obj);
	void remove_uncreated_objects();

	void schedule_undeletion(GameObject* obj);
	void remove_undeleted_objects();

	unsigned int add_dead_obj(GameObject* obj);
	void serialize_object_ref(GameObject* obj, MapFileO& file);
	void serialize_dead_objs(MapFileO& file);
	void deserialize_dead_objs(MapFileI& file);

	std::vector<std::unique_ptr<GameObject>> array_{};
	std::vector<GameObject*> to_uncreate_{};
	std::vector<unsigned int> free_ids_{};

	std::vector<GameObject*> dead_obj_list_{};
	std::map<GameObject*, unsigned int> dead_obj_map_{};
	std::vector<GameObject*> to_undelete_{};
	std::vector<unsigned int> free_dead_ids_{};
};

#endif // GAMEOBJECTARRAY_H
