#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <string>
#include <vector>
#include <memory>
#include "renderer.h"

// 文字样式标志
enum class TextStyle : int {
    NORMAL    = 0,
    BOLD      = 1 << 0,
    ITALIC    = 1 << 1,
    UNDERLINE = 1 << 2,
    SHADOW    = 1 << 3,
    OUTLINE   = 1 << 4,
    RUBY      = 1 << 5,    // 有注音
};

inline TextStyle operator|(TextStyle a, TextStyle b) {
    return (TextStyle)((int)a | (int)b);
}
inline TextStyle operator&(TextStyle a, TextStyle b) {
    return (TextStyle)((int)a & (int)b);
}
inline bool hasStyle(TextStyle flags, TextStyle check) {
    return ((int)flags & (int)check) != 0;
}

// 富文本片段（一个字或一段相同样式的文字）
struct RichSegment {
    std::string text;           // 文字内容
    std::string ruby;           // 注音（ルビ），空表示无
    TextStyle style = TextStyle::NORMAL;
    SDL_Color color = {255, 255, 255, 255};
    SDL_Color shadowColor = {0, 0, 0, 128};
    SDL_Color outlineColor = {0, 0, 0, 255};
};

// 富文本行
struct RichLine {
    std::vector<RichSegment> segments;
    int width = 0;
    int height = 0;
    int baseHeight = 0;     // 不含注音的高度
    int rubyHeight = 0;     // 注音高度
};

// 富文本渲染样式配置
struct RichTextConfig {
    int lineSpacing = 4;        // 行间距
    int charSpacing = 0;        // 字间距
    int rubySize = 12;          // 注音字号
    int rubyOffset = 2;         // 注音与正文间距
    SDL_Color defaultColor = {255, 255, 255, 255};
    SDL_Color defaultShadowColor = {0, 0, 0, 128};
    SDL_Color defaultOutlineColor = {0, 0, 0, 255};
    bool defaultShadow = false;
    bool defaultOutline = false;
};

// 富文本渲染器
class RichTextRenderer {
public:
    RichTextRenderer(Renderer* renderer);
    ~RichTextRenderer();

    // 设置字体（正常体、粗体）
    void setFont(TTF_Font* normal, TTF_Font* bold = nullptr);

    // 解析标记文本为富文本片段
    // 支持的标记：
    //   {b}...{/b}       粗体
    //   {i}...{/i}       斜体
    //   {u}...{/u}       下划线
    //   {ruby:注音}文字{/ruby}  注音
    //   {color:RRGGBB}...{/color}  颜色
    //   {shadow}...{/shadow}  阴影
    //   {outline}...{/outline}  边缘
    std::vector<RichSegment> parse(const std::string& markup) const;

    // 布局：将片段按最大宽度折行
    std::vector<RichLine> layout(const std::vector<RichSegment>& segments,
                                  int maxWidth, const RichTextConfig& config) const;

    // 渲染到指定位置
    void render(const std::vector<RichLine>& lines, int x, int y,
                const RichTextConfig& config) const;

    // 便捷方法：解析 + 布局 + 渲染
    void renderText(const std::string& markup, int x, int y, int maxWidth,
                    const RichTextConfig& config) const;

    // 测量文本尺寸
    void measure(const std::vector<RichLine>& lines, int& totalW, int& totalH,
                 const RichTextConfig& config) const;

private:
    Renderer* m_renderer;
    TTF_Font* m_fontNormal = nullptr;
    TTF_Font* m_fontBold = nullptr;

    // 渲染单个片段
    void renderSegment(const RichSegment& seg, int x, int y,
                       const RichTextConfig& config) const;

    // 获取片段尺寸
    void segmentSize(const RichSegment& seg, int& w, int& h, int& rubyH,
                     const RichTextConfig& config) const;

    // 渲染带阴影/边缘的文字
    std::shared_ptr<Texture> renderStyledText(const std::string& text, TTF_Font* font,
                                               SDL_Color color, TextStyle style,
                                               SDL_Color shadowColor, SDL_Color outlineColor) const;

    // 颜色解析
    SDL_Color parseColor(const std::string& hex) const;
};
