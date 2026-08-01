#pragma once

#include <lua.hpp>
#include <string>
#include <functional>
#include <unordered_map>
#include <variant>
#include <vector>

// Lua 值类型（用于 C++ <-> Lua 数据传递）
using LuaValue = std::variant<std::monostate, bool, double, std::string, void*>;

// 脚本引擎 - Lua 解释器封装
class ScriptEngine {
public:
    ScriptEngine();
    ~ScriptEngine();

    // 初始化
    bool init();

    // 脚本加载与执行
    bool loadFile(const std::string& path);
    bool loadString(const std::string& code);
    bool execute();

    // 调用 Lua 函数
    bool callFunction(const std::string& name);
    bool callFunction(const std::string& name, const std::vector<LuaValue>& args);

    // 获取/设置全局变量
    void setGlobal(const std::string& name, const LuaValue& value);
    LuaValue getGlobal(const std::string& name);

    // 注册 C++ 函数到 Lua
    void registerFunction(const std::string& name, lua_CFunction func);

    // 注册模块（一组函数）
    void registerModule(const std::string& moduleName,
                        const std::vector<std::pair<std::string, lua_CFunction>>& funcs);

    // 协程支持（用于脚本暂停/恢复）
    bool createCoroutine(const std::string& funcName);
    int resumeCoroutine(int nargs = 0);
    bool isCoroutineDone() const;

    // 获取 Lua state
    lua_State* getState() { return m_L; }

    // 错误处理
    std::string getLastError() const { return m_lastError; }

    // 获取栈上的参数（供 C++ 函数使用）
    static std::string toString(lua_State* L, int idx);
    static double toNumber(lua_State* L, int idx);
    static bool toBool(lua_State* L, int idx);

private:
    lua_State* m_L = nullptr;
    lua_State* m_coroutine = nullptr;
    bool m_coroutineDone = true;
    std::string m_lastError;

    static int errorHandler(lua_State* L);
};
