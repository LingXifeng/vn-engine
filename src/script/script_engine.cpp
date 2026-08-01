#include "script_engine.h"
#include <iostream>

ScriptEngine::ScriptEngine() {
    m_L = luaL_newstate();
}

ScriptEngine::~ScriptEngine() {
    if (m_L) lua_close(m_L);
}

bool ScriptEngine::init() {
    if (!m_L) return false;
    luaL_openlibs(m_L);
    return true;
}

bool ScriptEngine::loadFile(const std::string& path) {
    if (luaL_loadfile(m_L, path.c_str()) != LUA_OK) {
        m_lastError = lua_tostring(m_L, -1);
        std::cerr << "Lua load error: " << m_lastError << std::endl;
        lua_pop(m_L, 1);
        return false;
    }
    // Execute the loaded chunk to define functions and variables
    if (lua_pcall(m_L, 0, 0, 0) != LUA_OK) {
        m_lastError = lua_tostring(m_L, -1);
        std::cerr << "Lua exec error: " << m_lastError << std::endl;
        lua_pop(m_L, 1);
        return false;
    }
    return true;
}

bool ScriptEngine::loadString(const std::string& code) {
    if (luaL_loadstring(m_L, code.c_str()) != LUA_OK) {
        m_lastError = lua_tostring(m_L, -1);
        std::cerr << "Lua load error: " << m_lastError << std::endl;
        lua_pop(m_L, 1);
        return false;
    }
    return true;
}

bool ScriptEngine::execute() {
    int status = lua_pcall(m_L, 0, 0, 0);
    if (status != LUA_OK) {
        m_lastError = lua_tostring(m_L, -1);
        std::cerr << "Lua execute error: " << m_lastError << std::endl;
        lua_pop(m_L, 1);
        return false;
    }
    return true;
}

bool ScriptEngine::callFunction(const std::string& name) {
    lua_getglobal(m_L, name.c_str());
    if (!lua_isfunction(m_L, -1)) {
        lua_pop(m_L, 1);
        m_lastError = name + " is not a function";
        return false;
    }
    if (lua_pcall(m_L, 0, 0, 0) != LUA_OK) {
        m_lastError = lua_tostring(m_L, -1);
        std::cerr << "Lua call error: " << m_lastError << std::endl;
        lua_pop(m_L, 1);
        return false;
    }
    return true;
}

bool ScriptEngine::callFunction(const std::string& name, const std::vector<LuaValue>& args) {
    lua_getglobal(m_L, name.c_str());
    if (!lua_isfunction(m_L, -1)) {
        lua_pop(m_L, 1);
        m_lastError = name + " is not a function";
        return false;
    }
    for (const auto& arg : args) {
        std::visit([this](auto&& v) {
            using T = std::decay_t<decltype(v)>;
            if constexpr (std::is_same_v<T, bool>) lua_pushboolean(m_L, v);
            else if constexpr (std::is_same_v<T, double>) lua_pushnumber(m_L, v);
            else if constexpr (std::is_same_v<T, std::string>) lua_pushstring(m_L, v.c_str());
            else if constexpr (std::is_same_v<T, void*>) lua_pushlightuserdata(m_L, v);
            else lua_pushnil(m_L);
        }, arg);
    }
    int nargs = args.size();
    if (lua_pcall(m_L, nargs, 0, 0) != LUA_OK) {
        m_lastError = lua_tostring(m_L, -1);
        std::cerr << "Lua call error: " << m_lastError << std::endl;
        lua_pop(m_L, 1);
        return false;
    }
    return true;
}

void ScriptEngine::setGlobal(const std::string& name, const LuaValue& value) {
    std::visit([this, name](auto&& v) {
        using T = std::decay_t<decltype(v)>;
        if constexpr (std::is_same_v<T, bool>) { lua_pushboolean(m_L, v); lua_setglobal(m_L, name.c_str()); }
        else if constexpr (std::is_same_v<T, double>) { lua_pushnumber(m_L, v); lua_setglobal(m_L, name.c_str()); }
        else if constexpr (std::is_same_v<T, std::string>) { lua_pushstring(m_L, v.c_str()); lua_setglobal(m_L, name.c_str()); }
        else if constexpr (std::is_same_v<T, void*>) { lua_pushlightuserdata(m_L, v); lua_setglobal(m_L, name.c_str()); }
    }, value);
}

LuaValue ScriptEngine::getGlobal(const std::string& name) {
    lua_getglobal(m_L, name.c_str());
    LuaValue result;
    int type = lua_type(m_L, -1);
    switch (type) {
        case LUA_TBOOLEAN: result = (bool)lua_toboolean(m_L, -1); break;
        case LUA_TNUMBER:  result = lua_tonumber(m_L, -1); break;
        case LUA_TSTRING:  result = std::string(lua_tostring(m_L, -1)); break;
        default:           result = std::monostate{}; break;
    }
    lua_pop(m_L, 1);
    return result;
}

void ScriptEngine::registerFunction(const std::string& name, lua_CFunction func) {
    lua_register(m_L, name.c_str(), func);
}

void ScriptEngine::registerModule(const std::string& moduleName,
                                  const std::vector<std::pair<std::string, lua_CFunction>>& funcs) {
    lua_newtable(m_L);
    for (const auto& [name, func] : funcs) {
        lua_pushcfunction(m_L, func);
        lua_setfield(m_L, -2, name.c_str());
    }
    lua_setglobal(m_L, moduleName.c_str());
}

bool ScriptEngine::createCoroutine(const std::string& funcName) {
    lua_getglobal(m_L, funcName.c_str());
    if (!lua_isfunction(m_L, -1)) {
        lua_pop(m_L, 1);
        m_lastError = funcName + " is not a function";
        return false;
    }
    m_coroutine = lua_newthread(m_L);
    lua_pushvalue(m_L, -2);  // 复制函数到协程
    lua_xmove(m_L, m_coroutine, 1);
    lua_pop(m_L, 2);  // 移除主栈上的函数和线程
    m_coroutineDone = false;
    return true;
}

int ScriptEngine::resumeCoroutine(int nargs) {
    if (!m_coroutine || m_coroutineDone) return -1;
    int nres = 0;
    int status = lua_resume(m_coroutine, m_L, nargs, &nres);
    if (status == LUA_OK) {
        m_coroutineDone = true;
    } else if (status != LUA_YIELD) {
        m_lastError = lua_tostring(m_coroutine, -1);
        std::cerr << "Coroutine error: " << m_lastError << std::endl;
        m_coroutineDone = true;
    }
    return nres;
}

bool ScriptEngine::isCoroutineDone() const {
    return m_coroutineDone;
}

std::string ScriptEngine::toString(lua_State* L, int idx) {
    if (lua_isstring(L, idx)) return lua_tostring(L, idx);
    return "";
}

double ScriptEngine::toNumber(lua_State* L, int idx) {
    return lua_tonumber(L, idx);
}

bool ScriptEngine::toBool(lua_State* L, int idx) {
    return lua_toboolean(L, idx);
}

int ScriptEngine::errorHandler(lua_State* L) {
    const char* msg = lua_tostring(L, 1);
    luaL_traceback(L, L, msg, 1);
    return 1;
}
