#pragma once
#include <string>
#include <vector>

/**
 * Basically acts as a façade between glfw's mouse and Lua
 */
namespace LuaMouse {
  struct Mouse {
    std::string name;
    int button;
  };
  /**
   * Return not const as references to these values are used by lua.
   * Despite being "readonly" from lua's side, they can't be const.
   * (Even readonly variables can be changed in lua, with things like rawset.)
   */
  auto get_buttons() -> std::vector<Mouse> &;
}
