#ifndef ROOM_H
#define ROOM_H

#include "point.h"

class DeltaFrame;
class GameObjectArray;
class GraphicsManager;
class RoomMap;
class Camera;
class MapFileI;
class MapFileO;
class GameObject;
class Player;
class PlayingGlobalData;
class RoomLabelDrawer;

class Room {
public:
    Room(GraphicsManager* gfx, std::string name);
    ~Room();
    std::string const name();
    void initialize(GameObjectArray& objs, PlayingGlobalData* global, int w, int h, int d);
    void set_cam_pos(Point3 vpos, FPoint3 rpos, bool display_labels, bool snap);
    bool valid(Point3);
    RoomMap* map();
	Camera* camera();

    void write_to_file(MapFileO& file);
    void load_from_file(GameObjectArray& objs, MapFileI& file, PlayingGlobalData* global, Player** player_ptr);

    void draw_at_pos(Point3 cam_pos, bool display_labels, bool ortho, bool one_layer);
    void draw_at_player(Player* target, bool display_labels, bool ortho, bool one_layer);
	void draw(Point3 vpos, FPoint3 rpos, bool display_labels, bool ortho, bool one_layer);
    void update_view(Point3 vpos, FPoint3 rpos, bool display_labels, bool ortho);

    void extend_by(Point3 d);
    void shift_by(Point3 d);

	// This is used exclusively for making sure doors between rooms stay accurate
	Point3_S16 offset_pos_{ 0,0,0 };

	std::unique_ptr<RoomLabelDrawer> zone_label_{};
	std::unique_ptr<RoomLabelDrawer> context_label_{};

private:
	std::unique_ptr<RoomMap> map_{};
	std::unique_ptr<Camera> camera_{};
	std::string name_;

	GraphicsManager* gfx_;

    void read_objects(MapFileI& file, Player** player_ptr);
    void read_camera_rects(MapFileI& file);
    void read_snake_link(MapFileI& file);
    void read_door_dest(MapFileI& file);
    void read_threshold_signaler(MapFileI& file);
	void read_parity_signaler(MapFileI& file);
	void read_wall_positions(MapFileI& file);
    void read_wall_runs(MapFileI& file);
};

#endif // ROOM_H
