#include "scene_replay.h"
#include <algorithm>
#include <cmath>
#include <sstream>

SceneReplay::SceneReplay(Renderer* renderer) : m_renderer(renderer) {
}

SceneReplay::~SceneReplay() {
}

void SceneReplay::addScene(const SceneReplayEntry& scene) {
    m_scenes.push_back(scene);
    // 收集章节
    if (!scene.chapter.empty()) {
        bool found = false;
        for (const auto& ch : m_chapters) {
            if (ch == scene.chapter) { found = true; break; }
        }
        if (!found) m_chapters.push_back(scene.chapter);
    }
}

void SceneReplay::clearScenes() {
    m_scenes.clear();
    m_chapters.clear();
}

bool SceneReplay::unlockScene(const std::string& id) {
    for (auto& s : m_scenes) {
        if (s.id == id) {
            if (!s.unlocked) {
                s.unlocked = true;
                return true;
            }
            return false;
        }
    }
    return false;
}

bool SceneReplay::isUnlocked(const std::string& id) const {
    for (const auto& s : m_scenes) {
        if (s.id == id) return s.unlocked;
    }
    return false;
}

std::string SceneReplay::serialize() const {
    std::ostringstream oss;
    for (const auto& s : m_scenes) {
        if (s.unlocked) oss << s.id << "\n";
    }
    return oss.str();
}

bool SceneReplay::deserialize(const std::string& data) {
    std::istringstream iss(data);
    std::string id;
    while (std::getline(iss, id)) {
        for (auto& s : m_scenes) {
            if (s.id == id) s.unlocked = true;
        }
    }
    return true;
}

void SceneReplay::show() {
    m_visible = true;
    m_fadeAlpha = 0.0f;
    m_scrollY = 0.0f;
    m_targetScrollY = 0.0f;
    m_confirmVisible = false;
    m_filterChapter.clear();
}

void SceneReplay::hide() {
    m_visible = false;
    m_confirmVisible = false;
}

std::vector<SceneReplayEntry> SceneReplay::getFilteredScenes() const {
    if (m_filterChapter.empty()) return m_scenes;
    std::vector<SceneReplayEntry> result;
    for (const auto& s : m_scenes) {
        if (s.chapter == m_filterChapter) result.push_back(s);
    }
    return result;
}

void SceneReplay::updateScroll(float dt) {
    m_scrollY += (m_targetScrollY - m_scrollY) * std::min(dt * 10.0f, 1.0f);
}

SDL_Rect SceneReplay::getSceneRect(int index) const {
    int screenW = m_renderer->getWidth();
    int margin = 20;
    int cardW = 350;
    int cardH = 80;
    int perRow = std::max(1, (screenW - margin) / (cardW + margin));
    int row = index / perRow;
    int col = index % perRow;
    int totalW = perRow * cardW + (perRow - 1) * margin;
    int startX = (screenW - totalW) / 2;
    int x = startX + col * (cardW + margin);
    int y = 100 + row * (cardH + margin) - static_cast<int>(m_scrollY);
    return {x, y, cardW, cardH};
}

void SceneReplay::update(float dt, const Input& input) {
    if (!m_visible) return;

    m_fadeAlpha = std::min(m_fadeAlpha + dt * 8.0f, 1.0f);

    // 确认对话框
    if (m_confirmVisible) {
        int screenW = m_renderer->getWidth();
        int screenH = m_renderer->getHeight();
        SDL_Rect yesBtn = {screenW/2 - 120, screenH/2 + 20, 100, 40};
        SDL_Rect noBtn  = {screenW/2 + 20, screenH/2 + 20, 100, 40};

        if (input.isMouseButtonPressed(SDL_BUTTON_LEFT)) {
            int mx, my;
            input.getMousePosition(mx, my);
            if (mx >= yesBtn.x && mx < yesBtn.x + yesBtn.w &&
                my >= yesBtn.y && my < yesBtn.y + yesBtn.h) {
                if (m_confirmIndex >= 0 && m_confirmIndex < static_cast<int>(m_scenes.size())) {
                    const auto& scene = m_scenes[m_confirmIndex];
                    if (m_replayCallback) {
                        m_replayCallback(scene.scriptName, scene.startLine, scene.endLine);
                    }
                }
                m_confirmVisible = false;
                hide();
            } else if (mx >= noBtn.x && mx < noBtn.x + noBtn.w &&
                       my >= noBtn.y && my < noBtn.y + noBtn.h) {
                m_confirmVisible = false;
            }
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

    // 章节切换（左右箭头）
    if (input.isKeyPressed(SDL_SCANCODE_LEFT) || input.isKeyPressed(SDL_SCANCODE_RIGHT)) {
        // 循环切换章节
        if (!m_chapters.empty()) {
            int currentCh = -1;
            for (int i = 0; i < static_cast<int>(m_chapters.size()); ++i) {
                if (m_chapters[i] == m_filterChapter) { currentCh = i; break; }
            }
            if (input.isKeyPressed(SDL_SCANCODE_RIGHT)) {
                currentCh = (currentCh + 1) % (static_cast<int>(m_chapters.size()) + 1);
            } else {
                currentCh = (currentCh - 1 + static_cast<int>(m_chapters.size()) + 1) % (static_cast<int>(m_chapters.size()) + 1);
            }
            m_filterChapter = (currentCh == static_cast<int>(m_chapters.size())) ? "" : m_chapters[currentCh];
            m_scrollY = 0;
            m_targetScrollY = 0;
        }
    }

    // hover
    m_hoverIndex = -1;
    int mx, my;
    input.getMousePosition(mx, my);
    auto filtered = getFilteredScenes();
    for (int i = 0; i < static_cast<int>(filtered.size()); ++i) {
        SDL_Rect r = getSceneRect(i);
        if (mx >= r.x && mx < r.x + r.w && my >= r.y && my < r.y + r.h) {
            m_hoverIndex = i;
            break;
        }
    }

    // 点击
    if (input.isMouseButtonPressed(SDL_BUTTON_LEFT) && m_hoverIndex >= 0) {
        if (filtered[m_hoverIndex].unlocked) {
            // 找到原始索引
            for (int i = 0; i < static_cast<int>(m_scenes.size()); ++i) {
                if (m_scenes[i].id == filtered[m_hoverIndex].id) {
                    m_confirmVisible = true;
                    m_confirmIndex = i;
                    return;
                }
            }
        }
    }
}

void SceneReplay::render() {
    if (!m_visible) return;

    Uint8 alpha = static_cast<Uint8>(m_fadeAlpha * 255);
    int screenW = m_renderer->getWidth();
    int screenH = m_renderer->getHeight();

    m_renderer->drawRect(0, 0, screenW, screenH, {0, 0, 0, alpha});

    // 标题
    m_renderer->renderText("场 景 回 想", screenW/2 - 80, 15, {255, 220, 100, alpha}, m_font);

    // 章节标签
    std::string chLabel = m_filterChapter.empty() ? "全部" : m_filterChapter;
    m_renderer->renderText("章节: " + chLabel + "  (←→切换)", 20, 50,
                          {200, 200, 220, alpha}, m_smallFont);

    // 场景卡片
    auto filtered = getFilteredScenes();
    for (int i = 0; i < static_cast<int>(filtered.size()); ++i) {
        SDL_Rect r = getSceneRect(i);
        if (r.y + r.h < 80 || r.y > screenH) continue;
        renderSceneCard(filtered[i], i, alpha);
    }

    if (m_confirmVisible) {
        renderConfirm(alpha);
    }
}

void SceneReplay::renderSceneCard(const SceneReplayEntry& scene, int index, Uint8 alpha) {
    SDL_Rect r = getSceneRect(index);
    bool hover = (m_hoverIndex == index);

    SDL_Color bg = scene.unlocked
        ? (hover ? SDL_Color{40, 40, 60, alpha} : SDL_Color{25, 25, 35, alpha})
        : SDL_Color{15, 15, 20, static_cast<Uint8>(alpha * 0.5f)};

    m_renderer->drawRect(r.x, r.y, r.w, r.h, bg);

    SDL_Color border = hover ? SDL_Color{255, 220, 100, alpha} : SDL_Color{80, 80, 100, alpha};
    m_renderer->drawRect(r.x, r.y, r.w, 2, border);
    m_renderer->drawRect(r.x, r.y + r.h - 2, r.w, 2, border);
    m_renderer->drawRect(r.x, r.y, 2, r.h, border);
    m_renderer->drawRect(r.x + r.w - 2, r.y, 2, r.h, border);

    if (scene.unlocked) {
        m_renderer->renderText(scene.title, r.x + 10, r.y + 8, {255, 255, 255, alpha}, m_font);
        std::string desc = scene.description;
        if (desc.length() > 35) desc = desc.substr(0, 35) + "...";
        m_renderer->renderText(desc, r.x + 10, r.y + 35, {200, 200, 210, alpha}, m_smallFont);
        if (!scene.chapter.empty()) {
            m_renderer->renderText("[" + scene.chapter + "]", r.x + r.w - 100, r.y + 8,
                                  {150, 150, 160, alpha}, m_smallFont);
        }
    } else {
        m_renderer->renderText("??? ", r.x + 10, r.y + 8, {100, 100, 110, alpha}, m_font);
        m_renderer->renderText(scene.unlockCondition, r.x + 10, r.y + 35,
                              {120, 120, 130, alpha}, m_smallFont);
    }
}

void SceneReplay::renderConfirm(Uint8 alpha) {
    int screenW = m_renderer->getWidth();
    int screenH = m_renderer->getHeight();

    m_renderer->drawRect(0, 0, screenW, screenH, {0, 0, 0, 150});

    int boxW = 300, boxH = 120;
    int boxX = screenW/2 - boxW/2;
    int boxY = screenH/2 - boxH/2;
    m_renderer->drawRect(boxX, boxY, boxW, boxH, {30, 30, 40, 255});
    m_renderer->drawRect(boxX, boxY, boxW, 2, {255, 220, 100, 255});
    m_renderer->drawRect(boxX, boxY + boxH - 2, boxW, 2, {255, 220, 100, 255});

    m_renderer->renderText("回看这个场景?", boxX + 50, boxY + 25, {255, 255, 255, 255}, m_font);

    SDL_Rect yesBtn = {screenW/2 - 120, screenH/2 + 20, 100, 40};
    SDL_Rect noBtn  = {screenW/2 + 20, screenH/2 + 20, 100, 40};
    m_renderer->drawRect(yesBtn.x, yesBtn.y, yesBtn.w, yesBtn.h, {60, 80, 60, 255});
    m_renderer->drawRect(noBtn.x, noBtn.y, noBtn.w, noBtn.h, {80, 60, 60, 255});
    m_renderer->renderText("开始", yesBtn.x + 25, yesBtn.y + 10, {255, 255, 255, 255}, m_font);
    m_renderer->renderText("取消", noBtn.x + 30, noBtn.y + 10, {255, 255, 255, 255}, m_font);
}
