#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <string>
#include <vector>
#include <memory>
#include "renderer.h"
#include "input.h"
#include "audio.h"

// 语音回想条目（比 music_room 更详细的语音浏览）
struct VoiceRecallEntry {
    std::string id;
    std::string speaker;       // 说话者名
    std::string text;          // 台词
    std::string filePath;      // 语音文件
    std::string sceneName;     // 所属场景
    bool unlocked = false;
};

// 语音回想画廊
class VoiceGallery {
public:
    VoiceGallery(Renderer* renderer, Audio* audio);
    ~VoiceGallery();

    void setFont(TTF_Font* font) { m_font = font; }

    // 管理
    void addVoice(const std::string& id, const std::string& speaker,
                  const std::string& text, const std::string& filePath,
                  const std::string& sceneName = "");
    void unlockVoice(const std::string& id);
    void unlockAll();

    // 存档
    std::vector<std::string> getUnlockedIDs() const;
    void setUnlockedIDs(const std::vector<std::string>& ids);

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

    std::vector<VoiceRecallEntry> m_voices;
    int m_hoverIndex = -1;
    int m_selectedIndex = -1;
    int m_scrollOffset = 0;

    // 按说话者过滤
    std::string m_filterSpeaker;
    std::vector<std::string> m_speakers;

    // 布局
    int m_listX = 50;
    int m_listY = 90;
    int m_listW = 700;
    int m_itemH = 50;
    int m_visibleItems = 10;

    int getFilteredIndexAt(int x, int y) const;
    std::vector<int> getFilteredIndices() const;
    void renderList();
    void renderDetail();
    std::string truncate(const std::string& s, size_t maxLen) const;
};
