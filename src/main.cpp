#include "engine.h"
#include <iostream>
#include <string>
#include <chrono>
#include <thread>

// 默认配置
static EngineConfig getDefaultConfig() {
    EngineConfig config;
    config.title = "VN Engine - Visual Novel Demo";
    config.width = 1280;
    config.height = 720;
    config.fullscreen = false;
    config.vsync = true;
    return config;
}

// 解析命令行参数
static EngineConfig parseArgs(int argc, char* argv[], bool& testMode, std::string& screenshotPath, std::string& scriptPath) {
    EngineConfig config = getDefaultConfig();
    testMode = false;
    screenshotPath = "screenshot.bmp";
    scriptPath = "scripts/main.lua";
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--fullscreen" || arg == "-f") {
            config.fullscreen = true;
        } else if (arg == "--width" && i + 1 < argc) {
            config.width = std::stoi(argv[++i]);
        } else if (arg == "--height" && i + 1 < argc) {
            config.height = std::stoi(argv[++i]);
        } else if (arg == "--title" && i + 1 < argc) {
            config.title = argv[++i];
        } else if (arg == "--no-vsync") {
            config.vsync = false;
        } else if (arg == "--test") {
            testMode = true;
        } else if (arg == "--screenshot" && i + 1 < argc) {
            screenshotPath = argv[++i];
        } else if (arg == "--script" && i + 1 < argc) {
            scriptPath = argv[++i];
        } else if (arg == "--help" || arg == "-h") {
            std::cout << "Usage: vn_engine [options]\n"
                      << "Options:\n"
                      << "  --fullscreen, -f    Enable fullscreen\n"
                      << "  --width <px>        Window width (default: 1280)\n"
                      << "  --height <px>       Window height (default: 720)\n"
                      << "  --title <string>    Window title\n"
                      << "  --no-vsync          Disable vsync\n"
                      << "  --test              Run in test mode (save screenshot and exit)\n"
                      << "  --screenshot <path> Screenshot output path (default: screenshot.bmp)\n"
                      << "  --script <path>     Lua script to load (default: scripts/main.lua)\n"
                      << "  --help, -h          Show this help\n";
            exit(0);
        }
    }
    return config;
}

int main(int argc, char* argv[]) {
    std::cout << "========================================\n";
    std::cout << "  VN Engine - C++/SDL2/Lua\n";
    std::cout << "  Visual Novel Engine\n";
    std::cout << "========================================\n\n";

    bool testMode = false;
    std::string screenshotPath;
    std::string scriptPath;
    EngineConfig config = parseArgs(argc, argv, testMode, screenshotPath, scriptPath);

    Engine engine;
    if (!engine.init(config)) {
        std::cerr << "Engine initialization failed!" << std::endl;
        return 1;
    }

    // 加载并执行主脚本
    if (engine.loadScript(scriptPath)) {
        engine.startScript("main");
    } else {
        std::cerr << "Warning: Failed to load scripts/main.lua" << std::endl;
        std::cerr << "Running in empty mode. Press ESC or close window to exit." << std::endl;
    }

    if (testMode) {
        // 测试模式：运行若干帧后截图退出
        std::cout << "Test mode: running 120 frames..." << std::endl;
        for (int i = 0; i < 120; i++) {
            engine.handleEvents();
            engine.update();
            engine.render();
            std::this_thread::sleep_for(std::chrono::milliseconds(16));
        }
        if (engine.saveScreenshot(screenshotPath)) {
            std::cout << "Screenshot saved to " << screenshotPath << std::endl;
        } else {
            std::cerr << "Failed to save screenshot." << std::endl;
        }
    } else {
        // 正常主循环
        engine.run();
    }

    // 清理
    engine.shutdown();

    std::cout << "Goodbye!" << std::endl;
    return 0;
}
