#include "save_load_ui.h"
#include <algorithm>
#include <cmath>

SaveLoadUI::SaveLoadUI(Renderer* renderer, SaveLoad* saveLoad)
    : m_renderer(renderer), m_saveLoad(saveLoad) {
}

SaveLoadUI::~SaveLoadUI() {
}

void SaveLoadUI::showSave() {
    m_mode = SaveLoadMode::SAVE;
    m_visible = true;
    m_fadeAlpha = 0.0f;
    m_scrollY = 0.0f;
    m_targetScrollY = 0.0f;
    m_confirmVisible = false;
    refreshSlots();
}

void SaveLoadUI::showLoad() {
    m_mode = SaveLoadMode::LOAD;
    m_visible = true;
    m_fadeAlpha = 0.0f;
    m_scrollY = 0.0f;
    m_targetScrollY = 0.0f;
    m_confirmVisible = false;
    refreshSlots();
}

void SaveLoadUI::hide() {
    m_visible = false;
    m_confirmVisible = false;
}

void SaveLoadUI::setCurrentInfo(const std::string& title, const std::string& scriptName, int scriptLine) {
    m_currentTitle = title;
    m_currentScript = scriptName;
    m_currentLine = scriptLine;
}

void SaveLoadUI::refreshSlots() {
    m_slots.clear();
    for (int i = 0; i < m_slotCount; ++i) {
        m_slots.push_back(m_saveLoad->getSaveInfo(i));
    }
}

SDL_Rect SaveLoadUI::getSlotRect(int slot) const {
    int screenW = m_renderer->getWidth();
    int margin = 20;
    int slotW = (screenW - margin * (m_slotsPerRow + 1)) / m_slotsPerRow;
    int slotH = 120;
    int row = slot / m_slotsPerRow;
    int col = slot % m_slotsPerRow;
    int x = margin + col * (slotW + margin);
    int y = 60 + row * (slotH + margin) - static_cast<int>(m_scrollY);
    return {x, y, slotW, slotH};
}

void SaveLoadUI::update(float dt, const Input& input) {
    if (!m_visible) return;

    // 淡入
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
                // 确认操作
                if (m_mode == SaveLoadMode::SAVE) {
                    m_saveLoad->save(m_confirmSlot, m_currentTitle, m_currentScript, m_currentLine);
                    refreshSlots();
                    if (m_onSave) m_onSave(m_confirmSlot);
                } else {
                    m_saveLoad->load(m_confirmSlot);
                    if (m_onLoad) m_onLoad(m_confirmSlot);
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
    if (wheel != 0) {
        m_targetScrollY -= wheel * 40;
    }
    m_targetScrollY = std::max(0.0f, m_targetScrollY);
    m_scrollY += (m_targetScrollY - m_scrollY) * 0.2f;

    // 检测 hover 和点击
    m_hoverSlot = -1;
    int mx, my;
    input.getMousePosition(mx, my);

    for (int i = 0; i < m_slotCount; ++i) {
        SDL_Rect r = getSlotRect(i);
        if (mx >= r.x && mx < r.x + r.w && my >= r.y && my < r.y + r.h) {
            m_hoverSlot = i;
            break;
        }
    }

    // ESC 关闭
    if (input.isKeyPressed(SDL_SCANCODE_ESCAPE)) {
        hide();
        return;
    }

    // 点击槽位
    if (input.isMouseButtonPressed(SDL_BUTTON_LEFT) && m_hoverSlot >= 0) {
        int slot = m_hoverSlot;
        if (m_mode == SaveLoadMode::SAVE) {
            if (!m_slots[slot].empty) {
                // 覆盖确认
                m_confirmVisible = true;
                m_confirmSlot = slot;
                m_confirmText = "覆盖存档 #" + std::to_string(slot) + " ?";
            } else {
                m_saveLoad->save(slot, m_currentTitle, m_currentScript, m_currentLine);
                refreshSlots();
                if (m_onSave) m_onSave(slot);
                hide();
            }
        } else {
            if (!m_slots[slot].empty) {
                m_confirmVisible = true;
                m_confirmSlot = slot;
                m_confirmText = "读取存档 #" + std::to_string(slot) + " ?";
            }
        }
    }

    // 右键删除
    if (input.isMouseButtonPressed(SDL_BUTTON_RIGHT) && m_hoverSlot >= 0) {
        if (!m_slots[m_hoverSlot].empty) {
            m_saveLoad->deleteSave(m_hoverSlot);
            refreshSlots();
        }
    }
}

void SaveLoadUI::render() {
    if (!m_visible) return;

    Uint8 alpha = static_cast<Uint8>(m_fadeAlpha * 255);
    int screenW = m_renderer->getWidth();
    int screenH = m_renderer->getHeight();

    // 半透明背景
    m_renderer->drawRect(0, 0, screenW, screenH, {0, 0, 0, alpha});

    // 标题
    std::string title = (m_mode == SaveLoadMode::SAVE) ? "存 档" : "读 档";
    SDL_Color titleColor = {255, 220, 100, alpha};
    m_renderer->renderText(title, screenW/2 - 50, 15, titleColor, m_font);

    // 槽位
    for (int i = 0; i < m_slotCount; ++i) {
        // 裁剪可见区域
        SDL_Rect r = getSlotRect(i);
        if (r.y + r.h < 50 || r.y > screenH) continue;
        renderSlot(m_slots[i], i, alpha);
    }

    // 确认对话框
    if (m_confirmVisible) {
        renderConfirm(alpha);
    }
}

void SaveLoadUI::renderSlot(const SaveSlot& slot, int index, Uint8 alpha) {
    SDL_Rect r = getSlotRect(index);
    bool hover = (m_hoverSlot == index);

    // 背景
    SDL_Color bg = hover ? SDL_Color{40, 40, 60, static_cast<Uint8>(alpha * 0.9f)}
                         : SDL_Color{20, 20, 30, static_cast<Uint8>(alpha * 0.8f)};
    m_renderer->drawRect(r.x, r.y, r.w, r.h, bg);

    // 边框
    SDL_Color border = hover ? SDL_Color{255, 220, 100, alpha} : SDL_Color{80, 80, 100, alpha};
    m_renderer->drawRect(r.x, r.y, r.w, 2, border);
    m_renderer->drawRect(r.x, r.y + r.h - 2, r.w, 2, border);
    m_renderer->drawRect(r.x, r.y, 2, r.h, border);
    m_renderer->drawRect(r.x + r.w - 2, r.y, 2, r.h, border);

    // 槽位号
    SDL_Color numColor = {150, 150, 160, alpha};
    m_renderer->renderText("No." + std::to_string(index), r.x + 8, r.y + 5, numColor, m_smallFont);

    if (slot.empty) {
        // 空槽位
        SDL_Color emptyColor = {100, 100, 110, alpha};
        std::string emptyText = (m_mode == SaveLoadMode::SAVE) ? "[ 空 ]" : "--- 空 ---";
        m_renderer->renderText(emptyText, r.x + r.w/2 - 40, r.y + r.h/2 - 10, emptyColor, m_font);
    } else {
        // 缩略图区域
        int thumbW = 100, thumbH = 75;
        int thumbX = r.x + 8;
        int thumbY = r.y + 25;
        m_renderer->drawRect(thumbX, thumbY, thumbW, thumbH, {50, 50, 60, alpha});

        // 信息
        SDL_Color infoColor = {220, 220, 230, alpha};
        SDL_Color timeColor = {150, 150, 160, alpha};

        // 标题（截断）
        std::string title = slot.title;
        if (title.length() > 20) title = title.substr(0, 20) + "...";
        m_renderer->renderText(title, thumbX + thumbW + 10, thumbY, infoColor, m_smallFont);

        // 时间戳
        m_renderer->renderText(slot.timestamp, thumbX + thumbW + 10, thumbY + 25, timeColor, m_smallFont);

        // 脚本名
        m_renderer->renderText(slot.scriptName + ":" + std::to_string(slot.scriptLine),
                               thumbX + thumbW + 10, thumbY + 45, timeColor, m_smallFont);
    }
}

void SaveLoadUI::renderConfirm(Uint8 alpha) {
    int screenW = m_renderer->getWidth();
    int screenH = m_renderer->getHeight();

    // 遮罩
    m_renderer->drawRect(0, 0, screenW, screenH, {0, 0, 0, 150});

    // 对话框
    int boxW = 300, boxH = 120;
    int boxX = screenW/2 - boxW/2;
    int boxY = screenH/2 - boxH/2;
    m_renderer->drawRect(boxX, boxY, boxW, boxH, {30, 30, 40, 255});
    m_renderer->drawRect(boxX, boxY, boxW, 2, {255, 220, 100, 255});
    m_renderer->drawRect(boxX, boxY + boxH - 2, boxW, 2, {255, 220, 100, 255});

    // 文字
    m_renderer->renderText(m_confirmText, boxX + 30, boxY + 25, {255, 255, 255, 255}, m_font);

    // 按钮
    SDL_Rect yesBtn = {screenW/2 - 120, screenH/2 + 20, 100, 40};
    SDL_Rect noBtn  = {screenW/2 + 20, screenH/2 + 20, 100, 40};

    m_renderer->drawRect(yesBtn.x, yesBtn.y, yesBtn.w, yesBtn.h, {60, 80, 60, 255});
    m_renderer->drawRect(noBtn.x, noBtn.y, noBtn.w, noBtn.h, {80, 60, 60, 255});
    m_renderer->renderText("确定", yesBtn.x + 30, yesBtn.y + 10, {255, 255, 255, 255}, m_font);
    m_renderer->renderText("取消", noBtn.x + 30, noBtn.y + 10, {255, 255, 255, 255}, m_font);
}
