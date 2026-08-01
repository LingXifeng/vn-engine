#include "glossary.h"
#include <algorithm>
#include <cmath>
#include <sstream>

Glossary::Glossary(Renderer* renderer) : m_renderer(renderer) {
}

Glossary::~Glossary() {
}

void Glossary::addEntry(const GlossaryEntry& entry) {
    m_entries.push_back(entry);
    rebuildIndex();
}

void Glossary::addEntry(const std::string& term, const std::string& description,
                        const std::string& category) {
    GlossaryEntry entry;
    entry.term = term;
    entry.description = description;
    entry.category = category;
    entry.unlocked = true;
    m_entries.push_back(entry);
    rebuildIndex();
}

void Glossary::clearEntries() {
    m_entries.clear();
    m_termIndex.clear();
    m_categories.clear();
}

bool Glossary::unlockEntry(const std::string& term) {
    auto it = m_termIndex.find(term);
    if (it == m_termIndex.end()) return false;
    if (!m_entries[it->second].unlocked) {
        m_entries[it->second].unlocked = true;
        return true;
    }
    return false;
}

bool Glossary::isUnlocked(const std::string& term) const {
    auto it = m_termIndex.find(term);
    if (it == m_termIndex.end()) return false;
    return m_entries[it->second].unlocked;
}

const GlossaryEntry* Glossary::findEntry(const std::string& term) const {
    auto it = m_termIndex.find(term);
    if (it == m_termIndex.end()) return nullptr;
    return &m_entries[it->second];
}

std::vector<std::pair<int, int>> Glossary::findTermsInText(const std::string& text) const {
    std::vector<std::pair<int, int>> result;
    for (const auto& entry : m_entries) {
        if (!entry.unlocked) continue;
        size_t pos = 0;
        while ((pos = text.find(entry.term, pos)) != std::string::npos) {
            result.emplace_back(static_cast<int>(pos), static_cast<int>(entry.term.length()));
            pos += entry.term.length();
        }
    }
    // 按位置排序
    std::sort(result.begin(), result.end());
    return result;
}

std::string Glossary::serialize() const {
    std::ostringstream oss;
    for (const auto& e : m_entries) {
        if (e.unlocked) oss << e.term << "\n";
    }
    return oss.str();
}

bool Glossary::deserialize(const std::string& data) {
    std::istringstream iss(data);
    std::string term;
    while (std::getline(iss, term)) {
        auto it = m_termIndex.find(term);
        if (it != m_termIndex.end()) {
            m_entries[it->second].unlocked = true;
        }
    }
    return true;
}

void Glossary::show() {
    m_visible = true;
    m_fadeAlpha = 0.0f;
    m_scrollY = 0.0f;
    m_targetScrollY = 0.0f;
    m_detailVisible = false;
    m_filterCategory.clear();
}

void Glossary::hide() {
    m_visible = false;
    m_detailVisible = false;
    m_popupVisible = false;
}

void Glossary::showTermDetail(const std::string& term) {
    auto it = m_termIndex.find(term);
    if (it != m_termIndex.end() && m_entries[it->second].unlocked) {
        m_popupVisible = true;
        m_popupIndex = it->second;
        m_popupTimer = 0.0f;
    }
}

int Glossary::getUnlockedCount() const {
    int count = 0;
    for (const auto& e : m_entries) {
        if (e.unlocked) ++count;
    }
    return count;
}

void Glossary::rebuildIndex() {
    m_termIndex.clear();
    m_categories.clear();
    for (int i = 0; i < static_cast<int>(m_entries.size()); ++i) {
        m_termIndex[m_entries[i].term] = i;
        bool found = false;
        for (const auto& cat : m_categories) {
            if (cat == m_entries[i].category) { found = true; break; }
        }
        if (!found && !m_entries[i].category.empty()) {
            m_categories.push_back(m_entries[i].category);
        }
    }
}

std::vector<const GlossaryEntry*> Glossary::getFilteredEntries() const {
    std::vector<const GlossaryEntry*> result;
    for (const auto& e : m_entries) {
        if (!e.unlocked) continue;
        if (!m_filterCategory.empty() && e.category != m_filterCategory) continue;
        result.push_back(&e);
    }
    return result;
}

void Glossary::updateScroll(float dt) {
    m_scrollY += (m_targetScrollY - m_scrollY) * std::min(dt * 10.0f, 1.0f);
}

void Glossary::update(float dt, const Input& input) {
    if (!m_visible) return;

    m_fadeAlpha = std::min(m_fadeAlpha + dt * 8.0f, 1.0f);

    // 弹出提示自动消失
    if (m_popupVisible) {
        m_popupTimer += dt;
        if (m_popupTimer > 5.0f || input.isMouseButtonPressed(SDL_BUTTON_LEFT)) {
            m_popupVisible = false;
        }
        return;
    }

    // 详情面板
    if (m_detailVisible) {
        int screenW = m_renderer->getWidth();
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
    auto filtered = getFilteredEntries();
    int entryH = 50;
    int listX = 20;
    int listW = m_renderer->getWidth() - 40;

    for (int i = 0; i < static_cast<int>(filtered.size()); ++i) {
        int y = 60 + i * entryH - static_cast<int>(m_scrollY);
        if (mx >= listX && mx < listX + listW && my >= y && my < y + entryH) {
            m_hoverIndex = i;
            break;
        }
    }

    // 点击
    if (input.isMouseButtonPressed(SDL_BUTTON_LEFT) && m_hoverIndex >= 0) {
        m_detailVisible = true;
        m_detailIndex = m_hoverIndex;
    }
}

void Glossary::render() {
    if (!m_visible) return;

    Uint8 alpha = static_cast<Uint8>(m_fadeAlpha * 255);
    int screenW = m_renderer->getWidth();
    int screenH = m_renderer->getHeight();

    m_renderer->drawRect(0, 0, screenW, screenH, {0, 0, 0, alpha});

    // 标题
    m_renderer->renderText("词 典", screenW/2 - 50, 15, {255, 220, 100, alpha}, m_font);

    // 统计
    std::string stats = std::to_string(getUnlockedCount()) + " / " + std::to_string(getTotalCount());
    m_renderer->renderText(stats, screenW - 100, 18, {200, 200, 220, alpha}, m_smallFont);

    // 分类标签
    std::string catLabel = m_filterCategory.empty() ? "全部" : m_filterCategory;
    m_renderer->renderText("分类: " + catLabel, 20, 40, {180, 180, 200, alpha}, m_smallFont);

    // 条目列表
    auto filtered = getFilteredEntries();
    int entryH = 50;
    int listX = 20;
    int listW = screenW - 40;

    for (int i = 0; i < static_cast<int>(filtered.size()); ++i) {
        int y = 60 + i * entryH - static_cast<int>(m_scrollY);
        if (y + entryH < 55 || y > screenH) continue;
        renderEntry(*filtered[i], i, static_cast<float>(y), alpha);
    }

    if (m_detailVisible && m_detailIndex >= 0 && m_detailIndex < static_cast<int>(filtered.size())) {
        renderDetail(alpha);
    }

    if (m_popupVisible) {
        renderPopup(alpha);
    }
}

void Glossary::renderEntry(const GlossaryEntry& entry, int index, float y, Uint8 alpha) {
    int screenW = m_renderer->getWidth();
    bool hover = (m_hoverIndex == index);
    int entryH = 50;
    int listX = 20;
    int listW = screenW - 40;

    SDL_Color bg = hover ? SDL_Color{40, 40, 60, static_cast<Uint8>(alpha * 0.7f)}
                         : SDL_Color{20, 20, 30, static_cast<Uint8>(alpha * 0.5f)};
    m_renderer->drawRect(listX, static_cast<int>(y), listW, entryH - 5, bg);

    // 术语
    m_renderer->renderText(entry.term, listX + 10, static_cast<int>(y) + 5,
                          {255, 220, 100, alpha}, m_font);
    // 读音
    if (!entry.reading.empty()) {
        m_renderer->renderText("[" + entry.reading + "]", listX + 10 + entry.term.length() * 20,
                              static_cast<int>(y) + 8, {150, 150, 160, alpha}, m_smallFont);
    }
    // 分类
    if (!entry.category.empty()) {
        m_renderer->renderText("[" + entry.category + "]", screenW - 150,
                              static_cast<int>(y) + 5, {120, 120, 140, alpha}, m_smallFont);
    }
    // 描述（截断）
    std::string desc = entry.description;
    if (desc.length() > 40) desc = desc.substr(0, 40) + "...";
    m_renderer->renderText(desc, listX + 10, static_cast<int>(y) + 28,
                          {200, 200, 210, alpha}, m_smallFont);
}

void Glossary::renderDetail(Uint8 alpha) {
    auto filtered = getFilteredEntries();
    if (m_detailIndex < 0 || m_detailIndex >= static_cast<int>(filtered.size())) return;

    const auto& entry = *filtered[m_detailIndex];
    int screenW = m_renderer->getWidth();
    int screenH = m_renderer->getHeight();

    m_renderer->drawRect(0, 0, screenW, screenH, {0, 0, 0, 200});

    int panelW = 500, panelH = 250;
    int panelX = (screenW - panelW) / 2;
    int panelY = (screenH - panelH) / 2;

    m_renderer->drawRect(panelX, panelY, panelW, panelH, {30, 30, 40, 255});
    m_renderer->drawRect(panelX, panelY, panelW, 3, {255, 220, 100, 255});

    // 术语
    m_renderer->renderText(entry.term, panelX + 20, panelY + 15, {255, 220, 100, 255}, m_font);
    // 读音
    if (!entry.reading.empty()) {
        m_renderer->renderText("[" + entry.reading + "]", panelX + 20, panelY + 50,
                              {180, 180, 200, 255}, m_smallFont);
    }
    // 分类
    if (!entry.category.empty()) {
        m_renderer->renderText("分类: " + entry.category, panelX + panelW - 150, panelY + 15,
                              {150, 150, 160, 255}, m_smallFont);
    }
    // 描述
    m_renderer->renderTextWrapped(entry.description, panelX + 20, panelY + 80,
                                  panelW - 40, {220, 220, 230, 255}, m_smallFont);

    // 关闭按钮
    SDL_Rect closeBtn = {screenW - 120, 20, 80, 30};
    m_renderer->drawRect(closeBtn.x, closeBtn.y, closeBtn.w, closeBtn.h, {80, 50, 50, 255});
    m_renderer->renderText("关闭", closeBtn.x + 20, closeBtn.y + 5, {255, 255, 255, 255}, m_font);
}

void Glossary::renderPopup(Uint8 alpha) {
    if (m_popupIndex < 0 || m_popupIndex >= static_cast<int>(m_entries.size())) return;

    const auto& entry = m_entries[m_popupIndex];
    int screenW = m_renderer->getWidth();

    // 弹出框（底部）
    int popupW = 400, popupH = 100;
    int popupX = (screenW - popupW) / 2;
    int popupY = m_renderer->getHeight() - popupH - 20;

    // 淡出效果
    float fade = 1.0f;
    if (m_popupTimer > 4.0f) {
        fade = 1.0f - (m_popupTimer - 4.0f);
    }
    Uint8 a = static_cast<Uint8>(alpha * fade);

    m_renderer->drawRect(popupX, popupY, popupW, popupH, {20, 20, 30, a});
    m_renderer->drawRect(popupX, popupY, popupW, 2, {255, 220, 100, a});

    m_renderer->renderText(entry.term, popupX + 15, popupY + 10, {255, 220, 100, a}, m_font);
    std::string desc = entry.description;
    if (desc.length() > 45) desc = desc.substr(0, 45) + "...";
    m_renderer->renderTextWrapped(desc, popupX + 15, popupY + 40, popupW - 30,
                                 {220, 220, 230, a}, m_smallFont);
}
