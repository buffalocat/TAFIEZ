#ifndef SNAKEBLOCK_H
#define SNAKEBLOCK_H

#include "gameobject.h"

class SnakeBlock: public ColoredBlock {
public:
	SnakeBlock(Point3 pos, int color, bool pushable, bool gravitable, int ends, bool weak);
    virtual ~SnakeBlock();

    virtual std::string name();
    virtual ObjCode obj_code();
    void serialize(MapFileO& file);
    static std::unique_ptr<GameObject> deserialize(MapFileI& file);
    bool relation_check();
    void relation_serialize(MapFileO& file);

	bool is_snake();
	Sticky sticky();

    void collect_sticky_links(RoomMap*, Sticky sticky_level, std::vector<GameObject*>& links);

    void conditional_drag(std::vector<GameObject*>& links, bool strong);
    void collect_dragged_snake_links(RoomMap*, Point3 dir, std::vector<GameObject*>& links, bool strong);

    bool moving_push_comp();

    bool in_links(SnakeBlock* sb);
    void add_link(SnakeBlock*, DeltaFrame*);
    void add_link_quiet(SnakeBlock*);
    void add_link_one_way(SnakeBlock*);
    void remove_link(SnakeBlock*, DeltaFrame*);
    void remove_link_quiet(SnakeBlock*);
    void remove_link_one_way(SnakeBlock*);
	// This method only exists because sometimes we want to break links
	// *after* removing something from the map
	void remove_link_one_way_undoable(SnakeBlock* sb, DeltaFrame* delta_frame);

    bool can_link(SnakeBlock*);

    void draw(GraphicsManager*);

    bool available();
    bool confused(RoomMap*);
    void collect_maybe_confused_neighbors(RoomMap*, std::set<SnakeBlock*>& check);
    void remove_wrong_color_links(DeltaFrame*);
	void break_tangible_links(DeltaFrame*, std::vector<GameObject*>& fall_check);
    void check_add_local_links(RoomMap*, DeltaFrame*);
	void break_blocked_links_horizontal(std::vector<GameObject*>& fall_check, RoomMap* map, DeltaFrame* delta_frame, Point3 dir);
	void break_blocked_links(std::vector<GameObject*>& fall_check, DeltaFrame* delta_frame);

    void reset_internal_state();

	void setup_on_put(RoomMap*, bool real);
    void cleanup_on_take(RoomMap*, bool real);

    std::unique_ptr<SnakeBlock> make_split_copy(RoomMap*, DeltaFrame*);

	std::vector<SnakeBlock*> links_{};

    int ends_;
	bool weak_;

	SnakeBlock* target_{};
    unsigned int distance_ = 0;
    bool dragged_ = false;
};


class SnakePuller {
public:
    SnakePuller(RoomMap*, DeltaFrame*,
                std::vector<GameObject*>& moving_blocks,
                std::set<SnakeBlock*>& link_add_check,
                std::vector<GameObject*>& fall_check);
    ~SnakePuller();
    void prepare_pull(SnakeBlock*);
    void perform_pulls();

private:
    RoomMap* map_;
    DeltaFrame* delta_frame_;
	std::vector<SnakeBlock*> snakes_to_pull_{};
    std::vector<GameObject*>& moving_blocks_;
    std::set<SnakeBlock*>& link_add_check_;
    std::vector<GameObject*>& fall_check_;
};

#endif // SNAKEBLOCK_H
