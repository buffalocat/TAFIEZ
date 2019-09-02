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
    void uncreate(GameObject* obj);

private:
    std::vector<std::unique_ptr<GameObject>> array_;
};

#endif // GAMEOBJECTARRAY_H
