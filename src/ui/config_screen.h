#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <string>
#include <vector>
#include <functional>
#include <memory>
#include "renderer.h"
#include "input.h"
#include "audio.h"

// 配置项类型
enum class ConfigItemType {
    SLIDER,     // 滑块（音量等）
    TOGGLE,     // 开关（全屏等）
    BUTTON      // 按钮（返回等）
};

// 配置项
struct ConfigItem {
    std::string label;
    ConfigItemType type;
    int minVal = 0;
    int maxVal = 100;
    int currentVal = 50;
    std::function<void(int)> onChange;
    SDL_Rect rect;
    bool hovered = false;
    int sliderHandleX = 0;
};

// 配置界面
class ConfigScreen {
public:
    ConfigScreen(Renderer* renderer, Audio* audio);
    ~ConfigScreen();

    void setFont(TTF_Font* font) { m_font = font; }

    // 添加配置项
    void addSlider(const std::string& label, int min, int max, int current, std::function<void(int)> onChange);
    void addToggle(const std::string& label, bool current, std::function<void(int)> onChange);
    void addButton(const std::string& label, std::function<void(int)> action);

    // 控制
    void show();
    void hide();
    bool isVisible() const { return m_visible; }

    // 更新与渲染
    void update(float dt, const Input& input);
    void render();

    // 结果
    int getSelectedIndex() const { return m_selectedIndex; }
    void resetSelection() { m_selectedIndex = -1; }

private:
    Renderer* m_renderer;
    Audio* m_audio;
    TTF_Font* m_font = nullptr;

    bool m_visible = false;
    std::vector<ConfigItem> m_items;
    int m_selectedIndex = -1;
    int m_hoverIndex = -1;
    bool m_dragging = false;

    // 布局
    int m_panelX = 0, m_panelY = 0;
    int m_panelW = 500, m_panelH = 400;
    int m_itemHeight = 50;
    int m_itemSpacing = 10;

    float m_fadeAlpha = 0.0f;

    void updateLayout();
    int getSliderValue(int itemIndex, int mouseX);
};
