#include "ending_list.h"
#include <algorithm>
#include <cmath>
#include <sstream>

EndingList::EndingList(Renderer* renderer) : m_renderer(renderer) {
}

EndingList::~EndingList() {
}

void EndingList::addEnding(const EndingEntry& ending) {
    m_endings.push_back(ending);
}

void EndingList::clearEndings() {
    m_endings.clear();
}

bool EndingList::unlockEnding(const std::string& id) {
    for (auto& e : m_endings) {
        if (e.id == id) {
            if (!e.unlocked) {
                e.unlocked = true;
                return true;
            }
            return false;
        }
    }
    return false;
}

bool EndingList::isUnlocked(const std::string& id) const {
    for (const auto& e : m_endings) {
        if (e.id == id) return e.unlocked;
    }
    return false;
}

int EndingList::getUnlockedCount() const {
    int count = 0;
    for (const auto& e : m_endings) {
        if (e.unlocked) ++count;
    }
    return count;
}

float EndingList::getCompletionRate() const {
    if (m_endings.empty()) return 0.0f;
    return static_cast<float>(getUnlockedCount()) / m_endings.size();
}

std::string EndingList::serialize() const {
    std::ostringstream oss;
    for (const auto& e : m_endings) {
        if (e.unlocked) {
            oss << e.id << "\n";
        }
    }
    return oss.str();
}

bool EndingList::deserialize(const std::string& data) {
    std::istringstream iss(data);
    std::string id;
    while (std::getline(iss, id)) {
        for (auto& e : m_endings) {
            if (e.id == id) e.unlocked = true;
        }
    }
    return true;
}

void EndingList::show() {
    m_visible = true;
    m_fadeAlpha = 0.0f;
    m_scrollY = 0.0f;
    m_targetScrollY = 0.0f;
    m_detailVisible = false;
}

void EndingList::hide() {
    m_visible = false;
    m_detailVisible = false;
}

void EndingList::updateScroll(float dt) {
    m_scrollY += (m_targetScrollY - m_scrollY) * std::min(dt * 10.0f, 1.0f);
}

SDL_Rect EndingList::getEndingRect(int index) const {
    int screenW = m_renderer->getWidth();
    int margin = 20;
    int cardW = 280;
    int cardH = 100;
    int perRow = std::max(1, (screenW - margin) / (cardW + margin));
    int row = index / perRow;
    int col = index % perRow;
    int totalW = perRow * cardW + (perRow - 1) * margin;
    int startX = (screenW - totalW) / 2;
    int x = startX + col * (cardW + margin);
    int y = 60 + row * (cardH + margin) - static_cast<int>(m_scrollY);
    return {x, y, cardW, cardH};
}

void EndingList::update(float dt, const Input& input) {
    if (!m_visible) return;

    m_fadeAlpha = std::min(m_fadeAlpha + dt * 8.0f, 1.0f);

    // 详情面板
    if (m_detailVisible) {
        int screenW = m_renderer->getWidth();
        int screenH = m_renderer->getHeight();
        SDL_Rect closeBtn = {screenW - 120, 20, 80, 30};

        if (input.isMouseButtonPressed(SDL_BUTTON_LEFT)) {
            int mx, my;
            input.getMousePosition(mx, my);
            if (mx >= closeBtn.x && mx < closeBtn.x + closeBtn.w &&
                my >= closeBtn.y && my < closeBtn.y + closeBtn.h) {
                m_detailVisible = false;
                return;
            }
        }
        if (input.isKeyPressed(SDL_SCANCODE_ESCAPE)) {
            m_detailVisible = false;
        }
        return;
    }

    // 滚动
    int wheel = input.getMouseWheelY();
    if (wheel != 0) m_targetScrollY -= wheel * 40;
    m_targetScrollY = std::max(0.0f, m_targetScrollY);
    updateScroll(dt);

    if (input.isKeyPressed(SDL_SCANCODE_ESCAPE)) {
        hide();
        return;
    }

    // hover
    m_hoverIndex = -1;
    int mx, my;
    input.getMousePosition(mx, my);
    for (int i = 0; i < static_cast<int>(m_endings.size()); ++i) {
        SDL_Rect r = getEndingRect(i);
        if (mx >= r.x && mx < r.x + r.w && my >= r.y && my < r.y + r.h) {
            m_hoverIndex = i;
            break;
        }
    }

    // 点击
    if (input.isMouseButtonPressed(SDL_BUTTON_LEFT) && m_hoverIndex >= 0) {
        if (m_endings[m_hoverIndex].unlocked) {
            m_detailVisible = true;
            m_detailIndex = m_hoverIndex;
        }
    }
}

void EndingList::render() {
    if (!m_visible) return;

    Uint8 alpha = static_cast<Uint8>(m_fadeAlpha * 255);
    int screenW = m_renderer->getWidth();
    int screenH = m_renderer->getHeight();

    m_renderer->drawRect(0, 0, screenW, screenH, {0, 0, 0, alpha});

    // 标题
    m_renderer->renderText("结 局 列 表", screenW/2 - 80, 15, {255, 220, 100, alpha}, m_font);

    // 统计
    std::string stats = std::to_string(getUnlockedCount()) + " / " +
                        std::to_string(getTotalCount()) + "  (" +
                        std::to_string(static_cast<int>(getCompletionRate() * 100)) + "%)";
    m_renderer->renderText(stats, screenW - 150, 18, {200, 200, 220, alpha}, m_smallFont);

    // 结局卡片
    for (int i = 0; i < static_cast<int>(m_endings.size()); ++i) {
        SDL_Rect r = getEndingRect(i);
        if (r.y + r.h < 50 || r.y > screenH) continue;
        renderEndingCard(m_endings[i], i, alpha);
    }

    if (m_detailVisible) {
        renderDetail(alpha);
    }
}

void EndingList::renderEndingCard(const EndingEntry& ending, int index, Uint8 alpha) {
    SDL_Rect r = getEndingRect(index);
    bool hover = (m_hoverIndex == index);

    // 根据优先级选颜色
    SDL_Color cardColor;
    if (!ending.unlocked) {
        cardColor = {20, 20, 25, static_cast<Uint8>(alpha * 0.6f)};
    } else if (ending.priority >= 10) {
        cardColor = {80, 60, 20, alpha};  // True End - 金色
    } else if (ending.priority >= 5) {
        cardColor = {20, 60, 30, alpha};  // Good End - 绿色
    } else {
        cardColor = {60, 20, 20, alpha};  // Normal/Bad End - 红色
    }

    if (hover) {
        cardColor.r = std::min(255, cardColor.r + 30);
        cardColor.g = std::min(255, cardColor.g + 30);
        cardColor.b = std::min(255, cardColor.b + 30);
    }

    m_renderer->drawRect(r.x, r.y, r.w, r.h, cardColor);

    // 边框
    SDL_Color border = hover ? SDL_Color{255, 220, 100, alpha} : SDL_Color{80, 80, 100, alpha};
    m_renderer->drawRect(r.x, r.y, r.w, 2, border);
    m_renderer->drawRect(r.x, r.y + r.h - 2, r.w, 2, border);
    m_renderer->drawRect(r.x, r.y, 2, r.h, border);
    m_renderer->drawRect(r.x + r.w - 2, r.y, 2, r.h, border);

    if (ending.unlocked) {
        // 标题
        m_renderer->renderText(ending.title, r.x + 10, r.y + 8, {255, 255, 255, alpha}, m_font);
        // 描述
        std::string desc = ending.description;
        if (desc.length() > 30) desc = desc.substr(0, 30) + "...";
        m_renderer->renderText(desc, r.x + 10, r.y + 35, {200, 200, 210, alpha}, m_smallFont);
        // 状态
        m_renderer->renderText("✓ 已达成", r.x + 10, r.y + 70, {100, 255, 100, alpha}, m_smallFont);
    } else {
        // 未解锁
        m_renderer->renderText("??? ", r.x + 10, r.y + 8, {100, 100, 110, alpha}, m_font);
        m_renderer->renderText(ending.unlockCondition, r.x + 10, r.y + 35,
                              {120, 120, 130, alpha}, m_smallFont);
        m_renderer->renderText("🔒 未达成", r.x + 10, r.y + 70, {150, 150, 160, alpha}, m_smallFont);
    }
}

void EndingList::renderDetail(Uint8 alpha) {
    if (m_detailIndex < 0 || m_detailIndex >= static_cast<int>(m_endings.size())) return;

    const auto& ending = m_endings[m_detailIndex];
    int screenW = m_renderer->getWidth();
    int screenH = m_renderer->getHeight();

    // 遮罩
    m_renderer->drawRect(0, 0, screenW, screenH, {0, 0, 0, 200});

    // 详情面板
    int panelW = 500, panelH = 300;
    int panelX = (screenW - panelW) / 2;
    int panelY = (screenH - panelH) / 2;

    m_renderer->drawRect(panelX, panelY, panelW, panelH, {30, 30, 40, 255});
    m_renderer->drawRect(panelX, panelY, panelW, 3, {255, 220, 100, 255});

    // 标题
    m_renderer->renderText(ending.title, panelX + 20, panelY + 15, {255, 220, 100, 255}, m_font);
    // 描述
    m_renderer->renderTextWrapped(ending.description, panelX + 20, panelY + 55,
                                  panelW - 40, {220, 220, 230, 255}, m_smallFont);
    // 解锁条件
    m_renderer->renderText("解锁条件: " + ending.unlockCondition, panelX + 20, panelY + 200,
                          {150, 150, 160, 255}, m_smallFont);

    // 关闭按钮
    SDL_Rect closeBtn = {screenW - 120, 20, 80, 30};
    m_renderer->drawRect(closeBtn.x, closeBtn.y, closeBtn.w, closeBtn.h, {80, 50, 50, 255});
    m_renderer->renderText("关闭", closeBtn.x + 20, closeBtn.y + 5, {255, 255, 255, 255}, m_font);
}
