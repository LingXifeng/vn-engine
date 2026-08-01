#pragma once

#include <SDL2/SDL.h>
#include <string>
#include <functional>
#include <memory>

// 游戏状态枚举
enum class GameState {
    LOGO,       // Logo 画面
    TITLE,      // 标题画面
    ADV,        // ADV（文字冒险）模式
    MENU,       // 菜单
    CONFIG,     // 配置
    SAVE_LOAD,  // 存档/读档
    CREDIT,     // 制作人员
    EXIT        // 退出
};

// 引擎配置
struct EngineConfig {
    std::string title = "VN Engine";
    int width = 1280;
    int height = 720;
    bool fullscreen = false;
    bool vsync = true;
};

// 前向声明
class Renderer;
class Audio;
class Input;
class ResourceManager;
class ScriptEngine;
class TextBox;
class Scene;
class TweenManager;
class LuaBindings;
class SaveLoad;
class TitleScreen;
class ConfigScreen;

// 引擎核心类
class Engine {
public:
    Engine();
    ~Engine();

    // 初始化引擎
    bool init(const EngineConfig& config);
    // 主循环
    void run();
    // 关闭引擎
    void shutdown();

    // 状态切换
    void changeState(GameState state);
    GameState getState() const { return m_state; }

    // 获取子系统
    Renderer* getRenderer() { return m_renderer.get(); }
    SDL_Window* getWindow() { return m_window; }
    Audio* getAudio() { return m_audio.get(); }
    Input* getInput() { return m_input.get(); }
    ResourceManager* getResources() { return m_resources.get(); }
    ScriptEngine* getScript() { return m_script.get(); }
    Scene* getScene() { return m_scene.get(); }
    TweenManager* getTweens() { return m_tweens.get(); }
    SaveLoad* getSaveLoad() { return m_saveLoad.get(); }

    // 窗口属性
    int getWidth() const { return m_config.width; }
    int getHeight() const { return m_config.height; }
    void setTitle(const std::string& title);

    // 脚本控制
    bool loadScript(const std::string& path);
    void startScript(const std::string& funcName);

    // 是否运行中
    bool isRunning() const { return m_running; }
    void quit() { m_running = false; }

    // 获取帧时间
    float getDeltaTime() const { return m_deltaTime; }

    // 截图
    bool saveScreenshot(const std::string& path);

    // 单帧执行（用于测试模式）
    void handleEvents();
    void update();
    void render();

private:

    SDL_Window* m_window = nullptr;
    SDL_GLContext m_glContext = nullptr;

    std::unique_ptr<Renderer> m_renderer;
    std::unique_ptr<Audio> m_audio;
    std::unique_ptr<Input> m_input;
    std::unique_ptr<ResourceManager> m_resources;
    std::unique_ptr<ScriptEngine> m_script;
    std::unique_ptr<Scene> m_scene;
    std::unique_ptr<TweenManager> m_tweens;
    std::unique_ptr<LuaBindings> m_bindings;
    std::unique_ptr<SaveLoad> m_saveLoad;
    std::unique_ptr<TitleScreen> m_titleScreen;
    std::unique_ptr<ConfigScreen> m_configScreen;

    EngineConfig m_config;
    GameState m_state = GameState::LOGO;
    bool m_running = false;
    float m_deltaTime = 0.0f;
    Uint64 m_lastTime = 0;
};
