#ifndef MAPDISPLAYSTATE_H
#define MAPDISPLAYSTATE_H

#include "objectmodifier.h"

enum class HubAccessFlag;
enum class HubCode;

class PlayingGlobalData;
enum class ParticleTexture;

struct MapSprite {
	glm::vec3 pos;
	ParticleTexture tex;
};

class MapDisplay : public ObjectModifier {
public:
	MapDisplay(GameObject* parent);
	~MapDisplay();

	void make_str(std::string&);
	ModCode mod_code();
	static void deserialize(MapFileI&, RoomMap*, GameObject*);

	std::unique_ptr<ObjectModifier> duplicate(GameObject*, RoomMap*, DeltaFrame*);

	void signal_animation(AnimationManager*, DeltaFrame*);
	void init_sprites(PlayingState*);
	void draw_special(GraphicsManager*, GLuint atlas);

	void generate_map();
	bool draw_zone(char zone, float dx, float dy);
	bool draw_hub(HubCode hub, float dx, float dy);
	bool draw_warp(HubCode hub, char zone, float dx, float dy);
	void draw_tex(ParticleTexture tex, float dx, float dy);

	bool visited(char zone);

private:
	PlayingGlobalData* global_{};
	std::vector<MapSprite> sprites_{};
	FPoint3 pos_;
};

#endif //MAPDISPLAYSTATE_H