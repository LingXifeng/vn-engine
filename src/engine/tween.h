#pragma once

#include <functional>
#include <vector>
#include <memory>
#include <cmath>

#ifndef M_PI
    #define M_PI 3.14159265358979323846
#endif

// 缓动函数类型
enum class EaseType {
    LINEAR,
    EASE_IN_QUAD,
    EASE_OUT_QUAD,
    EASE_IN_OUT_QUAD,
    EASE_IN_CUBIC,
    EASE_OUT_CUBIC,
    EASE_IN_OUT_CUBIC,
    EASE_IN_SINE,
    EASE_OUT_SINE,
    EASE_IN_OUT_SINE,
    EASE_IN_EXPO,
    EASE_OUT_EXPO,
    EASE_IN_OUT_EXPO
};

// 缓动函数计算
inline float ease(EaseType type, float t) {
    t = std::max(0.0f, std::min(1.0f, t));
    switch (type) {
        case EaseType::LINEAR: return t;
        case EaseType::EASE_IN_QUAD: return t * t;
        case EaseType::EASE_OUT_QUAD: return 1.0f - (1.0f - t) * (1.0f - t);
        case EaseType::EASE_IN_OUT_QUAD: return t < 0.5f ? 2*t*t : 1 - 2*(1-t)*(1-t);
        case EaseType::EASE_IN_CUBIC: return t * t * t;
        case EaseType::EASE_OUT_CUBIC: return 1.0f - std::pow(1.0f - t, 3);
        case EaseType::EASE_IN_OUT_CUBIC: return t < 0.5f ? 4*t*t*t : 1 - std::pow(-2*t+2, 3)/2;
        case EaseType::EASE_IN_SINE: return 1.0f - std::cos(t * M_PI / 2);
        case EaseType::EASE_OUT_SINE: return std::sin(t * M_PI / 2);
        case EaseType::EASE_IN_OUT_SINE: return -(std::cos(M_PI * t) - 1) / 2;
        case EaseType::EASE_IN_EXPO: return t == 0 ? 0 : std::pow(2, 10*t - 10);
        case EaseType::EASE_OUT_EXPO: return t == 1 ? 1 : 1 - std::pow(2, -10*t);
        case EaseType::EASE_IN_OUT_EXPO: {
            if (t == 0) return 0;
            if (t == 1) return 1;
            return t < 0.5f ? std::pow(2, 20*t-10)/2 : (2 - std::pow(2, -20*t+10))/2;
        }
        default: return t;
    }
}

// 补间动画
class Tween {
public:
    using Callback = std::function<void()>;

    Tween(float duration, EaseType easeType = EaseType::LINEAR)
        : m_duration(duration), m_easeType(easeType) {}

    // 设置更新回调（参数为插值进度 0-1）
    void onUpdate(std::function<void(float)> cb) { m_updateCallback = cb; }
    void onComplete(Callback cb) { m_completeCallback = cb; }

    bool update(float dt);  // 返回 true 表示完成
    void reset() { m_elapsed = 0; m_done = false; }
    bool isDone() const { return m_done; }

private:
    float m_duration;
    float m_elapsed = 0;
    EaseType m_easeType;
    bool m_done = false;
    std::function<void(float)> m_updateCallback;
    Callback m_completeCallback;
};

// 动画管理器
class TweenManager {
public:
    // 添加动画，返回动画指针（可设置回调）
    std::shared_ptr<Tween> add(float duration, std::function<void(float)> updateCb,
                               EaseType ease = EaseType::LINEAR,
                               Tween::Callback completeCb = nullptr);

    void update(float dt);
    void clear() { m_tweens.clear(); }
    size_t count() const { return m_tweens.size(); }

private:
    std::vector<std::shared_ptr<Tween>> m_tweens;
};
