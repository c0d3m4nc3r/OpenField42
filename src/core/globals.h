#pragma once

#include "core/thread_pool.h"

class Game;
class Window;
class Renderer;
class Input;
class Console;
class VFS;
class DebugUI;
class TemplateManager;
class GeometryManager;
class ShaderManager;
class TextureManager;
class ScriptManager;
struct World;

extern Game* g_Game;
extern Window* g_Window;
extern Renderer* g_Renderer;
extern Input* g_Input;
extern Console* g_Console;
extern VFS* g_VFS;
extern DebugUI* g_DebugUI;
extern TemplateManager* g_TemplateMgr;
extern GeometryManager* g_GeometryMgr;
extern ShaderManager* g_ShaderMgr;
extern TextureManager* g_TextureMgr;
extern ScriptManager* g_ScriptMgr;
extern World* g_World;

extern ThreadPool g_ThreadPool;