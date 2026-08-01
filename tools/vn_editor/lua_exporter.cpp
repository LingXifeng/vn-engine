#include "lua_exporter.h"

#include <fstream>
#include <sstream>
#include <filesystem>

// ============================================================================
// LuaExporter 实现
// ============================================================================

std::string LuaExporter::indentStr(int n) {
    return std::string(n * 4, ' ');
}

static std::string escapeText(const std::string& s) {
    std::string result;
    for (char c : s) {
        if (c == '"') result += "\\\"";
        else if (c == '\\') result += "\\\\";
        else if (c == '\n') result += "\\n";
        else result += c;
    }
    return result;
}

static bool ensureDir(const std::string& dir) {
    // 创建目录（递归，跨平台）
    std::error_code ec;
    std::filesystem::create_directories(dir, ec);
    return !ec;
}

std::string LuaExporter::generateCharacters(const EditorProject& project) {
    std::ostringstream oss;
    if (project.characters.empty()) return "";

    oss << "-- 角色定义\n";
    for (const auto& ch : project.characters) {
        if (ch.id == "narrator") continue;
        oss << "Character.define(\"" << escapeText(ch.id) << "\", \"" << escapeText(ch.name) << "\")\n";
        if (!ch.expressions.empty()) {
            for (const auto& expr : ch.expressions) {
                oss << "Character.addExpression(\"" << escapeText(ch.id)
                    << "\", \"" << escapeText(expr) << "\", \""
                    << escapeText(ch.id) << "_" << escapeText(expr) << ".png\")\n";
            }
        }
    }
    oss << "\n";
    return oss.str();
}

std::string LuaExporter::generateLine(const EditorProject& project, const EditorLine& line, int indent) {
    std::ostringstream oss;
    std::string ind = indentStr(indent);

    switch (line.type) {
        case LineType::Narrate:
            oss << ind << "Text.narrate(\"" << escapeText(line.text) << "\")\n";
            break;

        case LineType::Say: {
            // 查找角色显示名
            std::string displayName = line.character;
            const auto* ch = project.findCharacter(line.character);
            if (ch) displayName = ch->name;

            if (!line.expression.empty()) {
                oss << ind << "Character.show(\"" << escapeText(line.character)
                    << "\", \"" << escapeText(line.expression) << "\")\n";
            }
            oss << ind << "Text.say(\"" << escapeText(displayName)
                << "\", \"" << escapeText(line.text) << "\")\n";
            break;
        }

        case LineType::Choice: {
            // 生成选项
            oss << ind << "Flow.branch({";
            for (size_t i = 0; i < line.choices.size(); i++) {
                if (i > 0) oss << ", ";
                oss << "\"" << escapeText(line.choices[i].text) << "\"";
            }
            oss << "})\n";

            // 生成分支
            oss << ind << "local sel = Flow.getSelection()\n";
            for (size_t i = 0; i < line.choices.size(); i++) {
                oss << ind << (i == 0 ? "if" : "elseif")
                    << " sel == " << (i + 1) << " then\n";
                if (!line.choices[i].nextScene.empty()) {
                    oss << ind << "    " << line.choices[i].nextScene << "()\n";
                }
            }
            if (!line.choices.empty()) {
                oss << ind << "end\n";
            }
            break;
        }

        case LineType::Goto:
            oss << ind << line.targetScene << "()\n";
            break;

        case LineType::Wait:
            oss << ind << "Flow.wait(" << line.waitTime << ")\n";
            break;

        case LineType::BGM:
            oss << ind << "Audio.playBGM(\"" << escapeText(line.resourceId) << "\")\n";
            break;

        case LineType::BGMStop:
            oss << ind << "Audio.stopBGM()\n";
            break;

        case LineType::SFX:
            oss << ind << "Audio.playSFX(\"" << escapeText(line.resourceId) << "\")\n";
            break;

        case LineType::BG:
            oss << ind << "Scene.changeBG(\"" << escapeText(line.resourceId) << "\")\n";
            break;

        case LineType::FadeIn:
            oss << ind << "Scene.fadeIn(" << line.fadeTime << ")\n";
            break;

        case LineType::FadeOut:
            oss << ind << "Scene.fadeOut(" << line.fadeTime << ")\n";
            break;

        case LineType::Ending:
            oss << ind << "Ending.add(\"" << escapeText(line.endingId)
                << "\", \"" << escapeText(line.endingTitle)
                << "\", \"" << escapeText(line.endingDesc) << "\")\n";
            oss << ind << "Flow.endGame()\n";
            break;

        case LineType::Label:
            oss << ind << "-- ::" << line.label << "::\n";
            break;
    }

    return oss.str();
}

std::string LuaExporter::generateScene(const EditorProject& project, const EditorScene& scene) {
    std::ostringstream oss;

    oss << "-- 场景: " << scene.name << "\n";
    oss << "function " << scene.id << "()\n";

    // 设置默认背景和BGM
    if (!scene.background.empty()) {
        oss << "    Scene.changeBG(\"" << escapeText(scene.background) << "\")\n";
    }
    if (!scene.bgm.empty()) {
        oss << "    Audio.playBGM(\"" << escapeText(scene.bgm) << "\")\n";
    }

    // 生成对话行
    for (const auto& line : scene.lines) {
        oss << generateLine(project, line, 1);
    }

    oss << "end\n\n";
    return oss.str();
}

std::string LuaExporter::generateMain(const EditorProject& project) {
    std::ostringstream oss;

    oss << "-- ============================================================\n";
    oss << "-- " << project.name << "\n";
    oss << "-- 作者: " << project.author << "  版本: " << project.version << "\n";
    oss << "-- 由 vn_editor 自动生成\n";
    oss << "-- ============================================================\n\n";

    // 全局状态
    oss << "game = { chapter = 1, flags = {} }\n\n";

    // 角色定义
    oss << generateCharacters(project);

    // 场景函数
    for (const auto& scene : project.scenes) {
        oss << generateScene(project, scene);
    }

    // 主函数
    oss << "-- 主函数（由引擎作为协程启动）\n";
    oss << "function main()\n";
    oss << "    System.setTitle(\"" << escapeText(project.name) << "\")\n";
    oss << "    System.log(\"=== " << escapeText(project.name) << " ===\")\n";
    oss << "    Scene.fadeIn(0.5)\n";
    if (!project.startScene.empty()) {
        oss << "    " << project.startScene << "()\n";
    }
    oss << "    System.log(\"=== 游戏结束 ===\")\n";
    oss << "end\n";

    return oss.str();
}

bool LuaExporter::exportSingleFile(const EditorProject& project, const std::string& outputPath) {
    std::string content = generateMain(project);

    std::ofstream ofs(outputPath);
    if (!ofs) return false;
    ofs << content;
    ofs.close();
    return true;
}

bool LuaExporter::exportProject(const EditorProject& project, const std::string& outputDir) {
    if (!ensureDir(outputDir)) return false;

    // 导出 main.lua
    std::string mainPath = outputDir + "/main.lua";
    if (!exportSingleFile(project, mainPath)) return false;

    // 导出项目信息文件
    std::ofstream info(outputDir + "/project_info.txt");
    if (info) {
        info << "Project: " << project.name << "\n";
        info << "Author: " << project.author << "\n";
        info << "Version: " << project.version << "\n";
        info << "Scenes: " << project.scenes.size() << "\n";
        info << "Characters: " << project.characters.size() << "\n";
        info.close();
    }

    return true;
}
