#pragma once

#include <lua.hpp>
#include <memory>
#include "script_engine.h"
#include "../engine/engine.h"
#include "../engine/renderer.h"
#include "../engine/audio.h"
#include "../engine/input.h"
#include "../engine/resource_manager.h"
#include "../engine/tween.h"
#include "../adv/scene.h"
#include "../ui/title_screen.h"
#include "../ui/config_screen.h"
#include "../ui/cg_gallery.h"
#include "../ui/rich_text.h"
#include "../ui/music_room.h"
#include "../ui/voice_gallery.h"
#include "../ui/credits.h"
#include "../ui/flowchart.h"
#include "../engine/particle.h"
#include "../ui/save_load_ui.h"
#include "../ui/backlog.h"
#include "../ui/auto_skip.h"
#include "../ui/transition.h"
#include "../ui/background_animation.h"
#include "../ui/ending_list.h"
#include "../ui/scene_replay.h"
#include "../ui/glossary.h"
#include "../engine/video_player.h"
#include "../engine/config_persistence.h"
#include "../engine/window_resize.h"
#include "../engine/localization.h"
#include "../engine/character_expression.h"

// Lua 绑定 - 将 C++ 引擎 API 暴露给 Lua 脚本
class LuaBindings {
public:
    LuaBindings(ScriptEngine* script, Engine* engine, Scene* scene,
                Audio* audio, ResourceManager* resMgr, TweenManager* tweenMgr);
    ~LuaBindings();

    // 注册所有绑定
    void registerAll();

    // UI 组件更新与渲染（由引擎主循环调用）
    void updateUI(float dt, const Input& input);
    void renderUI();

private:
    ScriptEngine* m_script;
    Engine* m_engine;
    Scene* m_scene;
    Audio* m_audio;
    ResourceManager* m_resMgr;
    TweenManager* m_tweenMgr;

    // 新功能对象（由绑定层创建和管理）
    std::unique_ptr<ParticleSystem> m_particles;
    std::unique_ptr<CGGallery> m_cgGallery;
    std::unique_ptr<RichTextRenderer> m_richText;
    std::unique_ptr<MusicRoom> m_musicRoom;
    std::unique_ptr<VoiceGallery> m_voiceGallery;
    std::unique_ptr<Credits> m_credits;
    std::unique_ptr<Flowchart> m_flowchart;
    RichTextConfig m_richTextConfig;  // 富文本默认配置

    // 新功能2: 8个扩展功能
    std::unique_ptr<SaveLoadUI> m_saveLoadUI;
    std::unique_ptr<Backlog> m_backlog;
    std::unique_ptr<AutoSkip> m_autoSkip;
    std::unique_ptr<Transition> m_transition;
    std::unique_ptr<BackgroundAnimation> m_bgAnim;
    std::unique_ptr<EndingList> m_endingList;
    std::unique_ptr<SceneReplay> m_sceneReplay;
    std::unique_ptr<Glossary> m_glossary;

    // 新功能3: 5个扩展功能
    std::unique_ptr<VideoPlayer> m_videoPlayer;
    std::unique_ptr<ConfigPersistence> m_configPersist;
    std::unique_ptr<WindowResize> m_windowResize;
    std::unique_ptr<Localization> m_localization;
    std::unique_ptr<CharacterExpression> m_charExpr;

    // 注册各个模块
    void registerSceneAPI();    // 场景/背景
    void registerCharacterAPI(); // 角色立绘
    void registerTextAPI();     // 文字/对话
    void registerAudioAPI();    // 音频
    void registerTweenAPI();    // 动画
    void registerInputAPI();    // 输入
    void registerSystemAPI();   // 系统
    void registerFlowAPI();     // 流程控制（等待、跳转）

    // 新功能绑定
    void registerParticleAPI();   // 粒子特效
    void registerGalleryAPI();    // CG 画廊
    void registerRichTextAPI();   // 富文本
    void registerMusicRoomAPI();  // 音乐室
    void registerCreditsAPI();    // Credits 滚动
    void registerFlowchartAPI();  // 流程图

    // 新功能2绑定
    void registerSaveLoadUIAPI();   // 存档/读档 UI
    void registerBacklogAPI();      // 履历回看
    void registerAutoSkipAPI();     // 自动/跳过
    void registerTransitionAPI();   // 转场特效
    void registerBgAnimAPI();       // 背景动画
    void registerEndingListAPI();   // 结局列表
    void registerSceneReplayAPI();  // 场景回想
    void registerGlossaryAPI();     // 词典

    // 新功能3绑定
    void registerVideoAPI();        // 视频播放(FMV)
    void registerConfigPersistAPI();// 配置持久化
    void registerWindowAPI();       // 窗口缩放/全屏
    void registerLocalizationAPI(); // 多语言(i18n)
    void registerCharExprAPI();     // 角色表情差分

    // 资源包绑定
    void registerPackageAPI();      // 资源包(.pak)

    // 辅助：存储引擎指针到 Lua registry
    static void storeEnginePtr(lua_State* L, void* ptr, const char* key);
    static void* getEnginePtr(lua_State* L, const char* key);

    // === 场景 API 的 C 函数 ===
    static int l_setBackground(lua_State* L);
    static int l_clearBackground(lua_State* L);
    static int l_fadeIn(lua_State* L);
    static int l_fadeOut(lua_State* L);

    // === 角色 API ===
    static int l_showCharacter(lua_State* L);
    static int l_hideCharacter(lua_State* L);
    static int l_setExpression(lua_State* L);
    static int l_moveCharacter(lua_State* L);

    // === 文字 API ===
    static int l_say(lua_State* L);        // 显示对话
    static int l_narrate(lua_State* L);    // 旁白
    static int l_clearText(lua_State* L);
    static int l_setTextColor(lua_State* L);
    static int l_setTextSpeed(lua_State* L);

    // === 选项 API ===
    static int l_choice(lua_State* L);     // 显示选项

    // === 音频 API ===
    static int l_playBGM(lua_State* L);
    static int l_stopBGM(lua_State* L);
    static int l_playSE(lua_State* L);
    static int l_playVoice(lua_State* L);
    static int l_setVolume(lua_State* L);

    // === 动画 API ===
    static int l_tween(lua_State* L);
    static int l_wait(lua_State* L);

    // === 系统 API ===
    static int l_log(lua_State* L);
    static int l_getScreenSize(lua_State* L);
    static int l_setTitle(lua_State* L);
    static int l_quit(lua_State* L);

    // === 存档 API ===
    static int l_save(lua_State* L);
    static int l_load(lua_State* L);

    // === 粒子特效 API ===
    static int l_particleCreate(lua_State* L);
    static int l_particleSetType(lua_State* L);
    static int l_particleSetParam(lua_State* L);
    static int l_particleStart(lua_State* L);
    static int l_particleStop(lua_State* L);
    static int l_particleClear(lua_State* L);

    // === CG 画廊 API ===
    static int l_galleryUnlock(lua_State* L);
    static int l_galleryIsUnlocked(lua_State* L);
    static int l_galleryShow(lua_State* L);
    static int l_galleryLoadData(lua_State* L);
    static int l_gallerySaveData(lua_State* L);

    // === 富文本 API ===
    static int l_richTextSetRuby(lua_State* L);
    static int l_richTextSetBold(lua_State* L);
    static int l_richTextSetShadow(lua_State* L);
    static int l_richTextSetSpacing(lua_State* L);
    static int l_richTextRender(lua_State* L);

    // === 音乐室 API ===
    static int l_musicRoomAdd(lua_State* L);
    static int l_musicRoomShow(lua_State* L);
    static int l_musicRoomPlay(lua_State* L);
    static int l_musicRoomStop(lua_State* L);

    // === Credits API ===
    static int l_creditsAddTitle(lua_State* L);
    static int l_creditsAddHeading(lua_State* L);
    static int l_creditsAddName(lua_State* L);
    static int l_creditsAddSmall(lua_State* L);
    static int l_creditsLoadFile(lua_State* L);
    static int l_creditsShow(lua_State* L);
    static int l_creditsHide(lua_State* L);
    static int l_creditsSetSpeed(lua_State* L);

    // === 流程图 API ===
    static int l_fcAddNode(lua_State* L);
    static int l_fcAddEdge(lua_State* L);
    static int l_fcSetPosition(lua_State* L);
    static int l_fcMarkVisited(lua_State* L);
    static int l_fcMarkCurrent(lua_State* L);
    static int l_fcAutoLayout(lua_State* L);
    static int l_fcShow(lua_State* L);
    static int l_fcHide(lua_State* L);
    static int l_fcResetView(lua_State* L);
    static int l_fcGetVisited(lua_State* L);
    static int l_fcGetEndings(lua_State* L);

    // === 存档/读档 UI API ===
    static int l_slShowSave(lua_State* L);
    static int l_slShowLoad(lua_State* L);
    static int l_slHide(lua_State* L);
    static int l_slSetSlotCount(lua_State* L);

    // === 履历 API ===
    static int l_backlogAdd(lua_State* L);
    static int l_backlogShow(lua_State* L);
    static int l_backlogHide(lua_State* L);
    static int l_backlogClear(lua_State* L);

    // === 自动/跳过 API ===
    static int l_asSetAuto(lua_State* L);
    static int l_asSetSkip(lua_State* L);
    static int l_asToggleAuto(lua_State* L);
    static int l_asToggleSkip(lua_State* L);
    static int l_asSetAutoSpeed(lua_State* L);
    static int l_asStopAll(lua_State* L);

    // === 转场特效 API ===
    static int l_trFade(lua_State* L);
    static int l_trFadeWhite(lua_State* L);
    static int l_trSlide(lua_State* L);
    static int l_trDissolve(lua_State* L);
    static int l_trBlind(lua_State* L);
    static int l_trMosaic(lua_State* L);
    static int l_trCurtain(lua_State* L);
    static int l_trZoom(lua_State* L);
    static int l_trStop(lua_State* L);
    static int l_trIsActive(lua_State* L);

    // === 背景动画 API ===
    static int l_bgSetWeather(lua_State* L);
    static int l_bgShake(lua_State* L);
    static int l_bgStopShake(lua_State* L);

    // === 结局列表 API ===
    static int l_elAdd(lua_State* L);
    static int l_elUnlock(lua_State* L);
    static int l_elIsUnlocked(lua_State* L);
    static int l_elShow(lua_State* L);
    static int l_elHide(lua_State* L);
    static int l_elGetCompletion(lua_State* L);

    // === 场景回想 API ===
    static int l_srAdd(lua_State* L);
    static int l_srUnlock(lua_State* L);
    static int l_srShow(lua_State* L);
    static int l_srHide(lua_State* L);

    // === 词典 API ===
    static int l_glAdd(lua_State* L);
    static int l_glUnlock(lua_State* L);
    static int l_glShow(lua_State* L);
    static int l_glHide(lua_State* L);
    static int l_glShowTerm(lua_State* L);

    // === 视频播放(FMV) API ===
    static int l_videoLoad(lua_State* L);
    static int l_videoPlay(lua_State* L);
    static int l_videoPause(lua_State* L);
    static int l_videoStop(lua_State* L);
    static int l_videoIsPlaying(lua_State* L);
    static int l_videoSetLoop(lua_State* L);
    static int l_videoSetRegion(lua_State* L);
    static int l_videoSetSpeed(lua_State* L);
    static int l_videoSeek(lua_State* L);
    static int l_videoSetAudio(lua_State* L);

    // === 配置持久化 API ===
    static int l_configLoad(lua_State* L);
    static int l_configSave(lua_State* L);
    static int l_configSetString(lua_State* L);
    static int l_configSetInt(lua_State* L);
    static int l_configSetBool(lua_State* L);
    static int l_configGetString(lua_State* L);
    static int l_configGetInt(lua_State* L);
    static int l_configGetBool(lua_State* L);
    static int l_configSetWindow(lua_State* L);
    static int l_configGetWindow(lua_State* L);
    static int l_configSetAudio(lua_State* L);
    static int l_configGetAudio(lua_State* L);
    static int l_configSetLanguage(lua_State* L);
    static int l_configGetLanguage(lua_State* L);

    // === 窗口缩放/全屏 API ===
    static int l_winToggleFullscreen(lua_State* L);
    static int l_winSetFullscreen(lua_State* L);
    static int l_winIsFullscreen(lua_State* L);
    static int l_winSetResolution(lua_State* L);
    static int l_winGetResolution(lua_State* L);
    static int l_winGetAvailableResolutions(lua_State* L);
    static int l_winSetRenderScale(lua_State* L);
    static int l_winMaximize(lua_State* L);
    static int l_winCenter(lua_State* L);

    // === 多语言(i18n) API ===
    static int l_i18nLoad(lua_State* L);
    static int l_i18nSetLanguage(lua_State* L);
    static int l_i18nGetLanguage(lua_State* L);
    static int l_i18nTranslate(lua_State* L);
    static int l_i18nTranslateFormat(lua_State* L);
    static int l_i18nHas(lua_State* L);
    static int l_i18nAdd(lua_State* L);
    static int l_i18nGetAvailable(lua_State* L);

    // === 角色表情差分 API ===
    static int l_exprSetBase(lua_State* L);
    static int l_exprAddDiff(lua_State* L);
    static int l_exprSet(lua_State* L);
    static int l_exprSetInstant(lua_State* L);
    static int l_exprGetCurrent(lua_State* L);
    static int l_exprGetAvailable(lua_State* L);
    static int l_exprSetVisible(lua_State* L);
    static int l_exprSetAlpha(lua_State* L);
    static int l_exprSetPosition(lua_State* L);
    static int l_exprSetBlinking(lua_State* L);
    static int l_exprSetLipSync(lua_State* L);

    // === 资源包 API ===
    static int l_pkgLoad(lua_State* L);
    static int l_pkgUnload(lua_State* L);
    static int l_pkgIsLoaded(lua_State* L);
    static int l_pkgHas(lua_State* L);
    static int l_pkgList(lua_State* L);
    static int l_pkgGetFileCount(lua_State* L);
};
