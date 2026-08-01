#include "config_screen.h"
#include <algorithm>

ConfigScreen::ConfigScreen(Renderer* renderer, Audio* audio)
    : m_renderer(renderer), m_audio(audio) {}

ConfigScreen::~ConfigScreen() {}

void ConfigScreen::addSlider(const std::string& label, int min, int max, int current, std::function<void(int)> onChange) {
    ConfigItem item;
    item.label = label;
    item.type = ConfigItemType::SLIDER;
    item.minVal = min;
    item.maxVal = max;
    item.currentVal = current;
    item.onChange = onChange;
    m_items.push_back(item);
    updateLayout();
}

void ConfigScreen::addToggle(const std::string& label, bool current, std::function<void(int)> onChange) {
    ConfigItem item;
    item.label = label;
    item.type = ConfigItemType::TOGGLE;
    item.minVal = 0;
    item.maxVal = 1;
    item.currentVal = current ? 1 : 0;
    item.onChange = onChange;
    m_items.push_back(item);
    updateLayout();
}

void ConfigScreen::addButton(const std::string& label, std::function<void(int)> action) {
    ConfigItem item;
    item.label = label;
    item.type = ConfigItemType::BUTTON;
    item.currentVal = 0;
    item.onChange = action;
    m_items.push_back(item);
    updateLayout();
}

void ConfigScreen::show() {
    m_visible = true;
    m_fadeAlpha = 0.0f;
    m_selectedIndex = -1;
    m_hoverIndex = -1;
}

void ConfigScreen::hide() {
    m_visible = false;
}

void ConfigScreen::updateLayout() {
    int sw = m_renderer->getWidth();
    int sh = m_renderer->getHeight();
    m_panelX = (sw - m_panelW) / 2;
    m_panelY = (sh - m_panelH) / 2;

    int startY = m_panelY + 60;
    for (size_t i = 0; i < m_items.size(); i++) {
        m_items[i].rect.x = m_panelX + 40;
        m_items[i].rect.y = startY + i * (m_itemHeight + m_itemSpacing);
        m_items[i].rect.w = m_panelW - 80;
        m_items[i].rect.h = m_itemHeight;
    }
}

int ConfigScreen::getSliderValue(int itemIndex, int mouseX) {
    auto& item = m_items[itemIndex];
    int sliderX = item.rect.x + 150;
    int sliderW = item.rect.w - 170;
    float ratio = (float)(mouseX - sliderX) / sliderW;
    ratio = std::clamp(ratio, 0.0f, 1.0f);
    return item.minVal + (int)(ratio * (item.maxVal - item.minVal));
}

void ConfigScreen::update(float dt, const Input& input) {
    if (!m_visible) return;

    m_fadeAlpha = std::min(m_fadeAlpha + 3.0f * dt, 1.0f);

    int mx, my;
    input.getMousePosition(mx, my);

    // 拖拽滑块
    if (m_dragging && m_hoverIndex >= 0 && m_items[m_hoverIndex].type == ConfigItemType::SLIDER) {
        int newVal = getSliderValue(m_hoverIndex, mx);
        if (newVal != m_items[m_hoverIndex].currentVal) {
            m_items[m_hoverIndex].currentVal = newVal;
            if (m_items[m_hoverIndex].onChange) m_items[m_hoverIndex].onChange(newVal);
        }
        if (input.isMouseButtonReleased(1)) m_dragging = false;
        return;
    }

    // 悬停检测
    m_hoverIndex = -1;
    for (size_t i = 0; i < m_items.size(); i++) {
        if (input.isMouseInRect(m_items[i].rect.x, m_items[i].rect.y, m_items[i].rect.w, m_items[i].rect.h)) {
            m_hoverIndex = i;
            m_items[i].hovered = true;
        } else {
            m_items[i].hovered = false;
        }
    }

    // 点击处理
    if (input.isMouseButtonPressed(1) && m_hoverIndex >= 0) {
        auto& item = m_items[m_hoverIndex];
        switch (item.type) {
            case ConfigItemType::SLIDER:
                m_dragging = true;
                item.currentVal = getSliderValue(m_hoverIndex, mx);
                if (item.onChange) item.onChange(item.currentVal);
                break;
            case ConfigItemType::TOGGLE:
                item.currentVal = !item.currentVal;
                if (item.onChange) item.onChange(item.currentVal);
                break;
            case ConfigItemType::BUTTON:
                m_selectedIndex = m_hoverIndex;
                if (item.onChange) item.onChange(0);
                break;
        }
    }

    // 键盘左右调整
    if (m_hoverIndex >= 0 && m_items[m_hoverIndex].type == ConfigItemType::SLIDER) {
        if (input.isKeyPressed(SDL_SCANCODE_LEFT)) {
            m_items[m_hoverIndex].currentVal = std::max(m_items[m_hoverIndex].currentVal - 5, m_items[m_hoverIndex].minVal);
            if (m_items[m_hoverIndex].onChange) m_items[m_hoverIndex].onChange(m_items[m_hoverIndex].currentVal);
        }
        if (input.isKeyPressed(SDL_SCANCODE_RIGHT)) {
            m_items[m_hoverIndex].currentVal = std::min(m_items[m_hoverIndex].currentVal + 5, m_items[m_hoverIndex].maxVal);
            if (m_items[m_hoverIndex].onChange) m_items[m_hoverIndex].onChange(m_items[m_hoverIndex].currentVal);
        }
    }
}

void ConfigScreen::render() {
    if (!m_visible) return;

    Uint8 alpha = (Uint8)(m_fadeAlpha * 255);

    // 半透明遮罩
    m_renderer->drawRect(0, 0, m_renderer->getWidth(), m_renderer->getHeight(),
                         {0, 0, 0, (Uint8)(alpha * 0.5f)}, true);

    // 配置面板背景
    m_renderer->drawRect(m_panelX, m_panelY, m_panelW, m_panelH,
                         {30, 35, 50, alpha}, true);
    m_renderer->drawRect(m_panelX, m_panelY, m_panelW, m_panelH,
                         {100, 120, 160, alpha}, false);

    if (!m_font) return;

    // 标题
    auto titleTex = m_renderer->renderText("Config", m_font, {255, 220, 100, alpha});
    if (titleTex) {
        m_renderer->drawTexture(titleTex.get(),
                                m_panelX + (m_panelW - titleTex->width()) / 2, m_panelY + 15,
                                1.0f, 1.0f, alpha);
    }

    // 配置项
    for (size_t i = 0; i < m_items.size(); i++) {
        const auto& item = m_items[i];
        bool isHover = ((int)i == m_hoverIndex);
        SDL_Color labelColor = isHover ? SDL_Color{255, 220, 100, alpha} : SDL_Color{220, 220, 220, alpha};

        // 标签
        auto labelTex = m_renderer->renderText(item.label, m_font, labelColor);
        if (labelTex) {
            m_renderer->drawTexture(labelTex.get(), item.rect.x, item.rect.y + 10, 1.0f, 1.0f, alpha);
        }

        int sliderX = item.rect.x + 150;
        int sliderW = item.rect.w - 170;
        int sliderY = item.rect.y + item.rect.h / 2;

        switch (item.type) {
            case ConfigItemType::SLIDER: {
                // 滑块轨道
                m_renderer->drawRect(sliderX, sliderY - 4, sliderW, 8, {60, 60, 70, alpha}, true);
                // 已填充部分
                float ratio = (float)(item.currentVal - item.minVal) / (item.maxVal - item.minVal);
                int fillW = (int)(sliderW * ratio);
                m_renderer->drawRect(sliderX, sliderY - 4, fillW, 8, {80, 140, 200, alpha}, true);
                // 手柄
                int handleX = sliderX + fillW - 6;
                m_renderer->drawRect(handleX, sliderY - 10, 12, 20, {200, 220, 255, alpha}, true);
                // 数值
                auto valTex = m_renderer->renderText(std::to_string(item.currentVal), m_font, labelColor);
                if (valTex) {
                    m_renderer->drawTexture(valTex.get(), item.rect.x + item.rect.w - 40, item.rect.y + 10, 1.0f, 1.0f, alpha);
                }
                break;
            }
            case ConfigItemType::TOGGLE: {
                int boxX = sliderX;
                int boxY = sliderY - 12;
                m_renderer->drawRect(boxX, boxY, 50, 24, {60, 60, 70, alpha}, true);
                if (item.currentVal) {
                    m_renderer->drawRect(boxX + 2, boxY + 2, 46, 20, {80, 200, 120, alpha}, true);
                    auto onTex = m_renderer->renderText("ON", m_font, {255, 255, 255, alpha});
                    if (onTex) m_renderer->drawTexture(onTex.get(), boxX + 10, boxY, 1.0f, 1.0f, alpha);
                } else {
                    auto offTex = m_renderer->renderText("OFF", m_font, {180, 180, 180, alpha});
                    if (offTex) m_renderer->drawTexture(offTex.get(), boxX + 8, boxY, 1.0f, 1.0f, alpha);
                }
                break;
            }
            case ConfigItemType::BUTTON: {
                int btnX = sliderX;
                int btnY = sliderY - 16;
                SDL_Color btnBg = isHover ? SDL_Color{80, 100, 140, alpha} : SDL_Color{50, 60, 80, alpha};
                m_renderer->drawRect(btnX, btnY, 100, 32, btnBg, true);
                auto btnTex = m_renderer->renderText(item.label, m_font, labelColor);
                if (btnTex) {
                    m_renderer->drawTexture(btnTex.get(), btnX + (100 - btnTex->width()) / 2, btnY + 4, 1.0f, 1.0f, alpha);
                }
                break;
            }
        }
    }
}
