#pragma once

#include <SDL2/SDL.h>
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include "renderer.h"

// 转场类型
enum class TransitionType {
    NONE,
    FADE,         // 淡入淡出
    FADE_WHITE,   // 白色淡入淡出
    SLIDE_LEFT,   // 从左滑入
    SLIDE_RIGHT,  // 从右滑入
    SLIDE_UP,     // 从上滑入
    SLIDE_DOWN,   // 从下滑入
    DISSOLVE,     // 溶解
    BLIND,        // 百叶窗
    MOSAIC,       // 马赛克
    ZOOM,         // 缩放
    ROTATION,     // 旋转
    CURTAIN       // 窗帘
};

// 转场特效系统
class Transition {
public:
    Transition(Renderer* renderer);
    ~Transition();

    // 启动转场
    void start(TransitionType type, float duration = 0.5f,
               SDL_Color color = {0, 0, 0, 255});

    // 快捷方法
    void fadeIn(float duration = 0.5f);
    void fadeOut(float duration = 0.5f);
    void fadeToColor(SDL_Color color, float duration = 0.5f);
    void slide(TransitionType direction, float duration = 0.4f);
    void dissolve(float duration = 0.6f);
    void blind(float duration = 0.5f);
    void mosaic(float duration = 0.5f);
    void zoom(float duration = 0.4f);
    void curtain(float duration = 0.5f);

    // 更新与渲染（在场景渲染后调用）
    void update(float dt);
    void render();

    // 状态
    bool isActive() const { return m_active; }
    bool isComplete() const { return m_complete; }
    float getProgress() const;  // 0.0 ~ 1.0

    // 转场完成回调
    void setCompleteCallback(std::function<void()> cb) { m_completeCallback = cb; }

    // 中间点回调（转场到一半时调用，用于切换场景内容）
    void setMidpointCallback(std::function<void()> cb) { m_midpointCallback = cb; }

    // 获取/设置当前转场类型
    TransitionType getType() const { return m_type; }

private:
    Renderer* m_renderer;
    TransitionType m_type = TransitionType::NONE;
    float m_duration = 0.5f;
    float m_elapsed = 0.0f;
    bool m_active = false;
    bool m_complete = false;
    bool m_midpointCalled = false;
    SDL_Color m_color = {0, 0, 0, 255};

    std::function<void()> m_completeCallback;
    std::function<void()> m_midpointCallback;

    // 各类型渲染
    void renderFade(float progress);
    void renderSlide(float progress);
    void renderDissolve(float progress);
    void renderBlind(float progress);
    void renderMosaic(float progress);
    void renderZoom(float progress);
    void renderRotation(float progress);
    void renderCurtain(float progress);

    int lerp(int a, int b, float t);
    Uint8 lerpAlpha(Uint8 a, Uint8 b, float t);
};
