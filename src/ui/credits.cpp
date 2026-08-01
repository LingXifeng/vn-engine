#include "credits.h"
#include <fstream>
#include <algorithm>

Credits::Credits(Renderer* renderer) : m_renderer(renderer) {}
Credits::~Credits() {}

void Credits::addTitle(const std::string& text, SDL_Color color) {
    CreditSection s;
    s.type = CreditSectionType::TITLE;
    s.text = text;
    s.color = color;
    s.fontSize = 48;
    s.spacingAfter = 20;
    m_sections.push_back(s);
    m_cacheDirty = true;
}

void Credits::addHeading(const std::string& text, SDL_Color color) {
    CreditSection s;
    s.type = CreditSectionType::HEADING;
    s.text = text;
    s.color = color;
    s.fontSize = 32;
    s.spacingAfter = 15;
    m_sections.push_back(s);
    m_cacheDirty = true;
}

void Credits::addName(const std::string& text, SDL_Color color) {
    CreditSection s;
    s.type = CreditSectionType::NAME;
    s.text = text;
    s.color = color;
    s.fontSize = 24;
    s.spacingAfter = 8;
    m_sections.push_back(s);
    m_cacheDirty = true;
}

void Credits::addSmall(const std::string& text, SDL_Color color) {
    CreditSection s;
    s.type = CreditSectionType::SMALL;
    s.text = text;
    s.color = color;
    s.fontSize = 16;
    s.spacingAfter = 5;
    m_sections.push_back(s);
    m_cacheDirty = true;
}

void Credits::addBlank(int spacing) {
    CreditSection s;
    s.type = CreditSectionType::BLANK;
    s.spacingAfter = spacing;
    m_sections.push_back(s);
    m_cacheDirty = true;
}

bool Credits::loadFromFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) return false;

    m_sections.clear();
    std::string line;
    while (std::getline(file, line)) {
        if (line.empty()) { addBlank(20); continue; }
        if (line[0] == '#') { addTitle(line.substr(1)); continue; }
        if (line[0] == '@') { addHeading(line.substr(1)); continue; }
        if (line[0] == '%') { addSmall(line.substr(1)); continue; }
        addName(line);
    }
    m_cacheDirty = true;
    return true;
}

TTF_Font* Credits::getFontForType(CreditSectionType type) const {
    switch (type) {
        case CreditSectionType::TITLE: return m_titleFont ? m_titleFont : m_bodyFont;
        case CreditSectionType::HEADING: return m_headingFont ? m_headingFont : m_bodyFont;
        case CreditSectionType::SMALL: return m_smallFont ? m_smallFont : m_bodyFont;
        default: return m_bodyFont;
    }
}

void Credits::rebuildCache() {
    m_cached.clear();
    m_totalHeight = 0;

    int sw = m_renderer->getWidth();

    for (const auto& sec : m_sections) {
        CachedSection cached;
        cached.type = sec.type;
        cached.spacingAfter = sec.spacingAfter;

        if (sec.type == CreditSectionType::BLANK || sec.text.empty()) {
            cached.texture = nullptr;
            cached.height = 0;
        } else {
            TTF_Font* font = getFontForType(sec.type);
            if (font) {
                cached.texture = m_renderer->renderText(sec.text, font, sec.color);
                cached.height = cached.texture ? cached.texture->height() : sec.fontSize;
            } else {
                cached.texture = nullptr;
                cached.height = sec.fontSize;
            }
        }

        m_cached.push_back(cached);
        m_totalHeight += cached.height + cached.spacingAfter;
    }

    m_cacheDirty = false;
}

void Credits::show() {
    m_visible = true;
    m_finished = false;
    reset();
}

void Credits::hide() {
    m_visible = false;
    m_finished = false;
}

void Credits::reset() {
    m_scrollY = 0.0f;
    if (m_cacheDirty) rebuildCache();
}

void Credits::update(float dt, const Input& input) {
    if (!m_visible) return;

    if (m_cacheDirty) rebuildCache();

    int sh = m_renderer->getHeight();

    // 滚动
    m_scrollY += m_scrollSpeed * dt;

    // 检查是否结束
    if (m_scrollY > m_totalHeight + sh) {
        if (m_loop) {
            m_scrollY = 0;
        } else {
            m_finished = true;
            m_visible = false;
        }
    }

    // 按键加速或跳过
    if (input.isKeyDown(SDL_SCANCODE_DOWN) || input.isKeyDown(SDL_SCANCODE_SPACE)) {
        m_scrollY += m_scrollSpeed * dt * 3;
    }
    if (input.isKeyPressed(SDL_SCANCODE_ESCAPE)) {
        m_finished = true;
        m_visible = false;
    }
}

void Credits::render() {
    if (!m_visible) return;

    int sw = m_renderer->getWidth();
    int sh = m_renderer->getHeight();

    // 背景
    m_renderer->drawRect(0, 0, sw, sh, {0, 0, 0, 255}, true);

    // 计算渲染位置
    // 内容从屏幕底部开始，向上滚动
    float y = sh - m_scrollY;

    for (size_t i = 0; i < m_cached.size(); i++) {
        const auto& cached = m_cached[i];

        if (cached.texture) {
            int texY = (int)y;

            // 渐入渐出效果
            float alpha = 1.0f;
            if (texY < m_fadeDistance) {
                alpha = (float)texY / m_fadeDistance;
            } else if (texY > sh - m_fadeDistance) {
                alpha = (float)(sh - texY) / m_fadeDistance;
            }
            alpha = std::clamp(alpha, 0.0f, 1.0f);

            if (alpha > 0.01f) {
                int texX = (sw - cached.texture->width()) / 2;
                Uint8 a = (Uint8)(alpha * 255);
                m_renderer->drawTexture(cached.texture.get(), texX, texY, 1.0f, 1.0f, a);
            }
        }

        y += cached.height + cached.spacingAfter;

        // 超出屏幕则停止
        if (y > sh + 100) break;
    }
}
