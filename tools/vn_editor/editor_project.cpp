#include "editor_project.h"

#include <lua.hpp>
#include <fstream>
#include <sstream>
#include <cstdio>

// ============================================================================
// EditorProject 方法
// ============================================================================

EditorScene* EditorProject::findScene(const std::string& id) {
    for (auto& s : scenes)
        if (s.id == id) return &s;
    return nullptr;
}

const EditorScene* EditorProject::findScene(const std::string& id) const {
    for (auto& s : scenes)
        if (s.id == id) return &s;
    return nullptr;
}

EditorCharacter* EditorProject::findCharacter(const std::string& id) {
    for (auto& c : characters)
        if (c.id == id) return &c;
    return nullptr;
}

const EditorCharacter* EditorProject::findCharacter(const std::string& id) const {
    for (auto& c : characters)
        if (c.id == id) return &c;
    return nullptr;
}

EditorScene& EditorProject::addScene(const std::string& id, const std::string& name) {
    scenes.emplace_back();
    scenes.back().id = id;
    scenes.back().name = name;
    return scenes.back();
}

EditorCharacter& EditorProject::addCharacter(const std::string& id, const std::string& name) {
    characters.emplace_back();
    characters.back().id = id;
    characters.back().name = name;
    return characters.back();
}

bool EditorProject::removeScene(const std::string& id) {
    for (size_t i = 0; i < scenes.size(); i++) {
        if (scenes[i].id == id) {
            scenes.erase(scenes.begin() + i);
            return true;
        }
    }
    return false;
}

const char* EditorProject::lineTypeName(LineType type) {
    switch (type) {
        case LineType::Narrate: return "narrate";
        case LineType::Say:     return "say";
        case LineType::Choice:  return "choice";
        case LineType::Goto:    return "goto";
        case LineType::Wait:    return "wait";
        case LineType::BGM:     return "bgm";
        case LineType::BGMStop: return "bgm_stop";
        case LineType::SFX:     return "sfx";
        case LineType::BG:      return "bg";
        case LineType::FadeIn:  return "fadein";
        case LineType::FadeOut: return "fadeout";
        case LineType::Ending:  return "ending";
        case LineType::Label:   return "label";
    }
    return "say";
}

LineType EditorProject::lineTypeFromName(const std::string& name) {
    if (name == "narrate")  return LineType::Narrate;
    if (name == "say")      return LineType::Say;
    if (name == "choice")   return LineType::Choice;
    if (name == "goto")     return LineType::Goto;
    if (name == "wait")     return LineType::Wait;
    if (name == "bgm")      return LineType::BGM;
    if (name == "bgm_stop") return LineType::BGMStop;
    if (name == "sfx")      return LineType::SFX;
    if (name == "bg")       return LineType::BG;
    if (name == "fadein")   return LineType::FadeIn;
    if (name == "fadeout")  return LineType::FadeOut;
    if (name == "ending")   return LineType::Ending;
    if (name == "label")    return LineType::Label;
    return LineType::Say;
}

// ============================================================================
// ProjectFile — Lua 加载/保存
// ============================================================================

// 辅助：从 Lua 表读取字符串字段
static std::string luaGetString(lua_State* L, const char* key, const char* def = "") {
    lua_getfield(L, -1, key);
    const char* s = lua_tostring(L, -1);
    std::string result = s ? s : def;
    lua_pop(L, 1);
    return result;
}

// 辅助：从 Lua 表读取数字字段
static float luaGetNumber(lua_State* L, const char* key, float def = 0.0f) {
    lua_getfield(L, -1, key);
    float result = lua_isnumber(L, -1) ? (float)lua_tonumber(L, -1) : def;
    lua_pop(L, 1);
    return result;
}

// 辅助：从 Lua 表读取整数字段
static int luaGetInteger(lua_State* L, const char* key, int def = 0) {
    lua_getfield(L, -1, key);
    int result = lua_isinteger(L, -1) ? (int)lua_tointeger(L, -1) : def;
    lua_pop(L, 1);
    return result;
}

bool ProjectFile::load(EditorProject& project, const std::string& path) {
    lua_State* L = luaL_newstate();
    luaL_openlibs(L);

    if (luaL_loadfile(L, path.c_str()) != LUA_OK || lua_pcall(L, 0, 1, 0) != LUA_OK) {
        fprintf(stderr, "Error loading project: %s\n", lua_tostring(L, -1));
        lua_close(L);
        return false;
    }

    if (!lua_istable(L, -1)) {
        fprintf(stderr, "Project file must return a table\n");
        lua_close(L);
        return false;
    }

    // 读取基本字段
    project.name = luaGetString(L, "name", "Untitled");
    project.author = luaGetString(L, "author", "Unknown");
    project.version = luaGetString(L, "version", "1.0");
    project.startScene = luaGetString(L, "start_scene", "scene1");

    // 读取角色列表
    lua_getfield(L, -1, "characters");
    if (lua_istable(L, -1)) {
        int len = (int)luaL_len(L, -1);
        for (int i = 1; i <= len; i++) {
            lua_geti(L, -1, i);
            if (lua_istable(L, -1)) {
                EditorCharacter ch;
                ch.id = luaGetString(L, "id");
                ch.name = luaGetString(L, "name", ch.id.c_str());
                ch.colorR = luaGetInteger(L, "r", 255);
                ch.colorG = luaGetInteger(L, "g", 255);
                ch.colorB = luaGetInteger(L, "b", 255);
                ch.voicePrefix = luaGetString(L, "voice_prefix");

                // 读取表情列表
                lua_getfield(L, -1, "expressions");
                if (lua_istable(L, -1)) {
                    int elen = (int)luaL_len(L, -1);
                    for (int j = 1; j <= elen; j++) {
                        lua_geti(L, -1, j);
                        if (lua_isstring(L, -1))
                            ch.expressions.push_back(lua_tostring(L, -1));
                        lua_pop(L, 1);
                    }
                }
                lua_pop(L, 1);

                project.characters.push_back(ch);
            }
            lua_pop(L, 1);
        }
    }
    lua_pop(L, 1);

    // 读取场景列表
    lua_getfield(L, -1, "scenes");
    if (lua_istable(L, -1)) {
        int len = (int)luaL_len(L, -1);
        for (int i = 1; i <= len; i++) {
            lua_geti(L, -1, i);
            if (lua_istable(L, -1)) {
                EditorScene sc;
                sc.id = luaGetString(L, "id");
                sc.name = luaGetString(L, "name", sc.id.c_str());
                sc.background = luaGetString(L, "background");
                sc.bgm = luaGetString(L, "bgm");

                // 读取对话行
                lua_getfield(L, -1, "lines");
                if (lua_istable(L, -1)) {
                    int llen = (int)luaL_len(L, -1);
                    for (int j = 1; j <= llen; j++) {
                        lua_geti(L, -1, j);
                        if (lua_istable(L, -1)) {
                            EditorLine line;
                            std::string typeName = luaGetString(L, "type", "say");
                            line.type = EditorProject::lineTypeFromName(typeName);
                            line.text = luaGetString(L, "text");
                            line.character = luaGetString(L, "character");
                            line.expression = luaGetString(L, "expression");
                            line.resourceId = luaGetString(L, "resource");
                            line.waitTime = luaGetNumber(L, "time", 0.0f);
                            line.fadeTime = luaGetNumber(L, "fade", 0.5f);
                            line.targetScene = luaGetString(L, "target");
                            line.label = luaGetString(L, "label");
                            line.endingId = luaGetString(L, "ending_id");
                            line.endingTitle = luaGetString(L, "ending_title");
                            line.endingDesc = luaGetString(L, "ending_desc");

                            // 读取选项
                            lua_getfield(L, -1, "options");
                            if (lua_istable(L, -1)) {
                                int olen = (int)luaL_len(L, -1);
                                for (int k = 1; k <= olen; k++) {
                                    lua_geti(L, -1, k);
                                    if (lua_istable(L, -1)) {
                                        EditorChoice ch;
                                        ch.text = luaGetString(L, "text");
                                        ch.nextScene = luaGetString(L, "next");
                                        line.choices.push_back(ch);
                                    }
                                    lua_pop(L, 1);
                                }
                            }
                            lua_pop(L, 1);

                            sc.lines.push_back(line);
                        }
                        lua_pop(L, 1);
                    }
                }
                lua_pop(L, 1);

                project.scenes.push_back(sc);
            }
            lua_pop(L, 1);
        }
    }
    lua_pop(L, 1);

    // 读取资源列表
    lua_getfield(L, -1, "resources");
    if (lua_istable(L, -1)) {
        int len = (int)luaL_len(L, -1);
        for (int i = 1; i <= len; i++) {
            lua_geti(L, -1, i);
            if (lua_istable(L, -1)) {
                EditorResource res;
                res.id = luaGetString(L, "id");
                res.path = luaGetString(L, "path");
                res.type = luaGetString(L, "type", "image");
                project.resources.push_back(res);
            }
            lua_pop(L, 1);
        }
    }
    lua_pop(L, 1);

    lua_close(L);
    return true;
}

// ============================================================================
// 保存 — 生成 Lua 代码
// ============================================================================

static std::string escapeLuaString(const std::string& s) {
    std::string result;
    for (char c : s) {
        if (c == '"') result += "\\\"";
        else if (c == '\\') result += "\\\\";
        else if (c == '\n') result += "\\n";
        else result += c;
    }
    return result;
}

bool ProjectFile::save(const EditorProject& project, const std::string& path) {
    std::ofstream ofs(path);
    if (!ofs) return false;

    ofs << "-- VN Engine Project File\n";
    ofs << "-- Generated by vn_editor\n\n";

    ofs << "return {\n";
    ofs << "    name = \"" << escapeLuaString(project.name) << "\",\n";
    ofs << "    author = \"" << escapeLuaString(project.author) << "\",\n";
    ofs << "    version = \"" << escapeLuaString(project.version) << "\",\n";
    ofs << "    start_scene = \"" << escapeLuaString(project.startScene) << "\",\n\n";

    // 角色
    ofs << "    characters = {\n";
    for (const auto& ch : project.characters) {
        ofs << "        {\n";
        ofs << "            id = \"" << escapeLuaString(ch.id) << "\",\n";
        ofs << "            name = \"" << escapeLuaString(ch.name) << "\",\n";
        ofs << "            r = " << ch.colorR << ", g = " << ch.colorG << ", b = " << ch.colorB << ",\n";
        if (!ch.voicePrefix.empty())
            ofs << "            voice_prefix = \"" << escapeLuaString(ch.voicePrefix) << "\",\n";
        ofs << "            expressions = {";
        for (size_t i = 0; i < ch.expressions.size(); i++) {
            if (i > 0) ofs << ", ";
            ofs << "\"" << escapeLuaString(ch.expressions[i]) << "\"";
        }
        ofs << "},\n";
        ofs << "        },\n";
    }
    ofs << "    },\n\n";

    // 场景
    ofs << "    scenes = {\n";
    for (const auto& sc : project.scenes) {
        ofs << "        {\n";
        ofs << "            id = \"" << escapeLuaString(sc.id) << "\",\n";
        ofs << "            name = \"" << escapeLuaString(sc.name) << "\",\n";
        if (!sc.background.empty())
            ofs << "            background = \"" << escapeLuaString(sc.background) << "\",\n";
        if (!sc.bgm.empty())
            ofs << "            bgm = \"" << escapeLuaString(sc.bgm) << "\",\n";

        ofs << "            lines = {\n";
        for (const auto& line : sc.lines) {
            ofs << "                {type = \"" << EditorProject::lineTypeName(line.type) << "\"";
            if (!line.text.empty())
                ofs << ", text = \"" << escapeLuaString(line.text) << "\"";
            if (!line.character.empty())
                ofs << ", character = \"" << escapeLuaString(line.character) << "\"";
            if (!line.expression.empty())
                ofs << ", expression = \"" << escapeLuaString(line.expression) << "\"";
            if (!line.resourceId.empty())
                ofs << ", resource = \"" << escapeLuaString(line.resourceId) << "\"";
            if (line.waitTime > 0.0f)
                ofs << ", time = " << line.waitTime;
            if (line.fadeTime > 0.0f)
                ofs << ", fade = " << line.fadeTime;
            if (!line.targetScene.empty())
                ofs << ", target = \"" << escapeLuaString(line.targetScene) << "\"";
            if (!line.label.empty())
                ofs << ", label = \"" << escapeLuaString(line.label) << "\"";
            if (!line.endingId.empty())
                ofs << ", ending_id = \"" << escapeLuaString(line.endingId) << "\"";
            if (!line.endingTitle.empty())
                ofs << ", ending_title = \"" << escapeLuaString(line.endingTitle) << "\"";
            if (!line.endingDesc.empty())
                ofs << ", ending_desc = \"" << escapeLuaString(line.endingDesc) << "\"";

            if (!line.choices.empty()) {
                ofs << ", options = {\n";
                for (const auto& ch : line.choices) {
                    ofs << "                    {text = \"" << escapeLuaString(ch.text)
                        << "\", next = \"" << escapeLuaString(ch.nextScene) << "\"},\n";
                }
                ofs << "                }";
            }
            ofs << "},\n";
        }
        ofs << "            },\n";
        ofs << "        },\n";
    }
    ofs << "    },\n\n";

    // 资源
    ofs << "    resources = {\n";
    for (const auto& res : project.resources) {
        ofs << "        {id = \"" << escapeLuaString(res.id)
            << "\", path = \"" << escapeLuaString(res.path)
            << "\", type = \"" << escapeLuaString(res.type) << "\"},\n";
    }
    ofs << "    },\n";

    ofs << "}\n";
    ofs.close();
    return true;
}

EditorProject ProjectFile::createNew(const std::string& name) {
    EditorProject project;
    project.name = name;
    project.author = "Unknown";
    project.version = "1.0";
    project.startScene = "scene1";

    // 添加默认角色
    auto& ch = project.addCharacter("narrator", "旁白");
    ch.colorR = 200; ch.colorG = 200; ch.colorB = 200;

    // 添加默认场景
    auto& sc = project.addScene("scene1", "Scene 1");
    sc.lines.emplace_back();
    sc.lines.back().type = LineType::Narrate;
    sc.lines.back().text = "故事从这里开始...";

    return project;
}
