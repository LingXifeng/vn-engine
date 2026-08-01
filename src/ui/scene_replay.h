#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include "renderer.h"
#include "input.h"

// 场景回想条目
struct SceneReplayEntry {
    std::string id;
    std::string title;          // 场景标题
    std::string description;    // 场景描述
    std::string thumbnailPath;  // 缩略图
    std::string scriptName;     // 脚本名
    int startLine = 0;          // 起始行
    int endLine = 0;            // 结束行
    std::string chapter;        // 所属章节
    bool unlocked = false;      // 是否解锁
    std::string unlockCondition;// 解锁条件
};

// 场景回想
class SceneReplay {
public:
    SceneReplay(Renderer* renderer);
    ~SceneReplay();

    void setFont(TTF_Font* font) { m_font = font; }
    void setSmallFont(TTF_Font* font) { m_smallFont = font; }

    // 添加场景
    void addScene(const SceneReplayEntry& scene);
    void clearScenes();

    // 解锁场景
    bool unlockScene(const std::string& id);
    bool isUnlocked(const std::string& id) const;

    // 保存/加载解锁状态
    std::string serialize() const;
    bool deserialize(const std::string& data);

    // 界面控制
    void show();
    void hide();
    bool isVisible() const { return m_visible; }

    // 回想回调（播放指定场景）
    void setReplayCallback(std::function<void(const std::string& scriptName,
                                               int startLine, int endLine)> cb) {
        m_replayCallback = cb;
    }

    // 更新与渲染
    void update(float dt, const Input& input);
    void render();

private:
    Renderer* m_renderer;
    TTF_Font* m_font = nullptr;
    TTF_Font* m_smallFont = nullptr;

    bool m_visible = false;
    float m_fadeAlpha = 0.0f;

    std::vector<SceneReplayEntry> m_scenes;
    int m_hoverIndex = -1;
    int m_selectedIndex = -1;

    // 滚动
    float m_scrollY = 0.0f;
    float m_targetScrollY = 0.0f;

    // 章节过滤
    std::string m_filterChapter;
    std::vector<std::string> m_chapters;

    // 确认对话框
    bool m_confirmVisible = false;
    int m_confirmIndex = -1;

    std::function<void(const std::string&, int, int)> m_replayCallback;

    void updateScroll(float dt);
    SDL_Rect getSceneRect(int index) const;
    void renderSceneCard(const SceneReplayEntry& scene, int index, Uint8 alpha);
    void renderConfirm(Uint8 alpha);
    std::vector<SceneReplayEntry> getFilteredScenes() const;
};
