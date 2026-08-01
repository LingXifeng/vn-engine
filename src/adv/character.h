#pragma once

#include <SDL2/SDL.h>
#include <string>
#include <memory>
#include "renderer.h"
#include "tween.h"

// 立绘位置预设
enum class CharPosition {
    LEFT,
    CENTER,
    RIGHT,
    CUSTOM
};

// 立绘表情/状态
struct CharState {
    std::string expression;  // 表情名
    std::shared_ptr<Texture> texture;
};

// 角色立绘
class Character {
public:
    Character(Renderer* renderer, const std::string& name = "");
    ~Character();

    // 设置
    void setName(const std::string& name) { m_name = name; }
    const std::string& getName() const { return m_name; }

    // 加载表情纹理
    void addExpression(const std::string& exprName, std::shared_ptr<Texture> tex);
    void setExpression(const std::string& exprName);

    // 位置控制
    void setPosition(CharPosition pos);
    void setCustomPosition(float x, float y);
    void setBaseY(float y) { m_baseY = y; }
    float getX() const { return m_x; }
    float getY() const { return m_y; }

    // 效果
    void setAlpha(Uint8 alpha) { m_alpha = alpha; }
    Uint8 getAlpha() const { return m_alpha; }
    void setScale(float sx, float sy) { m_scaleX = sx; m_scaleY = sy; }
    void setZOrder(int z) { m_zOrder = z; }
    int getZOrder() const { return m_zOrder; }

    // 显示/隐藏（带动画）
    void show(TweenManager* tweens = nullptr, float duration = 0.3f);
    void hide(TweenManager* tweens = nullptr, float duration = 0.3f);
    bool isVisible() const { return m_visible; }
    bool isHidden() const { return m_hidden; }

    // 渲染
    void render(Renderer* renderer);

private:
    Renderer* m_renderer;
    std::string m_name;

    std::unordered_map<std::string, CharState> m_expressions;
    std::string m_currentExpr;
    std::shared_ptr<Texture> m_texture;

    CharPosition m_position = CharPosition::LEFT;
    float m_x = 0, m_y = 0;
    float m_baseY = 0;  // 底部 Y 坐标
    float m_scaleX = 1.0f, m_scaleY = 1.0f;
    Uint8 m_alpha = 255;
    int m_zOrder = 10;

    bool m_visible = false;
    bool m_hidden = true;
};
