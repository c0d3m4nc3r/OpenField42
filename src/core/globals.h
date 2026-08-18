#pragma once

class Game;
class Window;
class Renderer;
class Input;
class Console;
class VFS;
class DebugUI;
class GeometryManager;
class ShaderManager;
class TextureManager;
struct World;

extern Game* g_Game;
extern Window* g_Window;
extern Renderer* g_Renderer;
extern Input* g_Input;
extern Console* g_Console;
extern VFS* g_VFS;
extern DebugUI* g_DebugUI;
extern GeometryManager* g_GeometryMgr;
extern ShaderManager* g_ShaderMgr;
extern TextureManager* g_TextureMgr;
extern World* g_World;