#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <string>
#include <functional>
#include <memory>

class Renderer;

// 转场类型
enum class TransitionType {
    NONE,
    FADE,       // 淡入淡出
    FADE_WHITE, // 白色淡入淡出
    SLIDE_LEFT,
    SLIDE_RIGHT,
    SLIDE_UP,
    SLIDE_DOWN,
    DISSOLVE,   // 溶解
    BLIND,      // 百叶窗
    MOSAIC,     // 马赛克
    CURTAIN,    // 窗帘
    ZOOM        // 缩放
};

// 转场特效系统
class Transition {
public:
    Transition(Renderer* renderer);
    ~Transition();

    // 开始转场
    // type: 转场类型
    // duration: 持续时间（秒）
    // 完成后自动停止
    void start(TransitionType type, float duration = 0.5f);

    // 便捷方法
    void fade(float duration = 0.5f) { start(TransitionType::FADE, duration); }
    void fadeWhite(float duration = 0.5f) { start(TransitionType::FADE_WHITE, duration); }
    void slideLeft(float duration = 0.4f) { start(TransitionType::SLIDE_LEFT, duration); }
    void slideRight(float duration = 0.4f) { start(TransitionType::SLIDE_RIGHT, duration); }
    void slideUp(float duration = 0.4f) { start(TransitionType::SLIDE_UP, duration); }
    void slideDown(float duration = 0.4f) { start(TransitionType::SLIDE_DOWN, duration); }
    void dissolve(float duration = 0.6f) { start(TransitionType::DISSOLVE, duration); }
    void blind(float duration = 0.5f) { start(TransitionType::BLIND, duration); }
    void mosaic(float duration = 0.5f) { start(TransitionType::MOSAIC, duration); }
    void curtain(float duration = 0.6f) { start(TransitionType::CURTAIN, duration); }
    void zoom(float duration = 0.4f) { start(TransitionType::ZOOM, duration); }

    // 停止
    void stop();

    // 是否正在转场中
    bool isActive() const { return m_active; }

    // 获取进度 (0.0 ~ 1.0)
    float getProgress() const { return m_progress; }

    // 设置完成回调
    void setCompleteCallback(std::function<void()> cb) { m_completeCallback = cb; }

    // 更新与渲染（渲染在所有内容之上）
    void update(float dt);
    void render();

private:
    Renderer* m_renderer;
    bool m_active = false;
    TransitionType m_type = TransitionType::NONE;
    float m_duration = 0.5f;
    float m_elapsed = 0.0f;
    float m_progress = 0.0f;

    std::function<void()> m_completeCallback;

    // 马赛克/百叶窗辅助
    int m_blindCount = 10;
    int m_mosaicSize = 20;

    void renderFade(Uint8 alpha);
    void renderFadeWhite(Uint8 alpha);
    void renderSlide(int direction);
    void renderDissolve(Uint8 alpha);
    void renderBlind(float progress);
    void renderMosaic(float progress);
    void renderCurtain(float progress);
    void renderZoom(float progress);
};
