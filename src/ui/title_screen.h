#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <string>
#include <vector>
#include <functional>
#include <memory>
#include "renderer.h"
#include "input.h"
#include "resource_manager.h"

// 菜单项
struct MenuItem {
    std::string text;
    std::function<void()> action;
    bool enabled = true;
    SDL_Rect rect;
    bool hovered = false;
};

// 标题画面
class TitleScreen {
public:
    TitleScreen(Renderer* renderer, ResourceManager* resMgr);
    ~TitleScreen();

    // 设置
    void setTitleImage(const std::string& path);
    void setFont(TTF_Font* font) { m_font = font; }
    void setMenuFont(TTF_Font* font) { m_menuFont = font; }

    // 菜单项
    void addMenuItem(const std::string& text, std::function<void()> action, bool enabled = true);
    void clearMenuItems();

    // 控制
    void show();
    void hide();
    bool isVisible() const { return m_visible; }

    // 更新与渲染
    void update(float dt, const Input& input);
    void render();

    // 选中结果（用于外部轮询）
    int getSelectedIndex() const { return m_selectedIndex; }
    void resetSelection() { m_selectedIndex = -1; }

private:
    Renderer* m_renderer;
    ResourceManager* m_resMgr;
    TTF_Font* m_font = nullptr;
    TTF_Font* m_menuFont = nullptr;

    bool m_visible = false;
    std::shared_ptr<Texture> m_titleImage;

    std::vector<MenuItem> m_items;
    int m_selectedIndex = -1;
    int m_hoverIndex = -1;

    // 菜单布局
    int m_menuX = 0;
    int m_menuY = 0;
    int m_itemHeight = 48;
    int m_itemSpacing = 8;

    // 淡入动画
    float m_fadeAlpha = 0.0f;
    float m_targetAlpha = 1.0f;

    void updateMenuLayout();
};
