#include <cstdint>
#include <string>

extern "C" {
#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>
}

#include <LuaBridge/LuaBridge.h>

using luabridge::LuaRef;
struct lua_State;

class ScriptComponent {
  private:
    // Engine
    LuaRef onUpdate;
    // Keyboard
    LuaRef onKeyPress;
    LuaRef onKeyRelease;
    LuaRef onTextEnter;
    // Mouse
    LuaRef onMouseMove;
    LuaRef onMouseScroll;
    LuaRef onMousePress;
    LuaRef onMouseRelease;

    std::string scriptFilename;

  public:
    static auto SetupLuaState(lua_State *lua) -> void;

    ScriptComponent(lua_State *lua, const std::string &filename);

    // Assuming DT is float for now, will change if needed.
    auto Update(float dt) -> void;
    // Key is sf::Keyboard::Key cast to int
    auto KeyPress(int key, bool alt, bool ctrl, bool shift) -> void;
    auto KeyRelease(int key, bool alt, bool ctrl, bool shift) -> void;
    auto TextEnter(uint32_t text) -> void;
    auto MouseMove(int mousex, int mousey) -> void;
    // No point in supporting multiple mouse wheels (although SFML does)
    auto MouseScroll(float delta, int mousex, int mousey) -> void;
    // Button is sf::Mouse::Button cast to int.
    auto MousePress(int button, int mousex, int mousey) -> void;
    auto MouseRelease(int button, int mousex, int mousey) -> void;
};
