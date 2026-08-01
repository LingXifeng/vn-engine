#include "lua_bindings.h"
#include <iostream>

// 全局指针键
static const char* K_SCENE    = "_scene_ptr";
static const char* K_AUDIO    = "_audio_ptr";
static const char* K_RESMGR   = "_resmgr_ptr";
static const char* K_TWEEN    = "_tween_ptr";
static const char* K_ENGINE   = "_engine_ptr";
static const char* K_SCRIPT   = "_script_ptr";
static const char* K_PARTICLE = "_particle_ptr";
static const char* K_GALLERY  = "_gallery_ptr";
static const char* K_RICHTEXT = "_richtext_ptr";
static const char* K_MUSICROOM = "_musicroom_ptr";
static const char* K_CREDITS  = "_credits_ptr";
static const char* K_FLOWCHART = "_flowchart_ptr";
static const char* K_SAVELOADUI = "_saveloadui_ptr";
static const char* K_BACKLOG    = "_backlog_ptr";
static const char* K_AUTOSKIP   = "_autoskip_ptr";
static const char* K_TRANSITION = "_transition_ptr";
static const char* K_BGANIM     = "_bganim_ptr";
static const char* K_ENDINGLIST = "_endinglist_ptr";
static const char* K_SCENEREPLAY = "_scenereplay_ptr";
static const char* K_GLOSSARY   = "_glossary_ptr";
static const char* K_VIDEO      = "_video_ptr";
static const char* K_CONFIGP    = "_configp_ptr";
static const char* K_WINDOW     = "_window_ptr";
static const char* K_I18N       = "_i18n_ptr";
static const char* K_CHAREXPR   = "_charexpr_ptr";

LuaBindings::LuaBindings(ScriptEngine* script, Engine* engine, Scene* scene,
                         Audio* audio, ResourceManager* resMgr, TweenManager* tweenMgr)
    : m_script(script), m_engine(engine), m_scene(scene),
      m_audio(audio), m_resMgr(resMgr), m_tweenMgr(tweenMgr) {
    // 创建新功能对象
    Renderer* renderer = engine ? engine->getRenderer() : nullptr;
    if (renderer) {
        m_particles = std::make_unique<ParticleSystem>(renderer);
        m_richText  = std::make_unique<RichTextRenderer>(renderer);
        m_credits   = std::make_unique<Credits>(renderer);
        m_flowchart = std::make_unique<Flowchart>(renderer);
    }
    if (renderer && resMgr) {
        m_cgGallery = std::make_unique<CGGallery>(renderer, resMgr);
    }
    if (renderer && audio) {
        m_musicRoom = std::make_unique<MusicRoom>(renderer, audio);
        m_voiceGallery = std::make_unique<VoiceGallery>(renderer, audio);
    }
    // 新功能2: 8个扩展功能
    if (renderer) {
        m_transition = std::make_unique<Transition>(renderer);
        m_bgAnim = std::make_unique<BackgroundAnimation>(renderer);
        m_endingList = std::make_unique<EndingList>(renderer);
        m_sceneReplay = std::make_unique<SceneReplay>(renderer);
        m_glossary = std::make_unique<Glossary>(renderer);
    }
    if (renderer && audio) {
        m_backlog = std::make_unique<Backlog>(renderer, audio);
    }
    m_autoSkip = std::make_unique<AutoSkip>();
    if (renderer && m_engine && m_engine->getSaveLoad()) {
        m_saveLoadUI = std::make_unique<SaveLoadUI>(renderer, m_engine->getSaveLoad());
    }

    // 新功能3: 5个扩展功能
    if (renderer) {
        m_videoPlayer = std::make_unique<VideoPlayer>(renderer);
        m_charExpr = std::make_unique<CharacterExpression>(renderer);
    }
    m_configPersist = std::make_unique<ConfigPersistence>();
    m_localization = std::make_unique<Localization>();
    // WindowResize 需要 SDL_Window，从 engine 获取
    // 但 engine.h 没有暴露 SDL_Window，我们在 registerAll 中延迟初始化
}

LuaBindings::~LuaBindings() {}

void LuaBindings::updateUI(float dt, const Input& input) {
    if (m_particles)   m_particles->update(dt);
    if (m_cgGallery)   m_cgGallery->update(dt, input);
    if (m_musicRoom)   m_musicRoom->update(dt, input);
    if (m_voiceGallery) m_voiceGallery->update(dt, input);
    if (m_credits)     m_credits->update(dt, input);
    if (m_flowchart)   m_flowchart->update(dt, input);
    // 新功能2
    if (m_saveLoadUI)  m_saveLoadUI->update(dt, input);
    if (m_backlog)     m_backlog->update(dt, input);
    if (m_autoSkip)    m_autoSkip->update(dt);
    if (m_transition)  m_transition->update(dt);
    if (m_bgAnim)      m_bgAnim->update(dt);
    if (m_endingList)  m_endingList->update(dt, input);
    if (m_sceneReplay) m_sceneReplay->update(dt, input);
    if (m_glossary)    m_glossary->update(dt, input);
    // 新功能3
    if (m_videoPlayer) m_videoPlayer->update(dt);
    if (m_charExpr)    m_charExpr->update(dt);
    if (m_windowResize) m_windowResize->update();
}

void LuaBindings::renderUI() {
    // 粒子特效在场景之上渲染
    if (m_particles)   m_particles->render();
    // 背景动画
    if (m_bgAnim)      m_bgAnim->render();
    // UI 界面叠加渲染
    if (m_cgGallery && m_cgGallery->isVisible())   m_cgGallery->render();
    if (m_musicRoom && m_musicRoom->isVisible())   m_musicRoom->render();
    if (m_voiceGallery && m_voiceGallery->isVisible()) m_voiceGallery->render();
    if (m_credits && m_credits->isVisible())       m_credits->render();
    if (m_flowchart && m_flowchart->isVisible())   m_flowchart->render();
    // 新功能2 UI
    if (m_saveLoadUI && m_saveLoadUI->isVisible()) m_saveLoadUI->render();
    if (m_backlog && m_backlog->isVisible())       m_backlog->render();
    if (m_endingList && m_endingList->isVisible()) m_endingList->render();
    if (m_sceneReplay && m_sceneReplay->isVisible()) m_sceneReplay->render();
    if (m_glossary && m_glossary->isVisible())     m_glossary->render();
    // 转场特效在最上层
    if (m_transition && m_transition->isActive())  m_transition->render();
    // 新功能3: 视频播放和角色表情差分
    if (m_engine && m_engine->getRenderer()) {
        if (m_videoPlayer && m_videoPlayer->isPlaying()) m_videoPlayer->render(m_engine->getRenderer());
        if (m_charExpr)    m_charExpr->render(m_engine->getRenderer());
    }
}

void LuaBindings::storeEnginePtr(lua_State* L, void* ptr, const char* key) {
    lua_pushlightuserdata(L, ptr);
    lua_setfield(L, LUA_REGISTRYINDEX, key);
}

void* LuaBindings::getEnginePtr(lua_State* L, const char* key) {
    lua_getfield(L, LUA_REGISTRYINDEX, key);
    void* ptr = lua_touserdata(L, -1);
    lua_pop(L, 1);
    return ptr;
}

void LuaBindings::registerAll() {
    lua_State* L = m_script->getState();

    // 存储指针到 registry
    storeEnginePtr(L, m_scene, K_SCENE);
    storeEnginePtr(L, m_audio, K_AUDIO);
    storeEnginePtr(L, m_resMgr, K_RESMGR);
    storeEnginePtr(L, m_tweenMgr, K_TWEEN);
    storeEnginePtr(L, m_engine, K_ENGINE);
    storeEnginePtr(L, m_script, K_SCRIPT);
    storeEnginePtr(L, m_particles.get(), K_PARTICLE);
    storeEnginePtr(L, m_cgGallery.get(), K_GALLERY);
    storeEnginePtr(L, m_richText.get(), K_RICHTEXT);
    storeEnginePtr(L, m_musicRoom.get(), K_MUSICROOM);
    storeEnginePtr(L, m_credits.get(), K_CREDITS);
    storeEnginePtr(L, m_flowchart.get(), K_FLOWCHART);

    registerSceneAPI();
    registerCharacterAPI();
    registerTextAPI();
    registerAudioAPI();
    registerTweenAPI();
    registerInputAPI();
    registerSystemAPI();
    registerFlowAPI();
    registerParticleAPI();
    registerGalleryAPI();
    registerRichTextAPI();
    registerMusicRoomAPI();
    registerCreditsAPI();
    registerFlowchartAPI();

    // 新功能2: 8个扩展功能
    if (m_saveLoadUI) {
        lua_pushlightuserdata(L, m_saveLoadUI.get());
        lua_setfield(L, LUA_REGISTRYINDEX, K_SAVELOADUI);
        registerSaveLoadUIAPI();
    }
    if (m_backlog) {
        lua_pushlightuserdata(L, m_backlog.get());
        lua_setfield(L, LUA_REGISTRYINDEX, K_BACKLOG);
        registerBacklogAPI();
    }
    if (m_autoSkip) {
        lua_pushlightuserdata(L, m_autoSkip.get());
        lua_setfield(L, LUA_REGISTRYINDEX, K_AUTOSKIP);
        registerAutoSkipAPI();
    }
    if (m_transition) {
        lua_pushlightuserdata(L, m_transition.get());
        lua_setfield(L, LUA_REGISTRYINDEX, K_TRANSITION);
        registerTransitionAPI();
    }
    if (m_bgAnim) {
        lua_pushlightuserdata(L, m_bgAnim.get());
        lua_setfield(L, LUA_REGISTRYINDEX, K_BGANIM);
        registerBgAnimAPI();
    }
    if (m_endingList) {
        lua_pushlightuserdata(L, m_endingList.get());
        lua_setfield(L, LUA_REGISTRYINDEX, K_ENDINGLIST);
        registerEndingListAPI();
    }
    if (m_sceneReplay) {
        lua_pushlightuserdata(L, m_sceneReplay.get());
        lua_setfield(L, LUA_REGISTRYINDEX, K_SCENEREPLAY);
        registerSceneReplayAPI();
    }
    if (m_glossary) {
        lua_pushlightuserdata(L, m_glossary.get());
        lua_setfield(L, LUA_REGISTRYINDEX, K_GLOSSARY);
        registerGlossaryAPI();
    }

    // 新功能3: 5个扩展功能
    if (m_videoPlayer) {
        lua_pushlightuserdata(L, m_videoPlayer.get());
        lua_setfield(L, LUA_REGISTRYINDEX, K_VIDEO);
        registerVideoAPI();
    }
    if (m_configPersist) {
        lua_pushlightuserdata(L, m_configPersist.get());
        lua_setfield(L, LUA_REGISTRYINDEX, K_CONFIGP);
        registerConfigPersistAPI();
    }
    if (m_localization) {
        lua_pushlightuserdata(L, m_localization.get());
        lua_setfield(L, LUA_REGISTRYINDEX, K_I18N);
        registerLocalizationAPI();
    }
    if (m_charExpr) {
        lua_pushlightuserdata(L, m_charExpr.get());
        lua_setfield(L, LUA_REGISTRYINDEX, K_CHAREXPR);
        registerCharExprAPI();
    }
    // WindowResize 需要 SDL_Window，从 engine 获取
    if (m_engine && m_engine->getWindow()) {
        m_windowResize = std::make_unique<WindowResize>(m_engine->getWindow());
        lua_pushlightuserdata(L, m_windowResize.get());
        lua_setfield(L, LUA_REGISTRYINDEX, K_WINDOW);
        registerWindowAPI();
    }

    // 资源包 API
    registerPackageAPI();
}

// === 场景 API ===

void LuaBindings::registerSceneAPI() {
    lua_State* L = m_script->getState();
    lua_newtable(L);

    lua_pushcfunction(L, l_setBackground);
    lua_setfield(L, -2, "setBackground");

    lua_pushcfunction(L, l_clearBackground);
    lua_setfield(L, -2, "clearBackground");

    lua_pushcfunction(L, l_fadeIn);
    lua_setfield(L, -2, "fadeIn");

    lua_pushcfunction(L, l_fadeOut);
    lua_setfield(L, -2, "fadeOut");

    lua_setglobal(L, "Scene");
}

int LuaBindings::l_setBackground(lua_State* L) {
    const char* path = luaL_checkstring(L, 1);
    float fade = luaL_optnumber(L, 2, 0.0);
    Scene* scene = (Scene*)getEnginePtr(L, K_SCENE);
    if (scene) scene->setBackground(path, fade);
    return 0;
}

int LuaBindings::l_clearBackground(lua_State* L) {
    Scene* scene = (Scene*)getEnginePtr(L, K_SCENE);
    if (scene) scene->clearBackground();
    return 0;
}

int LuaBindings::l_fadeIn(lua_State* L) {
    float duration = luaL_optnumber(L, 1, 0.5);
    // 简化：使用 tween 管理器实现全屏淡入
    TweenManager* tween = (TweenManager*)getEnginePtr(L, K_TWEEN);
    // 实际实现需要引擎支持全屏遮罩
    return 0;
}

int LuaBindings::l_fadeOut(lua_State* L) {
    float duration = luaL_optnumber(L, 1, 0.5);
    return 0;
}

// === 角色 API ===

void LuaBindings::registerCharacterAPI() {
    lua_State* L = m_script->getState();
    lua_newtable(L);

    lua_pushcfunction(L, l_showCharacter);
    lua_setfield(L, -2, "show");

    lua_pushcfunction(L, l_hideCharacter);
    lua_setfield(L, -2, "hide");

    lua_pushcfunction(L, l_setExpression);
    lua_setfield(L, -2, "setExpression");

    lua_pushcfunction(L, l_moveCharacter);
    lua_setfield(L, -2, "move");

    lua_setglobal(L, "Character");
}

int LuaBindings::l_showCharacter(lua_State* L) {
    const char* name = luaL_checkstring(L, 1);
    const char* expression = luaL_optstring(L, 2, "default");
    const char* posStr = luaL_optstring(L, 3, "left");
    float fade = luaL_optnumber(L, 4, 0.3);

    CharPosition pos = CharPosition::LEFT;
    if (std::string(posStr) == "center") pos = CharPosition::CENTER;
    else if (std::string(posStr) == "right") pos = CharPosition::RIGHT;

    Scene* scene = (Scene*)getEnginePtr(L, K_SCENE);
    if (scene) scene->showCharacter(name, expression, pos, fade);
    return 0;
}

int LuaBindings::l_hideCharacter(lua_State* L) {
    const char* name = luaL_checkstring(L, 1);
    float fade = luaL_optnumber(L, 2, 0.3);
    Scene* scene = (Scene*)getEnginePtr(L, K_SCENE);
    if (scene) scene->hideCharacter(name, fade);
    return 0;
}

int LuaBindings::l_setExpression(lua_State* L) {
    const char* name = luaL_checkstring(L, 1);
    const char* expression = luaL_checkstring(L, 2);
    Scene* scene = (Scene*)getEnginePtr(L, K_SCENE);
    if (scene) {
        Character* ch = scene->getCharacter(name);
        if (ch) ch->setExpression(expression);
    }
    return 0;
}

int LuaBindings::l_moveCharacter(lua_State* L) {
    // TODO: 角色移动动画
    return 0;
}

// === 文字 API ===

void LuaBindings::registerTextAPI() {
    lua_State* L = m_script->getState();
    lua_newtable(L);

    lua_pushcfunction(L, l_say);
    lua_setfield(L, -2, "say");

    lua_pushcfunction(L, l_narrate);
    lua_setfield(L, -2, "narrate");

    lua_pushcfunction(L, l_clearText);
    lua_setfield(L, -2, "clear");

    lua_pushcfunction(L, l_setTextColor);
    lua_setfield(L, -2, "setColor");

    lua_pushcfunction(L, l_setTextSpeed);
    lua_setfield(L, -2, "setSpeed");

    lua_pushcfunction(L, l_choice);
    lua_setfield(L, -2, "choice");

    lua_setglobal(L, "Text");
}

int LuaBindings::l_say(lua_State* L) {
    const char* name = luaL_checkstring(L, 1);
    const char* text = luaL_checkstring(L, 2);
    Scene* scene = (Scene*)getEnginePtr(L, K_SCENE);
    if (scene) scene->showDialogue(name, text);
    // yield 等待玩家点击
    return lua_yield(L, 0);
}

int LuaBindings::l_narrate(lua_State* L) {
    const char* text = luaL_checkstring(L, 1);
    Scene* scene = (Scene*)getEnginePtr(L, K_SCENE);
    if (scene) scene->showDialogue("", text);
    return lua_yield(L, 0);
}

int LuaBindings::l_clearText(lua_State* L) {
    Scene* scene = (Scene*)getEnginePtr(L, K_SCENE);
    if (scene) scene->clearDialogue();
    return 0;
}

int LuaBindings::l_setTextColor(lua_State* L) {
    int r = luaL_checkinteger(L, 1);
    int g = luaL_checkinteger(L, 2);
    int b = luaL_checkinteger(L, 3);
    Scene* scene = (Scene*)getEnginePtr(L, K_SCENE);
    if (scene) {
        scene->getTextBox()->setTextColor({(Uint8)r, (Uint8)g, (Uint8)b, 255});
    }
    return 0;
}

int LuaBindings::l_setTextSpeed(lua_State* L) {
    float speed = luaL_checknumber(L, 1);
    Scene* scene = (Scene*)getEnginePtr(L, K_SCENE);
    if (scene) scene->getTextBox()->setTypingSpeed(speed);
    return 0;
}

int LuaBindings::l_choice(lua_State* L) {
    // Text.choice({"选项1", "选项2", "选项3"}) 返回选中的索引
    luaL_checktype(L, 1, LUA_TTABLE);
    int n = luaL_len(L, 1);

    Scene* scene = (Scene*)getEnginePtr(L, K_SCENE);
    if (!scene) return 0;

    scene->clearChoices();
    for (int i = 1; i <= n; i++) {
        lua_geti(L, 1, i);
        const char* text = lua_tostring(L, -1);
        scene->addChoice(text);
        lua_pop(L, 1);
    }

    // yield 等待选择
    return lua_yield(L, 0);
    // 恢复后需要返回选择的索引
}

// === 音频 API ===

void LuaBindings::registerAudioAPI() {
    lua_State* L = m_script->getState();
    lua_newtable(L);

    lua_pushcfunction(L, l_playBGM);
    lua_setfield(L, -2, "playBGM");

    lua_pushcfunction(L, l_stopBGM);
    lua_setfield(L, -2, "stopBGM");

    lua_pushcfunction(L, l_playSE);
    lua_setfield(L, -2, "playSE");

    lua_pushcfunction(L, l_playVoice);
    lua_setfield(L, -2, "playVoice");

    lua_pushcfunction(L, l_setVolume);
    lua_setfield(L, -2, "setVolume");

    lua_setglobal(L, "Audio");
}

int LuaBindings::l_playBGM(lua_State* L) {
    const char* path = luaL_checkstring(L, 1);
    bool loop = lua_isboolean(L, 2) ? lua_toboolean(L, 2) : true;
    int fade = luaL_optinteger(L, 3, 0);
    Audio* audio = (Audio*)getEnginePtr(L, K_AUDIO);
    if (audio) audio->playBGM(path, loop, fade);
    return 0;
}

int LuaBindings::l_stopBGM(lua_State* L) {
    int fade = luaL_optinteger(L, 1, 0);
    Audio* audio = (Audio*)getEnginePtr(L, K_AUDIO);
    if (audio) audio->stopBGM(fade);
    return 0;
}

int LuaBindings::l_playSE(lua_State* L) {
    const char* path = luaL_checkstring(L, 1);
    Audio* audio = (Audio*)getEnginePtr(L, K_AUDIO);
    if (audio) audio->playSE(path);
    return 0;
}

int LuaBindings::l_playVoice(lua_State* L) {
    const char* path = luaL_checkstring(L, 1);
    Audio* audio = (Audio*)getEnginePtr(L, K_AUDIO);
    if (audio) audio->playVoice(path);
    return 0;
}

int LuaBindings::l_setVolume(lua_State* L) {
    const char* type = luaL_checkstring(L, 1);
    int vol = luaL_checkinteger(L, 2);
    Audio* audio = (Audio*)getEnginePtr(L, K_AUDIO);
    if (audio) {
        if (std::string(type) == "bgm") audio->setBGMVolume(vol);
        else if (std::string(type) == "se") audio->setSEVolume(vol);
        else if (std::string(type) == "voice") audio->setVoiceVolume(vol);
        else if (std::string(type) == "master") audio->setMasterVolume(vol);
    }
    return 0;
}

// === 动画 API ===

void LuaBindings::registerTweenAPI() {
    lua_State* L = m_script->getState();
    lua_newtable(L);

    lua_pushcfunction(L, l_tween);
    lua_setfield(L, -2, "start");

    lua_pushcfunction(L, l_wait);
    lua_setfield(L, -2, "wait");

    lua_setglobal(L, "Tween");
}

int LuaBindings::l_tween(lua_State* L) {
    // Tween.start(duration, callback) - 简化版
    float duration = luaL_checknumber(L, 1);
    // 完整实现需要支持 Lua 回调函数
    return 0;
}

int LuaBindings::l_wait(lua_State* L) {
    float duration = luaL_checknumber(L, 1);
    // yield 等待指定时间
    // 实际实现需要引擎计时器
    return lua_yield(L, 0);
}

// === 输入 API ===

void LuaBindings::registerInputAPI() {
    // 输入主要由引擎内部处理，Lua 层一般不直接访问
}

// === 系统 API ===

void LuaBindings::registerSystemAPI() {
    lua_State* L = m_script->getState();
    lua_newtable(L);

    lua_pushcfunction(L, l_log);
    lua_setfield(L, -2, "log");

    lua_pushcfunction(L, l_getScreenSize);
    lua_setfield(L, -2, "getScreenSize");

    lua_pushcfunction(L, l_setTitle);
    lua_setfield(L, -2, "setTitle");

    lua_pushcfunction(L, l_quit);
    lua_setfield(L, -2, "quit");

    lua_pushcfunction(L, l_save);
    lua_setfield(L, -2, "save");

    lua_pushcfunction(L, l_load);
    lua_setfield(L, -2, "load");

    lua_setglobal(L, "System");
}

int LuaBindings::l_log(lua_State* L) {
    const char* msg = luaL_checkstring(L, 1);
    std::cout << "[Lua] " << msg << std::endl;
    return 0;
}

int LuaBindings::l_getScreenSize(lua_State* L) {
    Engine* engine = (Engine*)getEnginePtr(L, K_ENGINE);
    if (engine) {
        lua_pushinteger(L, engine->getWidth());
        lua_pushinteger(L, engine->getHeight());
        return 2;
    }
    return 0;
}

int LuaBindings::l_setTitle(lua_State* L) {
    const char* title = luaL_checkstring(L, 1);
    Engine* engine = (Engine*)getEnginePtr(L, K_ENGINE);
    if (engine) engine->setTitle(title);
    return 0;
}

int LuaBindings::l_quit(lua_State* L) {
    Engine* engine = (Engine*)getEnginePtr(L, K_ENGINE);
    if (engine) engine->quit();
    return 0;
}

int LuaBindings::l_save(lua_State* L) {
    int slot = luaL_checkinteger(L, 1);
    // TODO: 存档系统
    std::cout << "[Save] slot " << slot << std::endl;
    return 0;
}

int LuaBindings::l_load(lua_State* L) {
    int slot = luaL_checkinteger(L, 1);
    // TODO: 读档系统
    std::cout << "[Load] slot " << slot << std::endl;
    return 0;
}

// === 流程控制 API ===

void LuaBindings::registerFlowAPI() {
    // 流程控制通过 coroutine yield 实现
    // say/choice/wait 等函数内部 yield
}


// === 粒子特效 API ===

void LuaBindings::registerParticleAPI() {
    lua_State* L = m_script->getState();
    lua_newtable(L);

    lua_pushcfunction(L, l_particleCreate);
    lua_setfield(L, -2, "create");

    lua_pushcfunction(L, l_particleSetType);
    lua_setfield(L, -2, "setType");

    lua_pushcfunction(L, l_particleSetParam);
    lua_setfield(L, -2, "setParam");

    lua_pushcfunction(L, l_particleStart);
    lua_setfield(L, -2, "start");

    lua_pushcfunction(L, l_particleStop);
    lua_setfield(L, -2, "stop");

    lua_pushcfunction(L, l_particleClear);
    lua_setfield(L, -2, "clear");

    lua_setglobal(L, "Particle");
}

int LuaBindings::l_particleCreate(lua_State* L) {
    // Particle.create(type, count, speed) - 创建并发射粒子
    const char* typeStr = luaL_checkstring(L, 1);
    int count = luaL_optinteger(L, 2, 100);
    float speed = luaL_optnumber(L, 3, 100.0f);

    ParticleType type = ParticleType::NONE;
    std::string t(typeStr);
    if (t == "rain") type = ParticleType::RAIN;
    else if (t == "snow") type = ParticleType::SNOW;
    else if (t == "confetti") type = ParticleType::CONFETTI;
    else if (t == "fire") type = ParticleType::FIRE;
    else if (t == "sakura") type = ParticleType::SAKURA;
    else if (t == "spark") type = ParticleType::SPARK;

    ParticleSystem* ps = (ParticleSystem*)getEnginePtr(L, K_PARTICLE);
    if (ps) {
        ParticleParams params;
        params.count = count;
        params.speed = speed;
        ps->emit(type, params);
    }
    return 0;
}

int LuaBindings::l_particleSetType(lua_State* L) {
    // Particle.setType(type) - 切换当前特效类型
    const char* typeStr = luaL_checkstring(L, 1);
    ParticleSystem* ps = (ParticleSystem*)getEnginePtr(L, K_PARTICLE);
    if (ps) {
        ps->stop();
        l_particleCreate(L);
    }
    return 0;
}

int LuaBindings::l_particleSetParam(lua_State* L) {
    // Particle.setParam(key, value) - 设置参数
    // 简化版：直接重新发射
    return 0;
}

int LuaBindings::l_particleStart(lua_State* L) {
    ParticleSystem* ps = (ParticleSystem*)getEnginePtr(L, K_PARTICLE);
    // 粒子系统在 emit 时自动启动
    return 0;
}

int LuaBindings::l_particleStop(lua_State* L) {
    ParticleSystem* ps = (ParticleSystem*)getEnginePtr(L, K_PARTICLE);
    if (ps) ps->stop();
    return 0;
}

int LuaBindings::l_particleClear(lua_State* L) {
    ParticleSystem* ps = (ParticleSystem*)getEnginePtr(L, K_PARTICLE);
    if (ps) ps->stop();
    return 0;
}

// === CG 画廊 API ===

void LuaBindings::registerGalleryAPI() {
    lua_State* L = m_script->getState();
    lua_newtable(L);

    lua_pushcfunction(L, l_galleryUnlock);
    lua_setfield(L, -2, "unlock");

    lua_pushcfunction(L, l_galleryIsUnlocked);
    lua_setfield(L, -2, "isUnlocked");

    lua_pushcfunction(L, l_galleryShow);
    lua_setfield(L, -2, "show");

    lua_pushcfunction(L, l_galleryLoadData);
    lua_setfield(L, -2, "loadData");

    lua_pushcfunction(L, l_gallerySaveData);
    lua_setfield(L, -2, "saveData");

    lua_setglobal(L, "Gallery");
}

int LuaBindings::l_galleryUnlock(lua_State* L) {
    const char* id = luaL_checkstring(L, 1);
    CGGallery* gallery = (CGGallery*)getEnginePtr(L, K_GALLERY);
    if (gallery) gallery->unlockCG(id);
    return 0;
}

int LuaBindings::l_galleryIsUnlocked(lua_State* L) {
    const char* id = luaL_checkstring(L, 1);
    CGGallery* gallery = (CGGallery*)getEnginePtr(L, K_GALLERY);
    lua_pushboolean(L, gallery ? gallery->isCGUnlocked(id) : false);
    return 1;
}

int LuaBindings::l_galleryShow(lua_State* L) {
    CGGallery* gallery = (CGGallery*)getEnginePtr(L, K_GALLERY);
    if (gallery) gallery->show();
    return 0;
}

int LuaBindings::l_galleryLoadData(lua_State* L) {
    // Gallery.loadData({id1, id2, ...}) - 从存档恢复解锁状态
    luaL_checktype(L, 1, LUA_TTABLE);
    int n = luaL_len(L, 1);
    CGGallery* gallery = (CGGallery*)getEnginePtr(L, K_GALLERY);
    if (gallery) {
        std::vector<std::string> ids;
        for (int i = 1; i <= n; i++) {
            lua_geti(L, 1, i);
            ids.push_back(lua_tostring(L, -1));
            lua_pop(L, 1);
        }
        gallery->setUnlockedIDs(ids);
    }
    return 0;
}

int LuaBindings::l_gallerySaveData(lua_State* L) {
    // 返回已解锁 ID 列表
    CGGallery* gallery = (CGGallery*)getEnginePtr(L, K_GALLERY);
    if (gallery) {
        auto ids = gallery->getUnlockedIDs();
        lua_newtable(L);
        for (size_t i = 0; i < ids.size(); i++) {
            lua_pushstring(L, ids[i].c_str());
            lua_seti(L, -2, i + 1);
        }
        return 1;
    }
    lua_newtable(L);
    return 1;
}

// === 富文本 API ===

void LuaBindings::registerRichTextAPI() {
    lua_State* L = m_script->getState();
    lua_newtable(L);

    lua_pushcfunction(L, l_richTextSetRuby);
    lua_setfield(L, -2, "setRuby");

    lua_pushcfunction(L, l_richTextSetBold);
    lua_setfield(L, -2, "setBold");

    lua_pushcfunction(L, l_richTextSetShadow);
    lua_setfield(L, -2, "setShadow");

    lua_pushcfunction(L, l_richTextSetSpacing);
    lua_setfield(L, -2, "setSpacing");

    lua_pushcfunction(L, l_richTextRender);
    lua_setfield(L, -2, "render");

    lua_setglobal(L, "RichText");
}

int LuaBindings::l_richTextSetRuby(lua_State* L) {
    // RichText.setRuby(base, ruby) - 注音通过标记文本实现
    // 此函数提示用户使用 {ruby:注音}文字{/ruby} 标记
    return 0;
}

int LuaBindings::l_richTextSetBold(lua_State* L) {
    // RichText.setBold(true/false) - 粗体通过 {b}...{/b} 标记实现
    return 0;
}

int LuaBindings::l_richTextSetShadow(lua_State* L) {
    // RichText.setShadow(r, g, b, offset) - 设置阴影颜色
    int r = luaL_optinteger(L, 1, 0);
    int g = luaL_optinteger(L, 2, 0);
    int b = luaL_optinteger(L, 3, 0);
    int offset = luaL_optinteger(L, 4, 2);
    // 通过全局 config 设置（需要存储 config）
    // 简化：标记中用 {shadow}...{/shadow}
    return 0;
}

int LuaBindings::l_richTextSetSpacing(lua_State* L) {
    // RichText.setSpacing(lineSpacing, charSpacing)
    float lineSpacing = luaL_optnumber(L, 1, 4.0f);
    int charSpacing = luaL_optinteger(L, 2, 0);
    // 存储到全局配置（通过 registry）
    lua_State* L2 = L;
    lua_pushnumber(L2, lineSpacing);
    lua_setfield(L2, LUA_REGISTRYINDEX, "_rt_lineSpacing");
    lua_pushinteger(L2, charSpacing);
    lua_setfield(L2, LUA_REGISTRYINDEX, "_rt_charSpacing");
    return 0;
}

int LuaBindings::l_richTextRender(lua_State* L) {
    // RichText.render(text, x, y, maxWidth) - 渲染富文本
    const char* text = luaL_checkstring(L, 1);
    int x = luaL_optinteger(L, 2, 0);
    int y = luaL_optinteger(L, 3, 0);
    int maxW = luaL_optinteger(L, 4, 0);

    RichTextRenderer* rt = (RichTextRenderer*)getEnginePtr(L, K_RICHTEXT);
    if (rt) {
        RichTextConfig config;
        // 从 registry 读取自定义间距
        lua_getfield(L, LUA_REGISTRYINDEX, "_rt_lineSpacing");
        if (lua_isnumber(L, -1)) config.lineSpacing = (int)lua_tonumber(L, -1);
        lua_pop(L, 1);
        lua_getfield(L, LUA_REGISTRYINDEX, "_rt_charSpacing");
        if (lua_isinteger(L, -1)) config.charSpacing = (int)lua_tointeger(L, -1);
        lua_pop(L, 1);

        rt->renderText(text, x, y, maxW, config);
    }
    return 0;
}

// === 音乐室 API ===

void LuaBindings::registerMusicRoomAPI() {
    lua_State* L = m_script->getState();
    lua_newtable(L);

    lua_pushcfunction(L, l_musicRoomAdd);
    lua_setfield(L, -2, "add");

    lua_pushcfunction(L, l_musicRoomShow);
    lua_setfield(L, -2, "show");

    lua_pushcfunction(L, l_musicRoomPlay);
    lua_setfield(L, -2, "play");

    lua_pushcfunction(L, l_musicRoomStop);
    lua_setfield(L, -2, "stop");

    lua_setglobal(L, "MusicRoom");
}

int LuaBindings::l_musicRoomAdd(lua_State* L) {
    // MusicRoom.add(id, title, artist, path) - 添加 BGM 到音乐室
    const char* id = luaL_checkstring(L, 1);
    const char* title = luaL_checkstring(L, 2);
    const char* artist = luaL_optstring(L, 3, "");
    const char* path = luaL_checkstring(L, 4);
    MusicRoom* mr = (MusicRoom*)getEnginePtr(L, K_MUSICROOM);
    if (mr) mr->addBGM(id, title, artist, path);
    return 0;
}

int LuaBindings::l_musicRoomShow(lua_State* L) {
    MusicRoom* mr = (MusicRoom*)getEnginePtr(L, K_MUSICROOM);
    if (mr) mr->show();
    return 0;
}

int LuaBindings::l_musicRoomPlay(lua_State* L) {
    const char* path = luaL_checkstring(L, 1);
    Audio* audio = (Audio*)getEnginePtr(L, K_AUDIO);
    if (audio) audio->playBGM(path, true);
    return 0;
}

int LuaBindings::l_musicRoomStop(lua_State* L) {
    Audio* audio = (Audio*)getEnginePtr(L, K_AUDIO);
    if (audio) audio->stopBGM();
    return 0;
}

// === Credits API ===

void LuaBindings::registerCreditsAPI() {
    lua_State* L = m_script->getState();
    lua_newtable(L);

    lua_pushcfunction(L, l_creditsAddTitle);
    lua_setfield(L, -2, "addTitle");

    lua_pushcfunction(L, l_creditsAddHeading);
    lua_setfield(L, -2, "addHeading");

    lua_pushcfunction(L, l_creditsAddName);
    lua_setfield(L, -2, "addName");

    lua_pushcfunction(L, l_creditsAddSmall);
    lua_setfield(L, -2, "addSmall");

    lua_pushcfunction(L, l_creditsLoadFile);
    lua_setfield(L, -2, "loadFile");

    lua_pushcfunction(L, l_creditsShow);
    lua_setfield(L, -2, "show");

    lua_pushcfunction(L, l_creditsHide);
    lua_setfield(L, -2, "hide");

    lua_pushcfunction(L, l_creditsSetSpeed);
    lua_setfield(L, -2, "setSpeed");

    lua_setglobal(L, "Credits");
}

int LuaBindings::l_creditsAddTitle(lua_State* L) {
    const char* text = luaL_checkstring(L, 1);
    Credits* credits = (Credits*)getEnginePtr(L, K_CREDITS);
    if (credits) credits->addTitle(text);
    return 0;
}

int LuaBindings::l_creditsAddHeading(lua_State* L) {
    const char* text = luaL_checkstring(L, 1);
    Credits* credits = (Credits*)getEnginePtr(L, K_CREDITS);
    if (credits) credits->addHeading(text);
    return 0;
}

int LuaBindings::l_creditsAddName(lua_State* L) {
    const char* text = luaL_checkstring(L, 1);
    Credits* credits = (Credits*)getEnginePtr(L, K_CREDITS);
    if (credits) credits->addName(text);
    return 0;
}

int LuaBindings::l_creditsAddSmall(lua_State* L) {
    const char* text = luaL_checkstring(L, 1);
    Credits* credits = (Credits*)getEnginePtr(L, K_CREDITS);
    if (credits) credits->addSmall(text);
    return 0;
}

int LuaBindings::l_creditsLoadFile(lua_State* L) {
    const char* path = luaL_checkstring(L, 1);
    Credits* credits = (Credits*)getEnginePtr(L, K_CREDITS);
    if (credits) {
        lua_pushboolean(L, credits->loadFromFile(path));
        return 1;
    }
    lua_pushboolean(L, false);
    return 1;
}

int LuaBindings::l_creditsShow(lua_State* L) {
    Credits* credits = (Credits*)getEnginePtr(L, K_CREDITS);
    if (credits) credits->show();
    return 0;
}

int LuaBindings::l_creditsHide(lua_State* L) {
    Credits* credits = (Credits*)getEnginePtr(L, K_CREDITS);
    if (credits) credits->hide();
    return 0;
}

int LuaBindings::l_creditsSetSpeed(lua_State* L) {
    float speed = luaL_checknumber(L, 1);
    Credits* credits = (Credits*)getEnginePtr(L, K_CREDITS);
    if (credits) credits->setScrollSpeed(speed);
    return 0;
}

// === 流程图 API ===

void LuaBindings::registerFlowchartAPI() {
    lua_State* L = m_script->getState();

    lua_newtable(L);

    lua_pushcfunction(L, l_fcAddNode);     lua_setfield(L, -2, "addNode");
    lua_pushcfunction(L, l_fcAddEdge);     lua_setfield(L, -2, "addEdge");
    lua_pushcfunction(L, l_fcSetPosition); lua_setfield(L, -2, "setPosition");
    lua_pushcfunction(L, l_fcMarkVisited); lua_setfield(L, -2, "markVisited");
    lua_pushcfunction(L, l_fcMarkCurrent); lua_setfield(L, -2, "markCurrent");
    lua_pushcfunction(L, l_fcAutoLayout);  lua_setfield(L, -2, "autoLayout");
    lua_pushcfunction(L, l_fcShow);        lua_setfield(L, -2, "show");
    lua_pushcfunction(L, l_fcHide);        lua_setfield(L, -2, "hide");
    lua_pushcfunction(L, l_fcResetView);   lua_setfield(L, -2, "resetView");
    lua_pushcfunction(L, l_fcGetVisited);  lua_setfield(L, -2, "getVisited");
    lua_pushcfunction(L, l_fcGetEndings);  lua_setfield(L, -2, "getEndings");

    lua_setglobal(L, "flowchart");
}

int LuaBindings::l_fcAddNode(lua_State* L) {
    // flowchart.addNode(id, label, type, scriptLabel?, description?)
    const char* id = luaL_checkstring(L, 1);
    const char* label = luaL_checkstring(L, 2);
    const char* typeStr = luaL_checkstring(L, 3);
    const char* scriptLabel = luaL_optstring(L, 4, "");
    const char* desc = luaL_optstring(L, 5, "");

    Flowchart* fc = (Flowchart*)getEnginePtr(L, K_FLOWCHART);
    if (!fc) return 0;

    FlowchartNodeType type = FlowchartNodeType::SCENE;
    std::string t(typeStr);
    if (t == "start")         type = FlowchartNodeType::START;
    else if (t == "chapter")  type = FlowchartNodeType::CHAPTER;
    else if (t == "branch")   type = FlowchartNodeType::BRANCH;
    else if (t == "scene")    type = FlowchartNodeType::SCENE;
    else if (t == "good")     type = FlowchartNodeType::ENDING_GOOD;
    else if (t == "normal")   type = FlowchartNodeType::ENDING_NORMAL;
    else if (t == "bad")      type = FlowchartNodeType::ENDING_BAD;
    else if (t == "true")     type = FlowchartNodeType::ENDING_TRUE;

    fc->addNode(id, label, type, scriptLabel, desc);
    return 0;
}

int LuaBindings::l_fcAddEdge(lua_State* L) {
    // flowchart.addEdge(fromId, toId, label?)
    const char* fromId = luaL_checkstring(L, 1);
    const char* toId = luaL_checkstring(L, 2);
    const char* label = luaL_optstring(L, 3, "");

    Flowchart* fc = (Flowchart*)getEnginePtr(L, K_FLOWCHART);
    if (fc) fc->addEdge(fromId, toId, label);
    return 0;
}

int LuaBindings::l_fcSetPosition(lua_State* L) {
    // flowchart.setPosition(id, x, y)
    const char* id = luaL_checkstring(L, 1);
    float x = luaL_checknumber(L, 2);
    float y = luaL_checknumber(L, 3);

    Flowchart* fc = (Flowchart*)getEnginePtr(L, K_FLOWCHART);
    if (fc) fc->setNodePosition(id, x, y);
    return 0;
}

int LuaBindings::l_fcMarkVisited(lua_State* L) {
    // flowchart.markVisited(id)
    const char* id = luaL_checkstring(L, 1);
    Flowchart* fc = (Flowchart*)getEnginePtr(L, K_FLOWCHART);
    if (fc) fc->markVisited(id);
    return 0;
}

int LuaBindings::l_fcMarkCurrent(lua_State* L) {
    // flowchart.markCurrent(id)
    const char* id = luaL_checkstring(L, 1);
    Flowchart* fc = (Flowchart*)getEnginePtr(L, K_FLOWCHART);
    if (fc) fc->markCurrent(id);
    return 0;
}

int LuaBindings::l_fcAutoLayout(lua_State* L) {
    Flowchart* fc = (Flowchart*)getEnginePtr(L, K_FLOWCHART);
    if (fc) fc->autoLayout();
    return 0;
}

int LuaBindings::l_fcShow(lua_State* L) {
    Flowchart* fc = (Flowchart*)getEnginePtr(L, K_FLOWCHART);
    if (fc) fc->show();
    return 0;
}

int LuaBindings::l_fcHide(lua_State* L) {
    Flowchart* fc = (Flowchart*)getEnginePtr(L, K_FLOWCHART);
    if (fc) fc->hide();
    return 0;
}

int LuaBindings::l_fcResetView(lua_State* L) {
    Flowchart* fc = (Flowchart*)getEnginePtr(L, K_FLOWCHART);
    if (fc) fc->resetView();
    return 0;
}

int LuaBindings::l_fcGetVisited(lua_State* L) {
    Flowchart* fc = (Flowchart*)getEnginePtr(L, K_FLOWCHART);
    if (!fc) {
        lua_newtable(L);
        return 1;
    }
    auto visited = fc->getVisitedNodes();
    lua_newtable(L);
    for (size_t i = 0; i < visited.size(); ++i) {
        lua_pushstring(L, visited[i].c_str());
        lua_rawseti(L, -2, static_cast<int>(i + 1));
    }
    return 1;
}

int LuaBindings::l_fcGetEndings(lua_State* L) {
    Flowchart* fc = (Flowchart*)getEnginePtr(L, K_FLOWCHART);
    if (!fc) {
        lua_newtable(L);
        return 1;
    }
    auto endings = fc->getReachedEndings();
    lua_newtable(L);
    for (size_t i = 0; i < endings.size(); ++i) {
        lua_pushstring(L, endings[i].c_str());
        lua_rawseti(L, -2, static_cast<int>(i + 1));
    }
    return 1;
}


// === 存档/读档 UI API ===

void LuaBindings::registerSaveLoadUIAPI() {
    lua_State* L = m_script->getState();
    lua_newtable(L);

    lua_pushcfunction(L, l_slShowSave);
    lua_setfield(L, -2, "showSave");

    lua_pushcfunction(L, l_slShowLoad);
    lua_setfield(L, -2, "showLoad");

    lua_pushcfunction(L, l_slHide);
    lua_setfield(L, -2, "hide");

    lua_pushcfunction(L, l_slSetSlotCount);
    lua_setfield(L, -2, "setSlotCount");

    lua_setglobal(L, "SaveLoad");
}

int LuaBindings::l_slShowSave(lua_State* L) {
    lua_getfield(L, LUA_REGISTRYINDEX, K_SAVELOADUI);
    auto* ui = static_cast<SaveLoadUI*>(lua_touserdata(L, -1));
    lua_pop(L, 1);
    if (ui) ui->showSave();
    return 0;
}

int LuaBindings::l_slShowLoad(lua_State* L) {
    lua_getfield(L, LUA_REGISTRYINDEX, K_SAVELOADUI);
    auto* ui = static_cast<SaveLoadUI*>(lua_touserdata(L, -1));
    lua_pop(L, 1);
    if (ui) ui->showLoad();
    return 0;
}

int LuaBindings::l_slHide(lua_State* L) {
    lua_getfield(L, LUA_REGISTRYINDEX, K_SAVELOADUI);
    auto* ui = static_cast<SaveLoadUI*>(lua_touserdata(L, -1));
    lua_pop(L, 1);
    if (ui) ui->hide();
    return 0;
}

int LuaBindings::l_slSetSlotCount(lua_State* L) {
    int count = luaL_checkinteger(L, 1);
    lua_getfield(L, LUA_REGISTRYINDEX, K_SAVELOADUI);
    auto* ui = static_cast<SaveLoadUI*>(lua_touserdata(L, -1));
    lua_pop(L, 1);
    if (ui) ui->setSlotCount(count);
    return 0;
}

// === 履历 API ===

void LuaBindings::registerBacklogAPI() {
    lua_State* L = m_script->getState();
    lua_newtable(L);

    lua_pushcfunction(L, l_backlogAdd);
    lua_setfield(L, -2, "add");

    lua_pushcfunction(L, l_backlogShow);
    lua_setfield(L, -2, "show");

    lua_pushcfunction(L, l_backlogHide);
    lua_setfield(L, -2, "hide");

    lua_pushcfunction(L, l_backlogClear);
    lua_setfield(L, -2, "clear");

    lua_setglobal(L, "Backlog");
}

int LuaBindings::l_backlogAdd(lua_State* L) {
    const char* speaker = luaL_optstring(L, 1, "");
    const char* text = luaL_checkstring(L, 2);
    const char* voice = luaL_optstring(L, 3, "");
    lua_getfield(L, LUA_REGISTRYINDEX, K_BACKLOG);
    auto* bl = static_cast<Backlog*>(lua_touserdata(L, -1));
    lua_pop(L, 1);
    if (bl) bl->addEntry(speaker, text, voice);
    return 0;
}

int LuaBindings::l_backlogShow(lua_State* L) {
    lua_getfield(L, LUA_REGISTRYINDEX, K_BACKLOG);
    auto* bl = static_cast<Backlog*>(lua_touserdata(L, -1));
    lua_pop(L, 1);
    if (bl) bl->show();
    return 0;
}

int LuaBindings::l_backlogHide(lua_State* L) {
    lua_getfield(L, LUA_REGISTRYINDEX, K_BACKLOG);
    auto* bl = static_cast<Backlog*>(lua_touserdata(L, -1));
    lua_pop(L, 1);
    if (bl) bl->hide();
    return 0;
}

int LuaBindings::l_backlogClear(lua_State* L) {
    lua_getfield(L, LUA_REGISTRYINDEX, K_BACKLOG);
    auto* bl = static_cast<Backlog*>(lua_touserdata(L, -1));
    lua_pop(L, 1);
    if (bl) bl->clear();
    return 0;
}

// === 自动/跳过 API ===

void LuaBindings::registerAutoSkipAPI() {
    lua_State* L = m_script->getState();
    lua_newtable(L);

    lua_pushcfunction(L, l_asSetAuto);
    lua_setfield(L, -2, "setAuto");

    lua_pushcfunction(L, l_asSetSkip);
    lua_setfield(L, -2, "setSkip");

    lua_pushcfunction(L, l_asToggleAuto);
    lua_setfield(L, -2, "toggleAuto");

    lua_pushcfunction(L, l_asToggleSkip);
    lua_setfield(L, -2, "toggleSkip");

    lua_pushcfunction(L, l_asSetAutoSpeed);
    lua_setfield(L, -2, "setAutoSpeed");

    lua_pushcfunction(L, l_asStopAll);
    lua_setfield(L, -2, "stopAll");

    lua_setglobal(L, "AutoSkip");
}

int LuaBindings::l_asSetAuto(lua_State* L) {
    bool enable = lua_toboolean(L, 1);
    lua_getfield(L, LUA_REGISTRYINDEX, K_AUTOSKIP);
    auto* as = static_cast<AutoSkip*>(lua_touserdata(L, -1));
    lua_pop(L, 1);
    if (as) as->setAutoEnabled(enable);
    return 0;
}

int LuaBindings::l_asSetSkip(lua_State* L) {
    bool enable = lua_toboolean(L, 1);
    lua_getfield(L, LUA_REGISTRYINDEX, K_AUTOSKIP);
    auto* as = static_cast<AutoSkip*>(lua_touserdata(L, -1));
    lua_pop(L, 1);
    if (as) as->setSkipEnabled(enable);
    return 0;
}

int LuaBindings::l_asToggleAuto(lua_State* L) {
    lua_getfield(L, LUA_REGISTRYINDEX, K_AUTOSKIP);
    auto* as = static_cast<AutoSkip*>(lua_touserdata(L, -1));
    lua_pop(L, 1);
    if (as) as->toggleAuto();
    return 0;
}

int LuaBindings::l_asToggleSkip(lua_State* L) {
    lua_getfield(L, LUA_REGISTRYINDEX, K_AUTOSKIP);
    auto* as = static_cast<AutoSkip*>(lua_touserdata(L, -1));
    lua_pop(L, 1);
    if (as) as->toggleSkip();
    return 0;
}

int LuaBindings::l_asSetAutoSpeed(lua_State* L) {
    float speed = luaL_checknumber(L, 1);
    lua_getfield(L, LUA_REGISTRYINDEX, K_AUTOSKIP);
    auto* as = static_cast<AutoSkip*>(lua_touserdata(L, -1));
    lua_pop(L, 1);
    if (as) as->setAutoSpeed(speed);
    return 0;
}

int LuaBindings::l_asStopAll(lua_State* L) {
    lua_getfield(L, LUA_REGISTRYINDEX, K_AUTOSKIP);
    auto* as = static_cast<AutoSkip*>(lua_touserdata(L, -1));
    lua_pop(L, 1);
    if (as) as->stopAll();
    return 0;
}

// === 转场特效 API ===

void LuaBindings::registerTransitionAPI() {
    lua_State* L = m_script->getState();
    lua_newtable(L);

    lua_pushcfunction(L, l_trFade);
    lua_setfield(L, -2, "fade");

    lua_pushcfunction(L, l_trFadeWhite);
    lua_setfield(L, -2, "fadeWhite");

    lua_pushcfunction(L, l_trSlide);
    lua_setfield(L, -2, "slide");

    lua_pushcfunction(L, l_trDissolve);
    lua_setfield(L, -2, "dissolve");

    lua_pushcfunction(L, l_trBlind);
    lua_setfield(L, -2, "blind");

    lua_pushcfunction(L, l_trMosaic);
    lua_setfield(L, -2, "mosaic");

    lua_pushcfunction(L, l_trCurtain);
    lua_setfield(L, -2, "curtain");

    lua_pushcfunction(L, l_trZoom);
    lua_setfield(L, -2, "zoom");

    lua_pushcfunction(L, l_trStop);
    lua_setfield(L, -2, "stop");

    lua_pushcfunction(L, l_trIsActive);
    lua_setfield(L, -2, "isActive");

    lua_setglobal(L, "Transition");
}

int LuaBindings::l_trFade(lua_State* L) {
    float duration = luaL_optnumber(L, 1, 0.5f);
    lua_getfield(L, LUA_REGISTRYINDEX, K_TRANSITION);
    auto* tr = static_cast<Transition*>(lua_touserdata(L, -1));
    lua_pop(L, 1);
    if (tr) tr->fade(duration);
    return 0;
}

int LuaBindings::l_trFadeWhite(lua_State* L) {
    float duration = luaL_optnumber(L, 1, 0.5f);
    lua_getfield(L, LUA_REGISTRYINDEX, K_TRANSITION);
    auto* tr = static_cast<Transition*>(lua_touserdata(L, -1));
    lua_pop(L, 1);
    if (tr) tr->fadeWhite(duration);
    return 0;
}

int LuaBindings::l_trSlide(lua_State* L) {
    const char* dir = luaL_optstring(L, 1, "left");
    float duration = luaL_optnumber(L, 2, 0.4f);
    lua_getfield(L, LUA_REGISTRYINDEX, K_TRANSITION);
    auto* tr = static_cast<Transition*>(lua_touserdata(L, -1));
    lua_pop(L, 1);
    if (!tr) return 0;
    std::string d(dir);
    if (d == "left") tr->slideLeft(duration);
    else if (d == "right") tr->slideRight(duration);
    else if (d == "up") tr->slideUp(duration);
    else if (d == "down") tr->slideDown(duration);
    else tr->slideLeft(duration);
    return 0;
}

int LuaBindings::l_trDissolve(lua_State* L) {
    float duration = luaL_optnumber(L, 1, 0.6f);
    lua_getfield(L, LUA_REGISTRYINDEX, K_TRANSITION);
    auto* tr = static_cast<Transition*>(lua_touserdata(L, -1));
    lua_pop(L, 1);
    if (tr) tr->dissolve(duration);
    return 0;
}

int LuaBindings::l_trBlind(lua_State* L) {
    float duration = luaL_optnumber(L, 1, 0.5f);
    lua_getfield(L, LUA_REGISTRYINDEX, K_TRANSITION);
    auto* tr = static_cast<Transition*>(lua_touserdata(L, -1));
    lua_pop(L, 1);
    if (tr) tr->blind(duration);
    return 0;
}

int LuaBindings::l_trMosaic(lua_State* L) {
    float duration = luaL_optnumber(L, 1, 0.6f);
    lua_getfield(L, LUA_REGISTRYINDEX, K_TRANSITION);
    auto* tr = static_cast<Transition*>(lua_touserdata(L, -1));
    lua_pop(L, 1);
    if (tr) tr->mosaic(duration);
    return 0;
}

int LuaBindings::l_trCurtain(lua_State* L) {
    float duration = luaL_optnumber(L, 1, 0.5f);
    lua_getfield(L, LUA_REGISTRYINDEX, K_TRANSITION);
    auto* tr = static_cast<Transition*>(lua_touserdata(L, -1));
    lua_pop(L, 1);
    if (tr) tr->curtain(duration);
    return 0;
}

int LuaBindings::l_trZoom(lua_State* L) {
    float duration = luaL_optnumber(L, 1, 0.5f);
    lua_getfield(L, LUA_REGISTRYINDEX, K_TRANSITION);
    auto* tr = static_cast<Transition*>(lua_touserdata(L, -1));
    lua_pop(L, 1);
    if (tr) tr->zoom(duration);
    return 0;
}

int LuaBindings::l_trStop(lua_State* L) {
    lua_getfield(L, LUA_REGISTRYINDEX, K_TRANSITION);
    auto* tr = static_cast<Transition*>(lua_touserdata(L, -1));
    lua_pop(L, 1);
    if (tr) tr->stop();
    return 0;
}

int LuaBindings::l_trIsActive(lua_State* L) {
    lua_getfield(L, LUA_REGISTRYINDEX, K_TRANSITION);
    auto* tr = static_cast<Transition*>(lua_touserdata(L, -1));
    lua_pop(L, 1);
    lua_pushboolean(L, tr && tr->isActive());
    return 1;
}

// === 背景动画 API ===

void LuaBindings::registerBgAnimAPI() {
    lua_State* L = m_script->getState();
    lua_newtable(L);

    lua_pushcfunction(L, l_bgSetWeather);
    lua_setfield(L, -2, "setWeather");

    lua_pushcfunction(L, l_bgShake);
    lua_setfield(L, -2, "shake");

    lua_pushcfunction(L, l_bgStopShake);
    lua_setfield(L, -2, "stopShake");

    lua_setglobal(L, "BgEffect");
}

int LuaBindings::l_bgSetWeather(lua_State* L) {
    const char* type = luaL_optstring(L, 1, "none");
    float intensity = luaL_optnumber(L, 2, 1.0f);
    lua_getfield(L, LUA_REGISTRYINDEX, K_BGANIM);
    auto* bg = static_cast<BackgroundAnimation*>(lua_touserdata(L, -1));
    lua_pop(L, 1);
    if (!bg) return 0;
    std::string t(type);
    if (t == "rain") bg->setWeather(WeatherType::RAIN);
    else if (t == "snow") bg->setWeather(WeatherType::SNOW);
    else if (t == "fog") bg->setWeather(WeatherType::FOG);
    else bg->setWeather(WeatherType::NONE);
    bg->setWeatherIntensity(intensity);
    return 0;
}

int LuaBindings::l_bgShake(lua_State* L) {
    float intensity = luaL_optnumber(L, 1, 8.0f);
    float duration = luaL_optnumber(L, 2, 0.3f);
    lua_getfield(L, LUA_REGISTRYINDEX, K_BGANIM);
    auto* bg = static_cast<BackgroundAnimation*>(lua_touserdata(L, -1));
    lua_pop(L, 1);
    if (bg) bg->shake(intensity, duration);
    return 0;
}

int LuaBindings::l_bgStopShake(lua_State* L) {
    lua_getfield(L, LUA_REGISTRYINDEX, K_BGANIM);
    auto* bg = static_cast<BackgroundAnimation*>(lua_touserdata(L, -1));
    lua_pop(L, 1);
    if (bg) bg->stopShake();
    return 0;
}

// === 结局列表 API ===

void LuaBindings::registerEndingListAPI() {
    lua_State* L = m_script->getState();
    lua_newtable(L);

    lua_pushcfunction(L, l_elAdd);
    lua_setfield(L, -2, "add");

    lua_pushcfunction(L, l_elUnlock);
    lua_setfield(L, -2, "unlock");

    lua_pushcfunction(L, l_elIsUnlocked);
    lua_setfield(L, -2, "isUnlocked");

    lua_pushcfunction(L, l_elShow);
    lua_setfield(L, -2, "show");

    lua_pushcfunction(L, l_elHide);
    lua_setfield(L, -2, "hide");

    lua_pushcfunction(L, l_elGetCompletion);
    lua_setfield(L, -2, "getCompletion");

    lua_setglobal(L, "Ending");
}

int LuaBindings::l_elAdd(lua_State* L) {
    // add(id, title, description, condition)
    EndingEntry entry;
    entry.id = luaL_checkstring(L, 1);
    entry.title = luaL_checkstring(L, 2);
    entry.description = luaL_optstring(L, 3, "");
    entry.unlockCondition = luaL_optstring(L, 4, "");
    lua_getfield(L, LUA_REGISTRYINDEX, K_ENDINGLIST);
    auto* el = static_cast<EndingList*>(lua_touserdata(L, -1));
    lua_pop(L, 1);
    if (el) el->addEnding(entry);
    return 0;
}

int LuaBindings::l_elUnlock(lua_State* L) {
    const char* id = luaL_checkstring(L, 1);
    lua_getfield(L, LUA_REGISTRYINDEX, K_ENDINGLIST);
    auto* el = static_cast<EndingList*>(lua_touserdata(L, -1));
    lua_pop(L, 1);
    if (el) el->unlockEnding(id);
    return 0;
}

int LuaBindings::l_elIsUnlocked(lua_State* L) {
    const char* id = luaL_checkstring(L, 1);
    lua_getfield(L, LUA_REGISTRYINDEX, K_ENDINGLIST);
    auto* el = static_cast<EndingList*>(lua_touserdata(L, -1));
    lua_pop(L, 1);
    lua_pushboolean(L, el && el->isUnlocked(id));
    return 1;
}

int LuaBindings::l_elShow(lua_State* L) {
    lua_getfield(L, LUA_REGISTRYINDEX, K_ENDINGLIST);
    auto* el = static_cast<EndingList*>(lua_touserdata(L, -1));
    lua_pop(L, 1);
    if (el) el->show();
    return 0;
}

int LuaBindings::l_elHide(lua_State* L) {
    lua_getfield(L, LUA_REGISTRYINDEX, K_ENDINGLIST);
    auto* el = static_cast<EndingList*>(lua_touserdata(L, -1));
    lua_pop(L, 1);
    if (el) el->hide();
    return 0;
}

int LuaBindings::l_elGetCompletion(lua_State* L) {
    lua_getfield(L, LUA_REGISTRYINDEX, K_ENDINGLIST);
    auto* el = static_cast<EndingList*>(lua_touserdata(L, -1));
    lua_pop(L, 1);
    if (el) lua_pushnumber(L, el->getCompletionRate());
    else lua_pushnumber(L, 0.0);
    return 1;
}

// === 场景回想 API ===

void LuaBindings::registerSceneReplayAPI() {
    lua_State* L = m_script->getState();
    lua_newtable(L);

    lua_pushcfunction(L, l_srAdd);
    lua_setfield(L, -2, "add");

    lua_pushcfunction(L, l_srUnlock);
    lua_setfield(L, -2, "unlock");

    lua_pushcfunction(L, l_srShow);
    lua_setfield(L, -2, "show");

    lua_pushcfunction(L, l_srHide);
    lua_setfield(L, -2, "hide");

    lua_setglobal(L, "SceneReplay");
}

int LuaBindings::l_srAdd(lua_State* L) {
    // add(id, title, description, scriptName, startLine, endLine, chapter)
    SceneReplayEntry entry;
    entry.id = luaL_checkstring(L, 1);
    entry.title = luaL_checkstring(L, 2);
    entry.description = luaL_optstring(L, 3, "");
    entry.scriptName = luaL_optstring(L, 4, "");
    entry.startLine = luaL_optinteger(L, 5, 0);
    entry.endLine = luaL_optinteger(L, 6, 0);
    entry.chapter = luaL_optstring(L, 7, "");
    lua_getfield(L, LUA_REGISTRYINDEX, K_SCENEREPLAY);
    auto* sr = static_cast<SceneReplay*>(lua_touserdata(L, -1));
    lua_pop(L, 1);
    if (sr) sr->addScene(entry);
    return 0;
}

int LuaBindings::l_srUnlock(lua_State* L) {
    const char* id = luaL_checkstring(L, 1);
    lua_getfield(L, LUA_REGISTRYINDEX, K_SCENEREPLAY);
    auto* sr = static_cast<SceneReplay*>(lua_touserdata(L, -1));
    lua_pop(L, 1);
    if (sr) sr->unlockScene(id);
    return 0;
}

int LuaBindings::l_srShow(lua_State* L) {
    lua_getfield(L, LUA_REGISTRYINDEX, K_SCENEREPLAY);
    auto* sr = static_cast<SceneReplay*>(lua_touserdata(L, -1));
    lua_pop(L, 1);
    if (sr) sr->show();
    return 0;
}

int LuaBindings::l_srHide(lua_State* L) {
    lua_getfield(L, LUA_REGISTRYINDEX, K_SCENEREPLAY);
    auto* sr = static_cast<SceneReplay*>(lua_touserdata(L, -1));
    lua_pop(L, 1);
    if (sr) sr->hide();
    return 0;
}

// === 词典 API ===

void LuaBindings::registerGlossaryAPI() {
    lua_State* L = m_script->getState();
    lua_newtable(L);

    lua_pushcfunction(L, l_glAdd);
    lua_setfield(L, -2, "add");

    lua_pushcfunction(L, l_glUnlock);
    lua_setfield(L, -2, "unlock");

    lua_pushcfunction(L, l_glShow);
    lua_setfield(L, -2, "show");

    lua_pushcfunction(L, l_glHide);
    lua_setfield(L, -2, "hide");

    lua_pushcfunction(L, l_glShowTerm);
    lua_setfield(L, -2, "showTerm");

    lua_setglobal(L, "Glossary");
}

int LuaBindings::l_glAdd(lua_State* L) {
    // add(term, description, category, reading)
    std::string term = luaL_checkstring(L, 1);
    std::string desc = luaL_checkstring(L, 2);
    std::string cat = luaL_optstring(L, 3, "一般");
    std::string reading = luaL_optstring(L, 4, "");
    lua_getfield(L, LUA_REGISTRYINDEX, K_GLOSSARY);
    auto* gl = static_cast<Glossary*>(lua_touserdata(L, -1));
    lua_pop(L, 1);
    if (gl) {
        GlossaryEntry entry;
        entry.term = term;
        entry.description = desc;
        entry.category = cat;
        entry.reading = reading;
        gl->addEntry(entry);
    }
    return 0;
}

int LuaBindings::l_glUnlock(lua_State* L) {
    const char* term = luaL_checkstring(L, 1);
    lua_getfield(L, LUA_REGISTRYINDEX, K_GLOSSARY);
    auto* gl = static_cast<Glossary*>(lua_touserdata(L, -1));
    lua_pop(L, 1);
    if (gl) gl->unlockEntry(term);
    return 0;
}

int LuaBindings::l_glShow(lua_State* L) {
    lua_getfield(L, LUA_REGISTRYINDEX, K_GLOSSARY);
    auto* gl = static_cast<Glossary*>(lua_touserdata(L, -1));
    lua_pop(L, 1);
    if (gl) gl->show();
    return 0;
}

int LuaBindings::l_glHide(lua_State* L) {
    lua_getfield(L, LUA_REGISTRYINDEX, K_GLOSSARY);
    auto* gl = static_cast<Glossary*>(lua_touserdata(L, -1));
    lua_pop(L, 1);
    if (gl) gl->hide();
    return 0;
}

int LuaBindings::l_glShowTerm(lua_State* L) {
    const char* term = luaL_checkstring(L, 1);
    lua_getfield(L, LUA_REGISTRYINDEX, K_GLOSSARY);
    auto* gl = static_cast<Glossary*>(lua_touserdata(L, -1));
    lua_pop(L, 1);
    if (gl) gl->showTermDetail(term);
    return 0;
}

// ===================================================================
// 新功能3: 5个扩展功能 - Lua 绑定实现
// ===================================================================

// === 视频播放(FMV) API ===

void LuaBindings::registerVideoAPI() {
    lua_State* L = m_script->getState();
    lua_newtable(L);

    lua_pushcfunction(L, l_videoLoad);       lua_setfield(L, -2, "load");
    lua_pushcfunction(L, l_videoPlay);       lua_setfield(L, -2, "play");
    lua_pushcfunction(L, l_videoPause);      lua_setfield(L, -2, "pause");
    lua_pushcfunction(L, l_videoStop);       lua_setfield(L, -2, "stop");
    lua_pushcfunction(L, l_videoIsPlaying);  lua_setfield(L, -2, "isPlaying");
    lua_pushcfunction(L, l_videoSetLoop);    lua_setfield(L, -2, "setLoop");
    lua_pushcfunction(L, l_videoSetRegion);  lua_setfield(L, -2, "setRegion");
    lua_pushcfunction(L, l_videoSetSpeed);   lua_setfield(L, -2, "setSpeed");
    lua_pushcfunction(L, l_videoSeek);       lua_setfield(L, -2, "seek");
    lua_pushcfunction(L, l_videoSetAudio);   lua_setfield(L, -2, "setAudio");

    lua_setglobal(L, "Video");
}

int LuaBindings::l_videoLoad(lua_State* L) {
    auto* vp = static_cast<VideoPlayer*>(getEnginePtr(L, K_VIDEO));
    const char* path = luaL_checkstring(L, 1);
    int fps = luaL_optinteger(L, 2, 30);
    if (vp) lua_pushboolean(L, vp->load(path, fps));
    else lua_pushboolean(L, false);
    return 1;
}

int LuaBindings::l_videoPlay(lua_State* L) {
    auto* vp = static_cast<VideoPlayer*>(getEnginePtr(L, K_VIDEO));
    if (vp) vp->play();
    return 0;
}

int LuaBindings::l_videoPause(lua_State* L) {
    auto* vp = static_cast<VideoPlayer*>(getEnginePtr(L, K_VIDEO));
    if (vp) vp->pause();
    return 0;
}

int LuaBindings::l_videoStop(lua_State* L) {
    auto* vp = static_cast<VideoPlayer*>(getEnginePtr(L, K_VIDEO));
    if (vp) vp->stop();
    return 0;
}

int LuaBindings::l_videoIsPlaying(lua_State* L) {
    auto* vp = static_cast<VideoPlayer*>(getEnginePtr(L, K_VIDEO));
    if (vp) lua_pushboolean(L, vp->isPlaying());
    else lua_pushboolean(L, false);
    return 1;
}

int LuaBindings::l_videoSetLoop(lua_State* L) {
    auto* vp = static_cast<VideoPlayer*>(getEnginePtr(L, K_VIDEO));
    bool loop = lua_toboolean(L, 1);
    if (vp) vp->setLoop(loop);
    return 0;
}

int LuaBindings::l_videoSetRegion(lua_State* L) {
    auto* vp = static_cast<VideoPlayer*>(getEnginePtr(L, K_VIDEO));
    int x = luaL_checkinteger(L, 1);
    int y = luaL_checkinteger(L, 2);
    int w = luaL_checkinteger(L, 3);
    int h = luaL_checkinteger(L, 4);
    if (vp) vp->setRegion(x, y, w, h);
    return 0;
}

int LuaBindings::l_videoSetSpeed(lua_State* L) {
    auto* vp = static_cast<VideoPlayer*>(getEnginePtr(L, K_VIDEO));
    float speed = luaL_checknumber(L, 1);
    if (vp) vp->setSpeed(speed);
    return 0;
}

int LuaBindings::l_videoSeek(lua_State* L) {
    auto* vp = static_cast<VideoPlayer*>(getEnginePtr(L, K_VIDEO));
    int frame = luaL_checkinteger(L, 1);
    if (vp) vp->seek(frame);
    return 0;
}

int LuaBindings::l_videoSetAudio(lua_State* L) {
    auto* vp = static_cast<VideoPlayer*>(getEnginePtr(L, K_VIDEO));
    const char* path = luaL_checkstring(L, 1);
    if (vp) vp->setAudioPath(path);
    return 0;
}

// === 配置持久化 API ===

void LuaBindings::registerConfigPersistAPI() {
    lua_State* L = m_script->getState();
    lua_newtable(L);

    lua_pushcfunction(L, l_configLoad);        lua_setfield(L, -2, "load");
    lua_pushcfunction(L, l_configSave);        lua_setfield(L, -2, "save");
    lua_pushcfunction(L, l_configSetString);   lua_setfield(L, -2, "setString");
    lua_pushcfunction(L, l_configSetInt);      lua_setfield(L, -2, "setInt");
    lua_pushcfunction(L, l_configSetBool);     lua_setfield(L, -2, "setBool");
    lua_pushcfunction(L, l_configGetString);   lua_setfield(L, -2, "getString");
    lua_pushcfunction(L, l_configGetInt);      lua_setfield(L, -2, "getInt");
    lua_pushcfunction(L, l_configGetBool);     lua_setfield(L, -2, "getBool");
    lua_pushcfunction(L, l_configSetWindow);   lua_setfield(L, -2, "setWindow");
    lua_pushcfunction(L, l_configGetWindow);   lua_setfield(L, -2, "getWindow");
    lua_pushcfunction(L, l_configSetAudio);    lua_setfield(L, -2, "setAudio");
    lua_pushcfunction(L, l_configGetAudio);    lua_setfield(L, -2, "getAudio");
    lua_pushcfunction(L, l_configSetLanguage); lua_setfield(L, -2, "setLanguage");
    lua_pushcfunction(L, l_configGetLanguage); lua_setfield(L, -2, "getLanguage");

    lua_setglobal(L, "Config");
}

int LuaBindings::l_configLoad(lua_State* L) {
    auto* cp = static_cast<ConfigPersistence*>(getEnginePtr(L, K_CONFIGP));
    if (cp) lua_pushboolean(L, cp->load());
    else lua_pushboolean(L, false);
    return 1;
}

int LuaBindings::l_configSave(lua_State* L) {
    auto* cp = static_cast<ConfigPersistence*>(getEnginePtr(L, K_CONFIGP));
    if (cp) lua_pushboolean(L, cp->save());
    else lua_pushboolean(L, false);
    return 1;
}

int LuaBindings::l_configSetString(lua_State* L) {
    auto* cp = static_cast<ConfigPersistence*>(getEnginePtr(L, K_CONFIGP));
    const char* key = luaL_checkstring(L, 1);
    const char* val = luaL_checkstring(L, 2);
    if (cp) cp->setString(key, val);
    return 0;
}

int LuaBindings::l_configSetInt(lua_State* L) {
    auto* cp = static_cast<ConfigPersistence*>(getEnginePtr(L, K_CONFIGP));
    const char* key = luaL_checkstring(L, 1);
    int val = luaL_checkinteger(L, 2);
    if (cp) cp->setInt(key, val);
    return 0;
}

int LuaBindings::l_configSetBool(lua_State* L) {
    auto* cp = static_cast<ConfigPersistence*>(getEnginePtr(L, K_CONFIGP));
    const char* key = luaL_checkstring(L, 1);
    bool val = lua_toboolean(L, 2);
    if (cp) cp->setBool(key, val);
    return 0;
}

int LuaBindings::l_configGetString(lua_State* L) {
    auto* cp = static_cast<ConfigPersistence*>(getEnginePtr(L, K_CONFIGP));
    const char* key = luaL_checkstring(L, 1);
    const char* def = luaL_optstring(L, 2, "");
    if (cp) lua_pushstring(L, cp->getString(key, def).c_str());
    else lua_pushstring(L, def);
    return 1;
}

int LuaBindings::l_configGetInt(lua_State* L) {
    auto* cp = static_cast<ConfigPersistence*>(getEnginePtr(L, K_CONFIGP));
    const char* key = luaL_checkstring(L, 1);
    int def = luaL_optinteger(L, 2, 0);
    if (cp) lua_pushinteger(L, cp->getInt(key, def));
    else lua_pushinteger(L, def);
    return 1;
}

int LuaBindings::l_configGetBool(lua_State* L) {
    auto* cp = static_cast<ConfigPersistence*>(getEnginePtr(L, K_CONFIGP));
    const char* key = luaL_checkstring(L, 1);
    bool def = lua_toboolean(L, 2);
    if (cp) lua_pushboolean(L, cp->getBool(key, def));
    else lua_pushboolean(L, def);
    return 1;
}

int LuaBindings::l_configSetWindow(lua_State* L) {
    auto* cp = static_cast<ConfigPersistence*>(getEnginePtr(L, K_CONFIGP));
    int w = luaL_checkinteger(L, 1);
    int h = luaL_checkinteger(L, 2);
    bool fs = lua_toboolean(L, 3);
    if (cp) cp->setWindowConfig(w, h, fs);
    return 0;
}

int LuaBindings::l_configGetWindow(lua_State* L) {
    auto* cp = static_cast<ConfigPersistence*>(getEnginePtr(L, K_CONFIGP));
    int w, h; bool fs;
    if (cp) cp->getWindowConfig(w, h, fs);
    else { w = 1280; h = 720; fs = false; }
    lua_pushinteger(L, w);
    lua_pushinteger(L, h);
    lua_pushboolean(L, fs);
    return 3;
}

int LuaBindings::l_configSetAudio(lua_State* L) {
    auto* cp = static_cast<ConfigPersistence*>(getEnginePtr(L, K_CONFIGP));
    int master = luaL_checkinteger(L, 1);
    int bgm = luaL_checkinteger(L, 2);
    int voice = luaL_checkinteger(L, 3);
    int se = luaL_checkinteger(L, 4);
    if (cp) cp->setAudioConfig(master, bgm, voice, se);
    return 0;
}

int LuaBindings::l_configGetAudio(lua_State* L) {
    auto* cp = static_cast<ConfigPersistence*>(getEnginePtr(L, K_CONFIGP));
    int master, bgm, voice, se;
    if (cp) cp->getAudioConfig(master, bgm, voice, se);
    else { master = bgm = voice = se = 100; }
    lua_pushinteger(L, master);
    lua_pushinteger(L, bgm);
    lua_pushinteger(L, voice);
    lua_pushinteger(L, se);
    return 4;
}

int LuaBindings::l_configSetLanguage(lua_State* L) {
    auto* cp = static_cast<ConfigPersistence*>(getEnginePtr(L, K_CONFIGP));
    const char* lang = luaL_checkstring(L, 1);
    if (cp) cp->setLanguage(lang);
    return 0;
}

int LuaBindings::l_configGetLanguage(lua_State* L) {
    auto* cp = static_cast<ConfigPersistence*>(getEnginePtr(L, K_CONFIGP));
    if (cp) lua_pushstring(L, cp->getLanguage().c_str());
    else lua_pushstring(L, "ja");
    return 1;
}

// === 窗口缩放/全屏 API ===

void LuaBindings::registerWindowAPI() {
    lua_State* L = m_script->getState();
    lua_newtable(L);

    lua_pushcfunction(L, l_winToggleFullscreen);    lua_setfield(L, -2, "toggleFullscreen");
    lua_pushcfunction(L, l_winSetFullscreen);       lua_setfield(L, -2, "setFullscreen");
    lua_pushcfunction(L, l_winIsFullscreen);        lua_setfield(L, -2, "isFullscreen");
    lua_pushcfunction(L, l_winSetResolution);       lua_setfield(L, -2, "setResolution");
    lua_pushcfunction(L, l_winGetResolution);       lua_setfield(L, -2, "getResolution");
    lua_pushcfunction(L, l_winGetAvailableResolutions); lua_setfield(L, -2, "getAvailableResolutions");
    lua_pushcfunction(L, l_winSetRenderScale);      lua_setfield(L, -2, "setRenderScale");
    lua_pushcfunction(L, l_winMaximize);            lua_setfield(L, -2, "maximize");
    lua_pushcfunction(L, l_winCenter);              lua_setfield(L, -2, "center");

    lua_setglobal(L, "Window");
}

int LuaBindings::l_winToggleFullscreen(lua_State* L) {
    auto* wr = static_cast<WindowResize*>(getEnginePtr(L, K_WINDOW));
    if (wr) wr->toggleFullscreen();
    return 0;
}

int LuaBindings::l_winSetFullscreen(lua_State* L) {
    auto* wr = static_cast<WindowResize*>(getEnginePtr(L, K_WINDOW));
    bool fs = lua_toboolean(L, 1);
    if (wr) wr->setFullscreen(fs);
    return 0;
}

int LuaBindings::l_winIsFullscreen(lua_State* L) {
    auto* wr = static_cast<WindowResize*>(getEnginePtr(L, K_WINDOW));
    if (wr) lua_pushboolean(L, wr->isFullscreen());
    else lua_pushboolean(L, false);
    return 1;
}

int LuaBindings::l_winSetResolution(lua_State* L) {
    auto* wr = static_cast<WindowResize*>(getEnginePtr(L, K_WINDOW));
    int w = luaL_checkinteger(L, 1);
    int h = luaL_checkinteger(L, 2);
    if (wr) wr->setResolution(w, h);
    return 0;
}

int LuaBindings::l_winGetResolution(lua_State* L) {
    auto* wr = static_cast<WindowResize*>(getEnginePtr(L, K_WINDOW));
    int w, h;
    if (wr) wr->getResolution(w, h);
    else { w = 1280; h = 720; }
    lua_pushinteger(L, w);
    lua_pushinteger(L, h);
    return 2;
}

int LuaBindings::l_winGetAvailableResolutions(lua_State* L) {
    auto* wr = static_cast<WindowResize*>(getEnginePtr(L, K_WINDOW));
    lua_newtable(L);
    if (wr) {
        auto resos = wr->getAvailableResolutions();
        for (size_t i = 0; i < resos.size(); ++i) {
            lua_newtable(L);
            lua_pushinteger(L, resos[i].width);
            lua_setfield(L, -2, "width");
            lua_pushinteger(L, resos[i].height);
            lua_setfield(L, -2, "height");
            lua_pushstring(L, resos[i].label.c_str());
            lua_setfield(L, -2, "label");
            lua_rawseti(L, -2, i + 1);
        }
    }
    return 1;
}

int LuaBindings::l_winSetRenderScale(lua_State* L) {
    auto* wr = static_cast<WindowResize*>(getEnginePtr(L, K_WINDOW));
    float scale = luaL_checknumber(L, 1);
    if (wr) wr->setRenderScale(scale);
    return 0;
}

int LuaBindings::l_winMaximize(lua_State* L) {
    auto* wr = static_cast<WindowResize*>(getEnginePtr(L, K_WINDOW));
    if (wr) wr->maximize();
    return 0;
}

int LuaBindings::l_winCenter(lua_State* L) {
    auto* wr = static_cast<WindowResize*>(getEnginePtr(L, K_WINDOW));
    if (wr) wr->centerWindow();
    return 0;
}

// === 多语言(i18n) API ===

void LuaBindings::registerLocalizationAPI() {
    lua_State* L = m_script->getState();
    lua_newtable(L);

    lua_pushcfunction(L, l_i18nLoad);           lua_setfield(L, -2, "load");
    lua_pushcfunction(L, l_i18nSetLanguage);    lua_setfield(L, -2, "setLanguage");
    lua_pushcfunction(L, l_i18nGetLanguage);    lua_setfield(L, -2, "getLanguage");
    lua_pushcfunction(L, l_i18nTranslate);      lua_setfield(L, -2, "translate");
    lua_pushcfunction(L, l_i18nTranslateFormat); lua_setfield(L, -2, "translateFormat");
    lua_pushcfunction(L, l_i18nHas);            lua_setfield(L, -2, "has");
    lua_pushcfunction(L, l_i18nAdd);            lua_setfield(L, -2, "add");
    lua_pushcfunction(L, l_i18nGetAvailable);   lua_setfield(L, -2, "getAvailable");

    lua_setglobal(L, "I18n");
}

int LuaBindings::l_i18nLoad(lua_State* L) {
    auto* loc = static_cast<Localization*>(getEnginePtr(L, K_I18N));
    const char* lang = luaL_checkstring(L, 1);
    if (loc) lua_pushboolean(L, loc->loadLanguage(lang));
    else lua_pushboolean(L, false);
    return 1;
}

int LuaBindings::l_i18nSetLanguage(lua_State* L) {
    auto* loc = static_cast<Localization*>(getEnginePtr(L, K_I18N));
    const char* lang = luaL_checkstring(L, 1);
    if (loc) lua_pushboolean(L, loc->setLanguage(lang));
    else lua_pushboolean(L, false);
    return 1;
}

int LuaBindings::l_i18nGetLanguage(lua_State* L) {
    auto* loc = static_cast<Localization*>(getEnginePtr(L, K_I18N));
    if (loc) lua_pushstring(L, loc->getCurrentLanguage().c_str());
    else lua_pushstring(L, "ja");
    return 1;
}

int LuaBindings::l_i18nTranslate(lua_State* L) {
    auto* loc = static_cast<Localization*>(getEnginePtr(L, K_I18N));
    const char* key = luaL_checkstring(L, 1);
    if (loc) lua_pushstring(L, loc->translate(key).c_str());
    else lua_pushstring(L, key);
    return 1;
}

int LuaBindings::l_i18nTranslateFormat(lua_State* L) {
    auto* loc = static_cast<Localization*>(getEnginePtr(L, K_I18N));
    const char* key = luaL_checkstring(L, 1);
    std::vector<std::string> args;
    int n = lua_gettop(L);
    for (int i = 2; i <= n; ++i) {
        const char* arg = lua_tostring(L, i);
        args.push_back(arg ? arg : "");
    }
    if (loc) lua_pushstring(L, loc->translateFormat(key, args).c_str());
    else lua_pushstring(L, key);
    return 1;
}

int LuaBindings::l_i18nHas(lua_State* L) {
    auto* loc = static_cast<Localization*>(getEnginePtr(L, K_I18N));
    const char* key = luaL_checkstring(L, 1);
    if (loc) lua_pushboolean(L, loc->hasTranslation(key));
    else lua_pushboolean(L, false);
    return 1;
}

int LuaBindings::l_i18nAdd(lua_State* L) {
    auto* loc = static_cast<Localization*>(getEnginePtr(L, K_I18N));
    const char* lang = luaL_checkstring(L, 1);
    const char* key = luaL_checkstring(L, 2);
    const char* val = luaL_checkstring(L, 3);
    if (loc) loc->addTranslation(lang, key, val);
    return 0;
}

int LuaBindings::l_i18nGetAvailable(lua_State* L) {
    auto* loc = static_cast<Localization*>(getEnginePtr(L, K_I18N));
    lua_newtable(L);
    if (loc) {
        auto langs = loc->getLoadedLanguages();
        for (size_t i = 0; i < langs.size(); ++i) {
            lua_pushstring(L, langs[i].c_str());
            lua_rawseti(L, -2, i + 1);
        }
    }
    return 1;
}

// === 角色表情差分 API ===

void LuaBindings::registerCharExprAPI() {
    lua_State* L = m_script->getState();
    lua_newtable(L);

    lua_pushcfunction(L, l_exprSetBase);      lua_setfield(L, -2, "setBase");
    lua_pushcfunction(L, l_exprAddDiff);      lua_setfield(L, -2, "addDiff");
    lua_pushcfunction(L, l_exprSet);          lua_setfield(L, -2, "set");
    lua_pushcfunction(L, l_exprSetInstant);   lua_setfield(L, -2, "setInstant");
    lua_pushcfunction(L, l_exprGetCurrent);   lua_setfield(L, -2, "getCurrent");
    lua_pushcfunction(L, l_exprGetAvailable); lua_setfield(L, -2, "getAvailable");
    lua_pushcfunction(L, l_exprSetVisible);   lua_setfield(L, -2, "setVisible");
    lua_pushcfunction(L, l_exprSetAlpha);     lua_setfield(L, -2, "setAlpha");
    lua_pushcfunction(L, l_exprSetPosition);  lua_setfield(L, -2, "setPosition");
    lua_pushcfunction(L, l_exprSetBlinking);  lua_setfield(L, -2, "setBlinking");
    lua_pushcfunction(L, l_exprSetLipSync);   lua_setfield(L, -2, "setLipSync");

    lua_setglobal(L, "Expression");
}

int LuaBindings::l_exprSetBase(lua_State* L) {
    auto* ce = static_cast<CharacterExpression*>(getEnginePtr(L, K_CHAREXPR));
    const char* path = luaL_checkstring(L, 1);
    if (ce) {
        // 通过 ResourceManager 加载纹理
        auto* resMgr = static_cast<ResourceManager*>(getEnginePtr(L, K_RESMGR));
        if (resMgr) {
            auto tex = resMgr->getTexture(path);
            ce->setBaseTexture(tex);
        }
    }
    return 0;
}

int LuaBindings::l_exprAddDiff(lua_State* L) {
    auto* ce = static_cast<CharacterExpression*>(getEnginePtr(L, K_CHAREXPR));
    const char* name = luaL_checkstring(L, 1);
    const char* path = luaL_checkstring(L, 2);
    int blendInt = luaL_optinteger(L, 3, 0);  // 0=ALPHA, 1=ADD, 2=SUBTRACT, 3=OVERWRITE
    float offX = luaL_optnumber(L, 4, 0.0f);
    float offY = luaL_optnumber(L, 5, 0.0f);
    if (ce) {
        auto* resMgr = static_cast<ResourceManager*>(getEnginePtr(L, K_RESMGR));
        if (resMgr) {
            auto tex = resMgr->getTexture(path);
            DiffBlendMode blend = DiffBlendMode::ALPHA;
            switch (blendInt) {
                case 1: blend = DiffBlendMode::ADD; break;
                case 2: blend = DiffBlendMode::SUBTRACT; break;
                case 3: blend = DiffBlendMode::OVERWRITE; break;
                default: blend = DiffBlendMode::ALPHA; break;
            }
            ce->addDiff(name, tex, blend, offX, offY);
        }
    }
    return 0;
}

int LuaBindings::l_exprSet(lua_State* L) {
    auto* ce = static_cast<CharacterExpression*>(getEnginePtr(L, K_CHAREXPR));
    const char* name = luaL_checkstring(L, 1);
    float dur = luaL_optnumber(L, 2, 0.3f);
    if (ce) ce->setExpression(name, dur);
    return 0;
}

int LuaBindings::l_exprSetInstant(lua_State* L) {
    auto* ce = static_cast<CharacterExpression*>(getEnginePtr(L, K_CHAREXPR));
    const char* name = luaL_checkstring(L, 1);
    if (ce) ce->setExpressionInstant(name);
    return 0;
}

int LuaBindings::l_exprGetCurrent(lua_State* L) {
    auto* ce = static_cast<CharacterExpression*>(getEnginePtr(L, K_CHAREXPR));
    if (ce) lua_pushstring(L, ce->getCurrentExpression().c_str());
    else lua_pushstring(L, "");
    return 1;
}

int LuaBindings::l_exprGetAvailable(lua_State* L) {
    auto* ce = static_cast<CharacterExpression*>(getEnginePtr(L, K_CHAREXPR));
    lua_newtable(L);
    if (ce) {
        auto exprs = ce->getAvailableExpressions();
        for (size_t i = 0; i < exprs.size(); ++i) {
            lua_pushstring(L, exprs[i].c_str());
            lua_rawseti(L, -2, i + 1);
        }
    }
    return 1;
}

int LuaBindings::l_exprSetVisible(lua_State* L) {
    auto* ce = static_cast<CharacterExpression*>(getEnginePtr(L, K_CHAREXPR));
    const char* name = luaL_checkstring(L, 1);
    bool vis = lua_toboolean(L, 2);
    if (ce) ce->setDiffVisible(name, vis);
    return 0;
}

int LuaBindings::l_exprSetAlpha(lua_State* L) {
    auto* ce = static_cast<CharacterExpression*>(getEnginePtr(L, K_CHAREXPR));
    const char* name = luaL_checkstring(L, 1);
    int alpha = luaL_checkinteger(L, 2);
    if (ce) ce->setDiffAlpha(name, static_cast<Uint8>(alpha));
    return 0;
}

int LuaBindings::l_exprSetPosition(lua_State* L) {
    auto* ce = static_cast<CharacterExpression*>(getEnginePtr(L, K_CHAREXPR));
    float x = luaL_checknumber(L, 1);
    float y = luaL_checknumber(L, 2);
    if (ce) ce->setPosition(x, y);
    return 0;
}

int LuaBindings::l_exprSetBlinking(lua_State* L) {
    auto* ce = static_cast<CharacterExpression*>(getEnginePtr(L, K_CHAREXPR));
    bool enabled = lua_toboolean(L, 1);
    float interval = luaL_optnumber(L, 2, 3.0f);
    if (ce) ce->setBlinking(enabled, interval);
    return 0;
}

int LuaBindings::l_exprSetLipSync(lua_State* L) {
    auto* ce = static_cast<CharacterExpression*>(getEnginePtr(L, K_CHAREXPR));
    bool enabled = lua_toboolean(L, 1);
    if (ce) ce->setLipSync(enabled);
    return 0;
}

// === 资源包 API ===

void LuaBindings::registerPackageAPI() {
    lua_State* L = m_script->getState();
    lua_newtable(L);

    lua_pushcfunction(L, l_pkgLoad);
    lua_setfield(L, -2, "load");

    lua_pushcfunction(L, l_pkgUnload);
    lua_setfield(L, -2, "unload");

    lua_pushcfunction(L, l_pkgIsLoaded);
    lua_setfield(L, -2, "isLoaded");

    lua_pushcfunction(L, l_pkgHas);
    lua_setfield(L, -2, "has");

    lua_pushcfunction(L, l_pkgList);
    lua_setfield(L, -2, "list");

    lua_pushcfunction(L, l_pkgGetFileCount);
    lua_setfield(L, -2, "getFileCount");

    lua_setglobal(L, "Package");
}

int LuaBindings::l_pkgLoad(lua_State* L) {
    auto* resMgr = static_cast<ResourceManager*>(getEnginePtr(L, K_RESMGR));
    const char* path = luaL_checkstring(L, 1);
    if (!resMgr) {
        lua_pushboolean(L, 0);
        return 1;
    }
    lua_pushboolean(L, resMgr->loadPackage(path) ? 1 : 0);
    return 1;
}

int LuaBindings::l_pkgUnload(lua_State* L) {
    auto* resMgr = static_cast<ResourceManager*>(getEnginePtr(L, K_RESMGR));
    if (resMgr) resMgr->unloadPackage();
    return 0;
}

int LuaBindings::l_pkgIsLoaded(lua_State* L) {
    auto* resMgr = static_cast<ResourceManager*>(getEnginePtr(L, K_RESMGR));
    lua_pushboolean(L, (resMgr && resMgr->hasPackage()) ? 1 : 0);
    return 1;
}

int LuaBindings::l_pkgHas(lua_State* L) {
    auto* resMgr = static_cast<ResourceManager*>(getEnginePtr(L, K_RESMGR));
    const char* name = luaL_checkstring(L, 1);
    lua_pushboolean(L, (resMgr && resMgr->isInPackage(name)) ? 1 : 0);
    return 1;
}

int LuaBindings::l_pkgList(lua_State* L) {
    auto* resMgr = static_cast<ResourceManager*>(getEnginePtr(L, K_RESMGR));
    if (!resMgr || !resMgr->hasPackage()) {
        lua_newtable(L);
        return 1;
    }
    auto names = resMgr->listPackageResources();
    lua_newtable(L);
    for (size_t i = 0; i < names.size(); i++) {
        lua_pushstring(L, names[i].c_str());
        lua_rawseti(L, -2, static_cast<int>(i + 1));
    }
    return 1;
}

int LuaBindings::l_pkgGetFileCount(lua_State* L) {
    auto* resMgr = static_cast<ResourceManager*>(getEnginePtr(L, K_RESMGR));
    if (!resMgr || !resMgr->hasPackage()) {
        lua_pushinteger(L, 0);
        return 1;
    }
    // 通过 list 的大小获取
    auto names = resMgr->listPackageResources();
    lua_pushinteger(L, static_cast<lua_Integer>(names.size()));
    return 1;
}
