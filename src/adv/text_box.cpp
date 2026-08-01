#include "text_box.h"
#include <iostream>

TextBox::TextBox(Renderer* renderer) : m_renderer(renderer) {}
TextBox::~TextBox() {}

size_t TextBox::utf8CharLen(const std::string& str, size_t pos) {
    if (pos >= str.size()) return 0;
    char c = str[pos];
    if ((c & 0x80) == 0) return 1;
    if ((c & 0xE0) == 0xC0) return 2;
    if ((c & 0xF0) == 0xE0) return 3;
    if ((c & 0xF8) == 0xF0) return 4;
    return 1;
}

void TextBox::show(const std::string& name, const std::string& text) {
    m_name = name;
    m_fullText = text;
    m_displayedText.clear();
    m_displayedChars = 0;
    m_charTimer = 0;
    m_mode = TextMode::TYPING;
    m_visible = true;
    m_textDirty = true;
    updateTextTexture();
}

void TextBox::append(const std::string& text) {
    m_fullText += text;
    m_mode = TextMode::TYPING;
    m_textDirty = true;
}

void TextBox::clear() {
    m_name.clear();
    m_fullText.clear();
    m_displayedText.clear();
    m_displayedChars = 0;
    m_mode = TextMode::NORMAL;
    m_textTexture = nullptr;
    m_nameTexture = nullptr;
    m_textDirty = true;
}

void TextBox::skip() {
    if (m_mode == TextMode::TYPING) {
        m_displayedText = m_fullText;
        m_displayedChars = m_fullText.size();
        m_mode = TextMode::WAIT;
        m_textDirty = true;
        updateTextTexture();
    }
}

void TextBox::hide() {
    m_visible = false;
    clear();
}

bool TextBox::advance() {
    if (m_mode == TextMode::TYPING) {
        skip();
        return false;  // 本次点击只是跳过打字机
    }
    return true;  // 文字已全部显示，下次点击推进剧情
}

void TextBox::update(float dt) {
    if (m_mode == TextMode::TYPING) {
        m_charTimer += dt;
        float charInterval = 1.0f / m_typingSpeed;
        while (m_charTimer >= charInterval && m_displayedChars < m_fullText.size()) {
            m_charTimer -= charInterval;
            size_t charLen = utf8CharLen(m_fullText, m_displayedChars);
            if (charLen == 0) break;
            m_displayedText += m_fullText.substr(m_displayedChars, charLen);
            m_displayedChars += charLen;
            m_textDirty = true;
        }
        if (m_displayedChars >= m_fullText.size()) {
            m_mode = TextMode::WAIT;
        }
    }
}

void TextBox::updateTextTexture() {
    if (!m_font) return;
    if (!m_name.empty()) {
        m_nameTexture = m_renderer->renderText(m_name, m_font, m_nameColor);
    } else {
        m_nameTexture = nullptr;
    }
    if (!m_displayedText.empty()) {
        m_textTexture = m_renderer->renderTextWrapped(m_displayedText, m_font,
                                                       m_width - 40, 4, m_textColor);
    } else {
        m_textTexture = nullptr;
    }
    m_textDirty = false;
}

void TextBox::render() {
    if (!m_visible) return;

    // 绘制背景框
    if (m_showBox) {
        m_renderer->drawRect(m_x, m_y, m_width, m_height, m_backColor, true);
        // 边框
        SDL_Color borderColor = {100, 100, 120, 255};
        m_renderer->drawRect(m_x, m_y, m_width, m_height, borderColor, false);
    }

    // 更新文字纹理
    if (m_textDirty) {
        updateTextTexture();
    }

    // 绘制角色名
    if (m_nameTexture) {
        m_renderer->drawTexture(m_nameTexture.get(), m_x + 20, m_y - 30);
    }

    // 绘制正文
    if (m_textTexture) {
        m_renderer->drawTexture(m_textTexture.get(), m_x + 20, m_y + 20);
    }

    // 等待点击标记
    if (m_mode == TextMode::WAIT) {
        // 右下角小三角形
        int triX = m_x + m_width - 25;
        int triY = m_y + m_height - 20;
        SDL_Color triColor = {200, 200, 200, 255};
        m_renderer->drawLine(triX, triY, triX + 10, triY, triColor);
        m_renderer->drawLine(triX, triY, triX + 5, triY + 8, triColor);
        m_renderer->drawLine(triX + 10, triY, triX + 5, triY + 8, triColor);
    }
}
