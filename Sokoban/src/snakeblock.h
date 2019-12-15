#ifndef SNAKEBLOCK_H
#define SNAKEBLOCK_H

#include "gameobject.h"
#include "delta.h"

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

	std::unique_ptr<GameObject> duplicate(RoomMap*, DeltaFrame*);

	bool is_snake();
	Sticky sticky();

    void collect_sticky_links(RoomMap*, Sticky sticky_level, std::vector<GameObject*>& links);

    void collect_dragged_snake_links(RoomMap*, Point3 dir, std::vector<GameObject*>& strong, std::vector<GameObject*>& weak);

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
	void collect_all_viable_neighbors(RoomMap*, std::set<SnakeBlock*>& check);
    void collect_maybe_confused_neighbors(RoomMap*, std::set<SnakeBlock*>& check);
    void remove_wrong_color_links(DeltaFrame*);
	void break_tangible_links(DeltaFrame*, std::vector<GameObject*>& fall_check);
    void check_add_local_links(RoomMap*, DeltaFrame*);
	void break_blocked_links_horizontal(std::vector<GameObject*>& fall_check, RoomMap* map, DeltaFrame* delta_frame, Point3 dir);
	void break_blocked_links(std::vector<GameObject*>& fall_check, DeltaFrame* delta_frame);

    void reset_internal_state();

	void setup_on_put(RoomMap*, DeltaFrame*, bool real);
    void cleanup_on_take(RoomMap*, DeltaFrame*, bool real);

    std::unique_ptr<SnakeBlock> make_split_copy(RoomMap*, DeltaFrame*);

	std::vector<SnakeBlock*> links_{};

    int ends_;
	bool weak_;

	SnakeBlock* target_{};
    unsigned int distance_ = 0;
};


class SnakePuller {
public:
    SnakePuller(MoveProcessor*, RoomMap*, DeltaFrame*,
                std::vector<GameObject*>& moving_blocks,
                std::set<SnakeBlock*>& link_add_check,
                std::vector<GameObject*>& fall_check);
    ~SnakePuller();
    void prepare_pull(SnakeBlock*);
    void perform_pulls();

private:
	MoveProcessor* move_processor_;
    RoomMap* map_;
    DeltaFrame* delta_frame_;
	std::vector<SnakeBlock*> snakes_to_pull_{};
    std::vector<GameObject*>& moving_blocks_;
    std::set<SnakeBlock*>& link_add_check_;
    std::vector<GameObject*>& fall_check_;
};


class AddLinkDelta : public Delta {
public:
	AddLinkDelta(SnakeBlock* a, SnakeBlock* b);
	~AddLinkDelta();
	void revert();

private:
	SnakeBlock* a_;
	SnakeBlock* b_;
};


class RemoveLinkDelta : public Delta {
public:
	RemoveLinkDelta(SnakeBlock* a, SnakeBlock* b);
	~RemoveLinkDelta();
	void revert();

private:
	SnakeBlock* a_;
	SnakeBlock* b_;
};

class RemoveLinkOneWayDelta : public Delta {
public:
	RemoveLinkOneWayDelta(SnakeBlock* a, SnakeBlock* b);
	~RemoveLinkOneWayDelta();
	void revert();

private:
	SnakeBlock* a_;
	SnakeBlock* b_;
};

#endif // SNAKEBLOCK_H
