#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <functional>
#include "renderer.h"
#include "input.h"

// 词典条目
struct GlossaryEntry {
    std::string term;           // 术语
    std::string reading;        // 读音（可选）
    std::string category;       // 分类
    std::string description;    // 释义
    std::string imagePath;      // 配图（可选）
    bool unlocked = false;      // 是否解锁
    std::string unlockCondition;// 解锁条件
};

// 词典/用語集
class Glossary {
public:
    Glossary(Renderer* renderer);
    ~Glossary();

    void setFont(TTF_Font* font) { m_font = font; }
    void setSmallFont(TTF_Font* font) { m_smallFont = font; }

    // 添加词条
    void addEntry(const GlossaryEntry& entry);
    void addEntry(const std::string& term, const std::string& description,
                  const std::string& category = "一般");
    void clearEntries();

    // 解锁词条
    bool unlockEntry(const std::string& term);
    bool isUnlocked(const std::string& term) const;

    // 查找词条
    const GlossaryEntry* findEntry(const std::string& term) const;

    // 从文本中提取已知术语（用于高亮）
    std::vector<std::pair<int, int>> findTermsInText(const std::string& text) const;

    // 保存/加载
    std::string serialize() const;
    bool deserialize(const std::string& data);

    // 界面控制
    void show();
    void hide();
    bool isVisible() const { return m_visible; }

    // 显示特定词条详情（点击文本中术语时）
    void showTermDetail(const std::string& term);

    // 更新与渲染
    void update(float dt, const Input& input);
    void render();

    // 获取统计
    int getTotalCount() const { return static_cast<int>(m_entries.size()); }
    int getUnlockedCount() const;

private:
    Renderer* m_renderer;
    TTF_Font* m_font = nullptr;
    TTF_Font* m_smallFont = nullptr;

    bool m_visible = false;
    float m_fadeAlpha = 0.0f;

    std::vector<GlossaryEntry> m_entries;
    std::unordered_map<std::string, int> m_termIndex;  // 术语 -> 索引

    // 界面状态
    int m_hoverIndex = -1;
    int m_selectedIndex = -1;
    std::string m_filterCategory;
    std::vector<std::string> m_categories;

    // 滚动
    float m_scrollY = 0.0f;
    float m_targetScrollY = 0.0f;

    // 详情面板
    bool m_detailVisible = false;
    int m_detailIndex = -1;

    // 弹出提示（文本中点击术语）
    bool m_popupVisible = false;
    int m_popupIndex = -1;
    float m_popupTimer = 0.0f;

    void updateScroll(float dt);
    void rebuildIndex();
    std::vector<const GlossaryEntry*> getFilteredEntries() const;
    void renderEntry(const GlossaryEntry& entry, int index, float y, Uint8 alpha);
    void renderDetail(Uint8 alpha);
    void renderPopup(Uint8 alpha);
};
