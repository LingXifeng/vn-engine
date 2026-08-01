#pragma once

#include "editor_project.h"
#include <string>

// ============================================================================
// Lua 脚本导出器
// 将编辑器项目导出为可运行的 Lua 游戏脚本
// ============================================================================

class LuaExporter {
public:
    // 导出整个项目到指定目录
    // 生成 main.lua + 各场景脚本
    static bool exportProject(const EditorProject& project, const std::string& outputDir);

    // 导出为单个文件（所有场景合并）
    static bool exportSingleFile(const EditorProject& project, const std::string& outputPath);

private:
    // 生成 main.lua 内容
    static std::string generateMain(const EditorProject& project);

    // 生成场景函数
    static std::string generateScene(const EditorProject& project, const EditorScene& scene);

    // 生成对话行代码
    static std::string generateLine(const EditorProject& project, const EditorLine& line, int indent);

    // 生成角色注册代码
    static std::string generateCharacters(const EditorProject& project);

    // 缩进辅助
    static std::string indentStr(int n);
};
