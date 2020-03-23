#ifndef GAMEOBJECTARRAY_H
#define GAMEOBJECTARRAY_H


class GameObject;

class GameObjectArray
{
public:
    GameObjectArray();
    ~GameObjectArray();
    void push_object(std::unique_ptr<GameObject> obj);
    GameObject* operator[](unsigned int id) const;
    GameObject* safe_get(unsigned int id) const;
    void schedule_deletion(GameObject* obj);
	void remove_deleted_objects();

	std::vector<std::unique_ptr<GameObject>> array_{};
	std::vector<GameObject*> to_delete_{};
};

#endif // GAMEOBJECTARRAY_H
