// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files
#include <windows.h>

// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

#undef min
#undef max

// reference additional headers your program requires here

// std
#include <algorithm>
#include <array>
#include <deque>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <set>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

// third party
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <dear/imgui.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H

// mine
#include "point.h"
#include "shader.h"

#ifdef SOKOBAN_LOG
#define LOG(x) std::cout << x << std::endl
#else
#define LOG(x)
#endif
