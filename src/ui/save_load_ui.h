#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include "renderer.h"
#include "input.h"
#include "../engine/save_load.h"

// 存档/读档 UI 模式
enum class SaveLoadMode {
    SAVE,   // 存档模式
    LOAD    // 读档模式
};

// 存档/读档可视化界面
class SaveLoadUI {
public:
    SaveLoadUI(Renderer* renderer, SaveLoad* saveLoad);
    ~SaveLoadUI();

    void setFont(TTF_Font* font) { m_font = font; }
    void setSmallFont(TTF_Font* font) { m_smallFont = font; }

    // 模式控制
    void showSave();
    void showLoad();
    void hide();
    bool isVisible() const { return m_visible; }

    // 设置槽位数
    void setSlotCount(int count) { m_slotCount = count; }
    void setSlotCountPerRow(int count) { m_slotsPerRow = count; }

    // 截图回调（用于生成缩略图路径）
    void setThumbnailCallback(std::function<std::string(int slot)> cb) {
        m_thumbnailCallback = cb;
    }

    // 存档/读档完成回调
    void setOnSaveCallback(std::function<void(int slot)> cb) { m_onSave = cb; }
    void setOnLoadCallback(std::function<void(int slot)> cb) { m_onLoad = cb; }

    // 设置当前存档信息（用于存档时生成标题）
    void setCurrentInfo(const std::string& title, const std::string& scriptName, int scriptLine);

    // 更新与渲染
    void update(float dt, const Input& input);
    void render();

private:
    Renderer* m_renderer;
    SaveLoad* m_saveLoad;
    TTF_Font* m_font = nullptr;
    TTF_Font* m_smallFont = nullptr;

    bool m_visible = false;
    float m_fadeAlpha = 0.0f;
    SaveLoadMode m_mode = SaveLoadMode::SAVE;

    int m_slotCount = 20;
    int m_slotsPerRow = 4;
    int m_hoverSlot = -1;
    int m_selectedSlot = -1;

    // 滚动
    float m_scrollY = 0.0f;
    float m_targetScrollY = 0.0f;

    // 存档列表缓存
    std::vector<SaveSlot> m_slots;

    // 当前信息
    std::string m_currentTitle;
    std::string m_currentScript;
    int m_currentLine = 0;

    // 回调
    std::function<std::string(int)> m_thumbnailCallback;
    std::function<void(int)> m_onSave;
    std::function<void(int)> m_onLoad;

    // 确认对话框
    bool m_confirmVisible = false;
    std::string m_confirmText;
    int m_confirmSlot = -1;

    void refreshSlots();
    SDL_Rect getSlotRect(int slot) const;
    void renderSlot(const SaveSlot& slot, int index, Uint8 alpha);
    void renderConfirm(Uint8 alpha);
};
