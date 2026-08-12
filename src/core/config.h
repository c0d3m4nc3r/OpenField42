#pragma once

/* Window */

#define WINDOW_TITLE "OpenField42"
#define WINDOW_WIDTH 1600
#define WINDOW_HEIGHT 900
#define WINDOW_FULLSCREEN 0
#define WINDOW_RESIZABLE 0
#define WINDOW_BORDERLESS 0
#define WINDOW_VSYNC_ON 1

/* Rendering */

#define USE_LODS 1
#define USE_FRUSTUM_CULLING 1
#define RENDER_FOG 1
#define VIEW_DISTANCE 0 // Set to 0 to use view distance from Init.con

/* Texture Defaults */

#define DEFAULT_MIN_FILTER GL_LINEAR_MIPMAP_LINEAR
#define DEFAULT_MAG_FILTER GL_LINEAR
#define DEFAULT_WRAP_S GL_REPEAT
#define DEFAULT_WRAP_T GL_REPEAT

/* Controls */

#define MOVE_FORWARD_KEY Key::W
#define MOVE_BACKWARD_KEY Key::S
#define MOVE_LEFT_KEY Key::A
#define MOVE_RIGHT_KEY Key::D
#define MOVE_UP_KEY Key::Space
#define MOVE_DOWN_KEY Key::LShift

#define CAMERA_MOVE_SPEED 50.0f

/* Directories */

#define GAME_DATA_DIR "game_data"
#define ASSETS_DIR "assets"
