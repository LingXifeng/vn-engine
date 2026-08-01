#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <string>
#include <vector>
#include <memory>
#include "renderer.h"
#include "input.h"
#include "audio.h"

// BGM 条目
struct BGMEntry {
    std::string id;
    std::string title;       // 曲名
    std::string artist;      // 作曲/艺术家
    std::string filePath;    // 文件路径
    bool unlocked = false;   // 是否解锁
    int duration = 0;        // 时长（秒），0=未知
};

// 语音条目
struct VoiceEntry {
    std::string id;
    std::string speaker;     // 说话者
    std::string text;        // 语音文本
    std::string filePath;    // 音频文件
    bool unlocked = false;   // 是否解锁
    int duration = 0;        // 时长（秒）
};

// 音乐室
class MusicRoom {
public:
    MusicRoom(Renderer* renderer, Audio* audio);
    ~MusicRoom();

    void setFont(TTF_Font* font) { m_font = font; }

    // BGM 管理
    void addBGM(const std::string& id, const std::string& title,
                const std::string& artist, const std::string& filePath);
    void unlockBGM(const std::string& id);
    bool isBGMUnlocked(const std::string& id) const;

    // 语音管理
    void addVoice(const std::string& id, const std::string& speaker,
                  const std::string& text, const std::string& filePath);
    void unlockVoice(const std::string& id);

    // 存档
    std::vector<std::string> getUnlockedBGMIDs() const;
    std::vector<std::string> getUnlockedVoiceIDs() const;
    void setUnlockedBGMIDs(const std::vector<std::string>& ids);
    void setUnlockedVoiceIDs(const std::vector<std::string>& ids);

    // 控制
    void show();
    void hide();
    bool isVisible() const { return m_visible; }

    // 更新与渲染
    void update(float dt, const Input& input);
    void render();

private:
    Renderer* m_renderer;
    Audio* m_audio;
    TTF_Font* m_font = nullptr;

    bool m_visible = false;
    float m_fadeAlpha = 0.0f;

    // 标签页
    enum class Tab { BGM, VOICE };
    Tab m_currentTab = Tab::BGM;

    // BGM 列表
    std::vector<BGMEntry> m_bgms;
    int m_bgmHoverIndex = -1;
    int m_bgmSelectedIndex = -1;
    int m_bgmScrollOffset = 0;
    bool m_bgmPlaying = false;

    // 语音列表
    std::vector<VoiceEntry> m_voices;
    int m_voiceHoverIndex = -1;
    int m_voiceSelectedIndex = -1;
    int m_voiceScrollOffset = 0;
    bool m_voicePlaying = false;

    // 布局参数
    int m_listX = 50;
    int m_listY = 80;
    int m_listW = 700;
    int m_itemH = 40;
    int m_visibleItems = 12;

    // 播放状态
    float m_playTime = 0.0f;

    // 辅助
    int getItemIndexAt(int x, int y) const;
    void renderBGMList();
    void renderVoiceList();
    void renderTabs();
    void renderPlaybackBar();
    std::string formatTime(int seconds) const;
};
