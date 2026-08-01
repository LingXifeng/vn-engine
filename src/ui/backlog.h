#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include "renderer.h"
#include "input.h"
#include "audio.h"

// 履历条目
struct BacklogEntry {
    std::string speaker;     // 说话者
    std::string text;        // 对话内容
    std::string voiceFile;   // 语音文件路径（可空）
    std::string scriptName;  // 脚本名
    int scriptLine = 0;      // 脚本行号
    bool read = true;        // 是否已读
};

// 履历/回看
class Backlog {
public:
    Backlog(Renderer* renderer, Audio* audio);
    ~Backlog();

    void setFont(TTF_Font* font) { m_font = font; }
    void setSmallFont(TTF_Font* font) { m_smallFont = font; }

    // 添加对话记录
    void addEntry(const std::string& speaker, const std::string& text,
                  const std::string& voiceFile = "",
                  const std::string& scriptName = "", int scriptLine = 0);

    // 清空
    void clear();
    int size() const { return static_cast<int>(m_entries.size()); }

    // 获取所有条目
    const std::vector<BacklogEntry>& getEntries() const { return m_entries; }

    // 回看界面控制
    void show();
    void hide();
    bool isVisible() const { return m_visible; }

    // 跳转回调（跳回某一句）
    void setJumpCallback(std::function<void(const std::string& scriptName, int line)> cb) {
        m_jumpCallback = cb;
    }

    // 设置最大记录数
    void setMaxEntries(int max) { m_maxEntries = max; }

    // 更新与渲染
    void update(float dt, const Input& input);
    void render();

private:
    Renderer* m_renderer;
    Audio* m_audio;
    TTF_Font* m_font = nullptr;
    TTF_Font* m_smallFont = nullptr;

    bool m_visible = false;
    float m_fadeAlpha = 0.0f;

    std::vector<BacklogEntry> m_entries;
    int m_maxEntries = 500;

    // 滚动
    float m_scrollY = 0.0f;
    float m_targetScrollY = 0.0f;
    float m_contentHeight = 0.0f;

    // 交互
    int m_hoverIndex = -1;
    bool m_dragging = false;
    int m_dragStartY = 0;
    float m_scrollStartY = 0;

    // 回调
    std::function<void(const std::string&, int)> m_jumpCallback;

    // 语音回放状态
    int m_playingIndex = -1;

    void updateScroll(float dt);
    float getEntryHeight(int index) const;
    void renderEntry(const BacklogEntry& entry, int index, float y, Uint8 alpha);
};
