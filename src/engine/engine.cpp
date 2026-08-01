#include "engine.h"
#include "renderer.h"
#include "audio.h"
#include "input.h"
#include "resource_manager.h"
#include "script_engine.h"
#include "tween.h"
#include "../adv/scene.h"
#include "../script/lua_bindings.h"
#include "../ui/title_screen.h"
#include "../ui/config_screen.h"
#include "save_load.h"
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include <iostream>

Engine::Engine() {}

Engine::~Engine() {
    shutdown();
}

bool Engine::init(const EngineConfig& config) {
    m_config = config;

    // 初始化 SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER) < 0) {
        std::cerr << "SDL_Init failed: " << SDL_GetError() << std::endl;
        return false;
    }

    // 初始化 SDL_image
    int imgFlags = IMG_INIT_PNG | IMG_INIT_JPG;
    if (!(IMG_Init(imgFlags) & imgFlags)) {
        std::cerr << "IMG_Init failed: " << IMG_GetError() << std::endl;
        return false;
    }

    // 初始化 SDL_ttf
    if (TTF_Init() < 0) {
        std::cerr << "TTF_Init failed: " << TTF_GetError() << std::endl;
        return false;
    }

    // 初始化 SDL_mixer（失败时继续，允许无音频环境运行）
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        std::cerr << "Warning: Mix_OpenAudio failed: " << Mix_GetError() << std::endl;
        std::cerr << "Continuing without audio." << std::endl;
    } else {
        Mix_AllocateChannels(32); // 32 个音效通道
    }

    // 创建窗口
    Uint32 windowFlags = SDL_WINDOW_SHOWN;
    if (config.fullscreen) windowFlags |= SDL_WINDOW_FULLSCREEN;
    if (config.vsync) windowFlags |= SDL_RENDERER_PRESENTVSYNC;

    m_window = SDL_CreateWindow(
        config.title.c_str(),
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        config.width, config.height,
        windowFlags
    );
    if (!m_window) {
        std::cerr << "Window creation failed: " << SDL_GetError() << std::endl;
        return false;
    }

    // 创建子系统
    m_renderer = std::make_unique<Renderer>(m_window, config.width, config.height);
    if (!m_renderer->init()) return false;

    m_audio = std::make_unique<Audio>();
    m_input = std::make_unique<Input>();
    m_resources = std::make_unique<ResourceManager>(m_renderer.get());
    m_script = std::make_unique<ScriptEngine>();
    m_tweens = std::make_unique<TweenManager>();

    if (!m_script->init()) return false;

    // 创建场景
    m_scene = std::make_unique<Scene>(m_renderer.get(), m_resources.get(), m_tweens.get());

    // 加载默认字体
    TTF_Font* defaultFont = m_renderer->loadFont("assets/fonts/DroidSansJapanese.ttf", 24);
    if (defaultFont) {
        m_scene->getTextBox()->setFont(defaultFont);
        std::cout << "Default font loaded." << std::endl;
    } else {
        std::cerr << "Warning: Failed to load default font." << std::endl;
    }

    // 创建 Lua 绑定
    m_bindings = std::make_unique<LuaBindings>(
        m_script.get(), this, m_scene.get(),
        m_audio.get(), m_resources.get(), m_tweens.get());
    m_bindings->registerAll();

    // 创建存档系统
    m_saveLoad = std::make_unique<SaveLoad>(m_script.get());

    // 创建标题画面和配置界面
    m_titleScreen = std::make_unique<TitleScreen>(m_renderer.get(), m_resources.get());
    m_configScreen = std::make_unique<ConfigScreen>(m_renderer.get(), m_audio.get());

    m_lastTime = SDL_GetPerformanceCounter();
    m_running = true;
    m_state = GameState::LOGO;

    std::cout << "Engine initialized successfully." << std::endl;
    return true;
}

void Engine::run() {
    while (m_running) {
        handleEvents();
        update();
        render();
    }
}

void Engine::handleEvents() {
    m_input->update();
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        m_input->handleEvent(event);
        if (event.type == SDL_QUIT) {
            m_running = false;
        }
        if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) {
            // ESC 键：根据状态决定行为
            if (m_state == GameState::ADV) {
                changeState(GameState::MENU);
            } else if (m_state == GameState::MENU) {
                changeState(GameState::ADV);
            }
        }
    }
}

void Engine::update() {
    // 计算帧时间
    Uint64 currentTime = SDL_GetPerformanceCounter();
    m_deltaTime = (float)(currentTime - m_lastTime) / SDL_GetPerformanceFrequency();
    m_lastTime = currentTime;

    // 更新音频
    m_audio->update(m_deltaTime);

    // 更新动画
    m_tweens->update(m_deltaTime);

    // 更新场景
    if (m_scene) {
        m_scene->update(m_deltaTime, *m_input);

        // 处理脚本协程恢复
        if (!m_script->isCoroutineDone()) {
            // 如果对话等待点击或选项已选择，恢复协程
            if (m_scene->isDialogueWaiting() && m_input->hasClick()) {
                if (m_scene->advanceDialogue()) {
                    m_script->resumeCoroutine(0);
                }
            }
        }
    }

    // 更新 UI 组件（粒子、画廊、音乐室、Credits 等）
    if (m_bindings) {
        m_bindings->updateUI(m_deltaTime, *m_input);
    }

    // 状态机驱动的 UI
    switch (m_state) {
        case GameState::TITLE:
            if (m_titleScreen) m_titleScreen->update(m_deltaTime, *m_input);
            break;
        case GameState::CONFIG:
            if (m_configScreen) m_configScreen->update(m_deltaTime, *m_input);
            break;
        default:
            break;
    }
}

void Engine::render() {
    m_renderer->clear();

    // 渲染场景
    if (m_scene) {
        m_scene->render();
    }

    // 渲染 UI 组件（粒子特效、画廊、音乐室、Credits 等）
    if (m_bindings) {
        m_bindings->renderUI();
    }

    // 状态机驱动的 UI 渲染
    switch (m_state) {
        case GameState::TITLE:
            if (m_titleScreen) m_titleScreen->render();
            break;
        case GameState::CONFIG:
            if (m_configScreen) m_configScreen->render();
            break;
        default:
            break;
    }

    m_renderer->present();
}

void Engine::changeState(GameState state) {
    m_state = state;
}

void Engine::shutdown() {
    if (!m_window) return;

    m_saveLoad.reset();
    m_configScreen.reset();
    m_titleScreen.reset();
    m_bindings.reset();
    m_scene.reset();
    m_tweens.reset();
    m_script.reset();
    m_resources.reset();
    m_input.reset();
    m_audio.reset();
    m_renderer.reset();

    SDL_DestroyWindow(m_window);
    m_window = nullptr;

    Mix_CloseAudio();
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();

    std::cout << "Engine shutdown." << std::endl;
}

void Engine::setTitle(const std::string& title) {
    m_config.title = title;
    if (m_window) SDL_SetWindowTitle(m_window, title.c_str());
}

bool Engine::loadScript(const std::string& path) {
    return m_script->loadFile(path);
}

void Engine::startScript(const std::string& funcName) {
    m_script->createCoroutine(funcName);
    m_script->resumeCoroutine(0);
}

bool Engine::saveScreenshot(const std::string& path) {
    SDL_Rect rect = {0, 0, m_config.width, m_config.height};
    SDL_Surface* surface = SDL_CreateRGBSurface(0, rect.w, rect.h, 32,
        0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
    if (!surface) return false;
    SDL_RenderReadPixels(m_renderer->getSDLRenderer(), &rect,
        SDL_PIXELFORMAT_ARGB8888, surface->pixels, surface->pitch);
    bool ok = SDL_SaveBMP(surface, path.c_str()) == 0;
    SDL_FreeSurface(surface);
    return ok;
}
