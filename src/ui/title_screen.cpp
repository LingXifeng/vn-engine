#include "title_screen.h"
#include <algorithm>

TitleScreen::TitleScreen(Renderer* renderer, ResourceManager* resMgr)
    : m_renderer(renderer), m_resMgr(resMgr) {}

TitleScreen::~TitleScreen() {}

void TitleScreen::setTitleImage(const std::string& path) {
    m_titleImage = m_resMgr->getTexture(path);
}

void TitleScreen::addMenuItem(const std::string& text, std::function<void()> action, bool enabled) {
    MenuItem item;
    item.text = text;
    item.action = action;
    item.enabled = enabled;
    m_items.push_back(item);
    updateMenuLayout();
}

void TitleScreen::clearMenuItems() {
    m_items.clear();
    m_selectedIndex = -1;
}

void TitleScreen::show() {
    m_visible = true;
    m_fadeAlpha = 0.0f;
    m_targetAlpha = 1.0f;
    m_selectedIndex = -1;
    m_hoverIndex = -1;
}

void TitleScreen::hide() {
    m_visible = false;
    m_targetAlpha = 0.0f;
}

void TitleScreen::updateMenuLayout() {
    int screenWidth = m_renderer->getWidth();
    int screenHeight = m_renderer->getHeight();
    int totalHeight = m_items.size() * m_itemHeight + (m_items.size() - 1) * m_itemSpacing;
    m_menuX = screenWidth / 2 - 100;
    m_menuY = screenHeight / 2 - totalHeight / 2 + 50;

    for (size_t i = 0; i < m_items.size(); i++) {
        m_items[i].rect.x = m_menuX;
        m_items[i].rect.y = m_menuY + i * (m_itemHeight + m_itemSpacing);
        m_items[i].rect.w = 200;
        m_items[i].rect.h = m_itemHeight;
    }
}

void TitleScreen::update(float dt, const Input& input) {
    if (!m_visible) return;

    // 淡入动画
    float fadeSpeed = 3.0f;
    if (m_fadeAlpha < m_targetAlpha) {
        m_fadeAlpha = std::min(m_fadeAlpha + fadeSpeed * dt, m_targetAlpha);
    } else if (m_fadeAlpha > m_targetAlpha) {
        m_fadeAlpha = std::max(m_fadeAlpha - fadeSpeed * dt, m_targetAlpha);
    }

    // 鼠标悬停检测
    int mx, my;
    input.getMousePosition(mx, my);
    m_hoverIndex = -1;

    for (size_t i = 0; i < m_items.size(); i++) {
        if (!m_items[i].enabled) continue;
        if (input.isMouseInRect(m_items[i].rect.x, m_items[i].rect.y,
                                m_items[i].rect.w, m_items[i].rect.h)) {
            m_hoverIndex = i;
            m_items[i].hovered = true;
        } else {
            m_items[i].hovered = false;
        }
    }

    // 键盘导航
    if (input.isKeyPressed(SDL_SCANCODE_DOWN) || input.isKeyPressed(SDL_SCANCODE_S)) {
        m_hoverIndex = std::min(m_hoverIndex + 1, (int)m_items.size() - 1);
        while (m_hoverIndex >= 0 && m_hoverIndex < (int)m_items.size() && !m_items[m_hoverIndex].enabled)
            m_hoverIndex++;
        if (m_hoverIndex >= (int)m_items.size()) m_hoverIndex = -1;
    }
    if (input.isKeyPressed(SDL_SCANCODE_UP) || input.isKeyPressed(SDL_SCANCODE_W)) {
        m_hoverIndex = std::max(m_hoverIndex - 1, 0);
        while (m_hoverIndex >= 0 && !m_items[m_hoverIndex].enabled)
            m_hoverIndex--;
    }

    // 确认选择
    bool clicked = (m_hoverIndex >= 0 && input.isMouseButtonPressed(1)) ||
                   (m_hoverIndex >= 0 && input.isKeyPressed(SDL_SCANCODE_RETURN)) ||
                   (m_hoverIndex >= 0 && input.isKeyPressed(SDL_SCANCODE_SPACE));

    if (clicked && m_hoverIndex >= 0 && m_items[m_hoverIndex].enabled) {
        m_selectedIndex = m_hoverIndex;
        if (m_items[m_hoverIndex].action) {
            m_items[m_hoverIndex].action();
        }
    }
}

void TitleScreen::render() {
    if (!m_visible && m_fadeAlpha < 0.01f) return;

    Uint8 alpha = (Uint8)(m_fadeAlpha * 255);

    // 渲染标题图片
    if (m_titleImage) {
        m_renderer->drawTexture(m_titleImage.get(), 0, 0, 1.0f, 1.0f, alpha);
    } else {
        // 无图片时画半透明背景
        m_renderer->drawRect(0, 0, m_renderer->getWidth(), m_renderer->getHeight(),
                             {10, 10, 30, alpha}, true);
    }

    // 渲染菜单项
    if (!m_menuFont) return;

    for (size_t i = 0; i < m_items.size(); i++) {
        const auto& item = m_items[i];
        bool isHover = ((int)i == m_hoverIndex);

        // 悬停背景
        if (isHover && item.enabled) {
            m_renderer->drawRect(item.rect.x - 4, item.rect.y - 2,
                                 item.rect.w + 8, item.rect.h + 4,
                                 {60, 80, 120, (Uint8)(alpha * 0.6f)}, true);
        }

        // 文字
        SDL_Color color;
        if (!item.enabled) {
            color = {100, 100, 100, alpha};
        } else if (isHover) {
            color = {255, 220, 100, alpha};
        } else {
            color = {220, 220, 220, alpha};
        }

        auto tex = m_renderer->renderText(item.text, m_menuFont, color);
        if (tex) {
            int tx = item.rect.x + (item.rect.w - tex->width()) / 2;
            int ty = item.rect.y + (item.rect.h - tex->height()) / 2;
            m_renderer->drawTexture(tex.get(), tx, ty, 1.0f, 1.0f, alpha);
        }
    }
}
