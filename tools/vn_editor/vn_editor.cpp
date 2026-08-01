// ============================================================================
// vn_editor — VN 可视化脚本编辑器
// 用法:
//   vn_editor                    — 启动 GUI 编辑器（空项目）
//   vn_editor --new              — 启动 GUI 编辑器（新建项目）
//   vn_editor --open <file.lua>  — 打开项目文件
//   vn_editor --export <out.lua> — 导出 Lua 游戏脚本
//   vn_editor --cli              — CLI 模式（无图形界面）
//   vn_editor --cli --open <f> --export <o>  — CLI 批量导出
//   vn_editor --help             — 显示帮助
// ============================================================================

#include "editor_project.h"
#include "lua_exporter.h"

#include <iostream>
#include <string>
#include <vector>

// GUI 模式需要 SDL2
#ifdef USE_GUI
#include "editor_ui.h"
#include <SDL2/SDL.h>
#endif

// ============================================================================
// 命令行参数解析
// ============================================================================

struct CLIArgs {
    bool showHelp = false;
    bool newProject = false;
    bool cliMode = false;
    std::string openPath;
    std::string exportPath;
    std::string savePath;
    bool listScenes = false;
    bool validate = false;
};

static void printHelp() {
    std::cout << R"(
╔══════════════════════════════════════════════════════════╗
║           VN 可视化脚本编辑器 (vn_editor)                 ║
╠══════════════════════════════════════════════════════════╣
║ 用法:                                                     ║
║   vn_editor [选项]                                        ║
║                                                           ║
║ 选项:                                                     ║
║   --new              新建项目并启动 GUI                    ║
║   --open <path>      打开项目文件 (.lua)                   ║
║   --save <path>      保存项目到文件                        ║
║   --export <path>    导出为 Lua 游戏脚本                   ║
║   --cli              CLI 模式（无图形界面）                ║
║   --list             列出项目中的场景                      ║
║   --validate         验证项目文件                          ║
║   --help             显示此帮助                            ║
║                                                           ║
║ 示例:                                                     ║
║   vn_editor --new                                        ║
║   vn_editor --open myproject.lua                          ║
║   vn_editor --cli --open proj.lua --export game.lua       ║
║   vn_editor --cli --open proj.lua --list                  ║
╚══════════════════════════════════════════════════════════╝
)" << std::endl;
}

static CLIArgs parseArgs(int argc, char* argv[]) {
    CLIArgs args;
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--help" || arg == "-h") {
            args.showHelp = true;
        } else if (arg == "--new") {
            args.newProject = true;
        } else if (arg == "--cli") {
            args.cliMode = true;
        } else if (arg == "--open" && i + 1 < argc) {
            args.openPath = argv[++i];
        } else if (arg == "--export" && i + 1 < argc) {
            args.exportPath = argv[++i];
        } else if (arg == "--save" && i + 1 < argc) {
            args.savePath = argv[++i];
        } else if (arg == "--list") {
            args.listScenes = true;
        } else if (arg == "--validate") {
            args.validate = true;
        }
    }
    return args;
}

// ============================================================================
// CLI 模式
// ============================================================================

static int runCLI(const CLIArgs& args) {
    EditorProject project;

    // 加载或新建项目
    if (!args.openPath.empty()) {
        std::cout << "加载项目: " << args.openPath << std::endl;
        if (!ProjectFile::load(project, args.openPath)) {
            std::cerr << "错误: 无法加载项目文件: " << args.openPath << std::endl;
            return 1;
        }
        std::cout << "  项目名: " << project.name << std::endl;
        std::cout << "  作者:   " << project.author << std::endl;
        std::cout << "  版本:   " << project.version << std::endl;
        std::cout << "  场景数: " << project.scenes.size() << std::endl;
        std::cout << "  角色数: " << project.characters.size() << std::endl;
    } else if (args.newProject) {
        project = ProjectFile::createNew("新项目");
        std::cout << "已创建新项目" << std::endl;
    } else {
        std::cerr << "错误: CLI 模式需要 --open <file> 或 --new" << std::endl;
        return 1;
    }

    // 列出场景
    if (args.listScenes) {
        std::cout << "\n=== 场景列表 ===" << std::endl;
        for (size_t i = 0; i < project.scenes.size(); i++) {
            auto& s = project.scenes[i];
            std::cout << "  [" << i << "] " << s.id << " — " << s.name
                      << " (" << s.lines.size() << " 行)" << std::endl;
        }
        std::cout << "\n=== 角色列表 ===" << std::endl;
        for (size_t i = 0; i < project.characters.size(); i++) {
            auto& ch = project.characters[i];
            std::cout << "  [" << i << "] " << ch.id << " — " << ch.name << std::endl;
        }
    }

    // 验证项目
    if (args.validate) {
        std::cout << "\n=== 项目验证 ===" << std::endl;
        bool valid = true;

        // 检查起始场景存在
        if (!project.startScene.empty()) {
            bool found = false;
            for (auto& s : project.scenes) {
                if (s.id == project.startScene) { found = true; break; }
            }
            if (!found) {
                std::cerr << "  ⚠ 起始场景不存在: " << project.startScene << std::endl;
                valid = false;
            } else {
                std::cout << "  ✓ 起始场景: " << project.startScene << std::endl;
            }
        }

        // 检查场景 ID 唯一
        for (size_t i = 0; i < project.scenes.size(); i++) {
            for (size_t j = i + 1; j < project.scenes.size(); j++) {
                if (project.scenes[i].id == project.scenes[j].id) {
                    std::cerr << "  ⚠ 场景ID重复: " << project.scenes[i].id << std::endl;
                    valid = false;
                }
            }
        }

        // 检查跳转目标存在
        for (auto& s : project.scenes) {
            for (auto& line : s.lines) {
                if (line.type == LineType::Goto && !line.targetScene.empty()) {
                    bool found = false;
                    for (auto& s2 : project.scenes) {
                        if (s2.id == line.targetScene) { found = true; break; }
                    }
                    if (!found) {
                        std::cerr << "  ⚠ 场景 " << s.id << " 中跳转目标不存在: "
                                  << line.targetScene << std::endl;
                        valid = false;
                    }
                }
                if (line.type == LineType::Choice) {
                    for (auto& ch : line.choices) {
                        if (!ch.nextScene.empty()) {
                            bool found = false;
                            for (auto& s2 : project.scenes) {
                                if (s2.id == ch.nextScene) { found = true; break; }
                            }
                            if (!found) {
                                std::cerr << "  ⚠ 场景 " << s.id << " 中选项跳转目标不存在: "
                                          << ch.nextScene << std::endl;
                                valid = false;
                            }
                        }
                    }
                }
            }
        }

        if (valid) {
            std::cout << "  ✓ 项目验证通过" << std::endl;
        } else {
            std::cerr << "  ✗ 项目验证失败" << std::endl;
            return 1;
        }
    }

    // 保存
    if (!args.savePath.empty()) {
        std::cout << "保存项目到: " << args.savePath << std::endl;
        if (!ProjectFile::save(project, args.savePath)) {
            std::cerr << "错误: 保存失败" << std::endl;
            return 1;
        }
        std::cout << "  ✓ 已保存" << std::endl;
    }

    // 导出
    if (!args.exportPath.empty()) {
        std::cout << "导出游戏脚本到: " << args.exportPath << std::endl;
        if (!LuaExporter::exportSingleFile(project, args.exportPath)) {
            std::cerr << "错误: 导出失败" << std::endl;
            return 1;
        }
        std::cout << "  ✓ 已导出" << std::endl;
        std::cout << "  运行: vn_engine --script " << args.exportPath << std::endl;
    }

    return 0;
}

// ============================================================================
// GUI 模式
// ============================================================================

#ifdef USE_GUI
static int runGUI(const CLIArgs& args) {
    EditorProject project;

    // 加载或新建项目
    if (!args.openPath.empty()) {
        if (!ProjectFile::load(project, args.openPath)) {
            std::cerr << "错误: 无法加载项目文件: " << args.openPath << std::endl;
            return 1;
        }
        std::cout << "已加载项目: " << project.name << std::endl;
    } else {
        project = ProjectFile::createNew("新项目");
    }

    // 初始化 SDL2
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL2 初始化失败: " << SDL_GetError() << std::endl;
        std::cerr << "提示: 在无显示环境请使用 --cli 模式" << std::endl;
        return 1;
    }
    if (TTF_Init() < 0) {
        std::cerr << "SDL_ttf 初始化失败: " << TTF_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    // 创建编辑器 UI
    EditorUI editor;
    if (!editor.init(1280, 800)) {
        std::cerr << "编辑器初始化失败" << std::endl;
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    // 设置项目路径
    if (!args.openPath.empty()) {
        // editor 内部通过 menuSave 使用 m_projectPath
    }

    // 运行编辑器
    editor.run(project, args.exportPath);

    // 清理
    editor.shutdown();
    TTF_Quit();
    SDL_Quit();

    return 0;
}
#endif

// ============================================================================
// 主函数
// ============================================================================

int main(int argc, char* argv[]) {
    CLIArgs args = parseArgs(argc, argv);

    if (args.showHelp) {
        printHelp();
        return 0;
    }

    // CLI 模式
    if (args.cliMode) {
        return runCLI(args);
    }

    // GUI 模式
#ifdef USE_GUI
    return runGUI(args);
#else
    // 无 GUI 支持，回退到 CLI
    if (args.openPath.empty() && !args.newProject) {
        printHelp();
        return 0;
    }
    args.cliMode = true;
    return runCLI(args);
#endif
}
