#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include "renderer.h"
#include "input.h"

// 结局条目
struct EndingEntry {
    std::string id;
    std::string title;          // 结局名称
    std::string description;    // 结局描述
    std::string imagePath;      // 结局图片
    std::string scriptName;     // 对应脚本
    int scriptLine = 0;
    bool unlocked = false;      // 是否已达成
    std::string unlockCondition; // 解锁条件描述
    int priority = 0;           // 优先级（普通结局 < True End）
};

// 结局列表
class EndingList {
public:
    EndingList(Renderer* renderer);
    ~EndingList();

    void setFont(TTF_Font* font) { m_font = font; }
    void setSmallFont(TTF_Font* font) { m_smallFont = font; }

    // 添加结局
    void addEnding(const EndingEntry& ending);
    void clearEndings();

    // 解锁结局
    bool unlockEnding(const std::string& id);
    bool isUnlocked(const std::string& id) const;

    // 获取统计
    int getTotalCount() const { return static_cast<int>(m_endings.size()); }
    int getUnlockedCount() const;
    float getCompletionRate() const;

    // 保存/加载解锁状态
    std::string serialize() const;
    bool deserialize(const std::string& data);

    // 界面控制
    void show();
    void hide();
    bool isVisible() const { return m_visible; }

    // 选择结局回调（跳转到该结局）
    void setSelectCallback(std::function<void(const std::string& scriptName, int line)> cb) {
        m_selectCallback = cb;
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

    std::vector<EndingEntry> m_endings;
    int m_hoverIndex = -1;
    int m_selectedIndex = -1;

    // 滚动
    float m_scrollY = 0.0f;
    float m_targetScrollY = 0.0f;

    // 详情面板
    bool m_detailVisible = false;
    int m_detailIndex = -1;

    std::function<void(const std::string&, int)> m_selectCallback;

    void updateScroll(float dt);
    SDL_Rect getEndingRect(int index) const;
    void renderEndingCard(const EndingEntry& ending, int index, Uint8 alpha);
    void renderDetail(Uint8 alpha);
};
