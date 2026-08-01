#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <string>
#include <vector>
#include <memory>
#include "renderer.h"
#include "tween.h"

// 文字显示模式
enum class TextMode {
    NORMAL,     // 普通模式
    WAIT,       // 等待点击
    TYPING,     // 打字机效果中
    DONE        // 文字全部显示完毕
};

// 文字框
class TextBox {
public:
    TextBox(Renderer* renderer);
    ~TextBox();

    // 设置
    void setFont(TTF_Font* font) { m_font = font; }
    void setPosition(int x, int y) { m_x = x; m_y = y; }
    void setSize(int w, int h) { m_width = w; m_height = h; }
    void setBackColor(SDL_Color color) { m_backColor = color; }
    void setTextColor(SDL_Color color) { m_textColor = color; }
    void setNameColor(SDL_Color color) { m_nameColor = color; }
    void setTypingSpeed(float charsPerSec) { m_typingSpeed = charsPerSec; }

    // 显示文字
    void show(const std::string& name, const std::string& text);
    void append(const std::string& text);  // 追加文字（不换行）
    void clear();

    // 控制
    void skip();   // 跳过打字机效果，立即显示全部
    void hide();
    void setShowBox(bool show) { m_showBox = show; }

    // 更新与渲染
    void update(float dt);
    void render();

    // 状态
    TextMode getMode() const { return m_mode; }
    bool isWaitingClick() const { return m_mode == TextMode::WAIT; }
    bool isFullyShown() const { return m_mode == TextMode::DONE || m_mode == TextMode::WAIT; }

    // 点击推进
    bool advance();  // 返回 true 表示文字已全部显示（下次点击应推进剧情）

private:
    Renderer* m_renderer;
    TTF_Font* m_font = nullptr;

    int m_x = 0, m_y = 0;
    int m_width = 800, m_height = 160;
    bool m_showBox = true;
    bool m_visible = false;

    SDL_Color m_backColor = {0, 0, 0, 200};
    SDL_Color m_textColor = {255, 255, 255, 255};
    SDL_Color m_nameColor = {255, 220, 100, 255};

    std::string m_name;
    std::string m_fullText;
    std::string m_displayedText;
    size_t m_displayedChars = 0;
    float m_charTimer = 0;
    float m_typingSpeed = 30.0f;  // 每秒字符数

    TextMode m_mode = TextMode::NORMAL;

    // 渲染缓存
    std::shared_ptr<Texture> m_textTexture;
    std::shared_ptr<Texture> m_nameTexture;
    bool m_textDirty = true;

    void updateTextTexture();
    size_t utf8CharLen(const std::string& str, size_t pos);
};
