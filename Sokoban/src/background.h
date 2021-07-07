#pragma once

enum class WallColorType {
	Default = 0,
	Ascend,
	Descend,
	Wave,
	COUNT,
};

class WallColorSpec {
public:
	WallColorType type;
	int count;
	int offset;
	double min;
	double max;
};

enum class BackgroundSpecType {
	Default = 0,
	Linear = 1,
	HubAlpha = 2,
	HubBeta = 3,
	HubGamma = 4,
	HubDelta = 5,
	Nexus = 6,
	XNexus = 7,
	Omega = 8,
	One = 9,
	Mountain = 10,
	HFlag = 11,
	COUNT,
};

enum class BackgroundParticleType {
	None = 0,
	COUNT,
};

class BackgroundSpec {
public:
	BackgroundSpecType type;
	glm::vec4 color_1;
	glm::vec4 color_2;
	BackgroundParticleType particle_type;
};

class BackgroundAnimation {
public:
	BackgroundAnimation();
	~BackgroundAnimation();

	void snap_colors();
	void compute_cur_color();
	void set_positions(glm::vec3 camera_sight, glm::vec3 camera_up, FPoint3 player_pos);
	void update();
	void draw();

	FPoint3 cur_pos_{};
	std::vector<glm::vec3> vert_data_{};
	GLuint vao_{};
	GLuint vbo_{};
	glm::vec4 color_down_{};
	glm::vec4 color_up_{};
	glm::vec4 target_color_down_{};
	glm::vec4 target_color_up_{};
	glm::vec4 cur_color_down_{};
	glm::vec4 cur_color_up_{};
	BackgroundSpecType type_{};
	BackgroundParticleType particle_type_{};
	Shader shader_{ Shader("shaders/background_shader.vs", "shaders/background_shader.fs") };
};


std::vector<glm::vec3> generate_sphere_verts(int n_theta, int n_phi);


