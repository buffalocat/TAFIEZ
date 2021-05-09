#ifndef MAPDISPLAYSTATE_H
#define MAPDISPLAYSTATE_H

#include "objectmodifier.h"

enum class HubAccessFlag;
enum class HubCode;

class PlayingGlobalData;
enum class ParticleTexture;

class Font;
struct TextVertex3;

struct MapSprite {
	glm::vec3 pos;
	ParticleTexture tex;
	int color;
};


class MapDisplay : public ObjectModifier {
public:
	MapDisplay(GameObject* parent);
	~MapDisplay();

	void make_str(std::string&);
	ModCode mod_code();
	static void deserialize(MapFileI&, GameObjectArray*, GameObject*);

	std::unique_ptr<ObjectModifier> duplicate(GameObject*, RoomMap*, DeltaFrame*);

	void signal_animation(AnimationManager*, DeltaFrame*);
	void init_sprites(PlayingState*);
	void draw_special(GraphicsManager*, GLuint atlas);

	void generate_map();
	bool draw_zone(char zone, float dx, float dy);
	bool draw_hub(HubCode hub, float dx, float dy);
	bool draw_warp(HubCode hub, char zone, float dx, float dy);
	void draw_tex(ParticleTexture tex, float dx, float dy, float dz, int color);
	void draw_char(char c, float dx, float dy);

	bool visited(char zone);

private:
	PlayingGlobalData* global_{};
	std::vector<MapSprite> sprites_{};
	std::vector<TextVertex3> char_verts_{};
	glm::vec3 parent_pos_;
	glm::vec3 pos_;
	Font* font_;
	GLuint VAO_, VBO_;

	PlayingState* state_{};
};

#endif //MAPDISPLAYSTATE_H