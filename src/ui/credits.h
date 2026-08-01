#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <string>
#include <vector>
#include <memory>
#include "renderer.h"
#include "input.h"

// Credits 段落类型
enum class CreditSectionType {
    TITLE,       // 大标题（如 "STAFF"）
    HEADING,     // 中标题（如 "原画"）
    NAME,        // 人名
    SMALL,       // 小字（版权信息等）
    BLANK,       // 空行
};

// Credits 段落
struct CreditSection {
    CreditSectionType type = CreditSectionType::NAME;
    std::string text;
    SDL_Color color = {255, 255, 255, 255};
    int fontSize = 24;       // 字号
    int spacingAfter = 10;   // 段后间距
};

// Credits 滚动字幕
class Credits {
public:
    Credits(Renderer* renderer);
    ~Credits();

    // 设置字体（不同大小）
    void setTitleFont(TTF_Font* font) { m_titleFont = font; }
    void setHeadingFont(TTF_Font* font) { m_headingFont = font; }
    void setBodyFont(TTF_Font* font) { m_bodyFont = font; }
    void setSmallFont(TTF_Font* font) { m_smallFont = font; }

    // 添加内容
    void addTitle(const std::string& text, SDL_Color color = {255, 220, 100, 255});
    void addHeading(const std::string& text, SDL_Color color = {200, 200, 255, 255});
    void addName(const std::string& text, SDL_Color color = {255, 255, 255, 255});
    void addSmall(const std::string& text, SDL_Color color = {150, 150, 150, 255});
    void addBlank(int spacing = 30);

    // 从文件加载（简单文本格式）
    bool loadFromFile(const std::string& path);

    // 控制
    void show();
    void hide();
    bool isVisible() const { return m_visible; }
    bool isFinished() const { return m_finished; }

    // 设置
    void setScrollSpeed(float speed) { m_scrollSpeed = speed; }
    void setLoop(bool loop) { m_loop = loop; }
    void setFadeDistance(int pixels) { m_fadeDistance = pixels; }

    // 更新与渲染
    void update(float dt, const Input& input);
    void render();

    // 重置到开头
    void reset();

private:
    Renderer* m_renderer;
    TTF_Font* m_titleFont = nullptr;
    TTF_Font* m_headingFont = nullptr;
    TTF_Font* m_bodyFont = nullptr;
    TTF_Font* m_smallFont = nullptr;

    bool m_visible = false;
    bool m_finished = false;
    bool m_loop = false;

    std::vector<CreditSection> m_sections;

    // 滚动状态
    float m_scrollY = 0.0f;        // 当前滚动偏移
    float m_scrollSpeed = 50.0f;   // 滚动速度（像素/秒）
    float m_totalHeight = 0.0f;    // 总内容高度
    int m_fadeDistance = 80;       // 渐入渐出区域高度

    // 预渲染的段落纹理
    struct CachedSection {
        std::shared_ptr<Texture> texture;
        int height;
        int spacingAfter;
        CreditSectionType type;
    };
    std::vector<CachedSection> m_cached;
    bool m_cacheDirty = true;

    void rebuildCache();
    TTF_Font* getFontForType(CreditSectionType type) const;
    int getFontSizeForType(CreditSectionType type) const;
};
