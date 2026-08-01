#pragma once

#include <SDL2/SDL.h>
#include <string>
#include <vector>
#include <memory>
#include <functional>

class Renderer;

// 天气类型
enum class WeatherType {
    NONE,
    RAIN,
    SNOW,
    FOG,
    STARS,
    LIGHTNING
};

// 视差层
struct ParallaxLayer {
    std::string texturePath;
    float scrollSpeed = 1.0f;    // 滚动速度倍率
    float offsetX = 0.0f;
    float offsetY = 0.0f;
    bool horizontal = true;      // 水平滚动
    bool autoScroll = true;      // 自动滚动
    float currentOffset = 0.0f;
};

// 背景动画效果系统
class BackgroundAnimation {
public:
    BackgroundAnimation(Renderer* renderer);
    ~BackgroundAnimation();

    // --- 视差滚动 ---
    void addParallaxLayer(const ParallaxLayer& layer);
    void clearParallaxLayers();
    void setParallaxSpeed(float speed) { m_parallaxSpeed = speed; }

    // --- 天气效果 ---
    void setWeather(WeatherType type);
    WeatherType getWeather() const { return m_weather; }
    void setWeatherIntensity(float intensity) { m_weatherIntensity = intensity; }

    // --- 屏幕震动 ---
    void shake(float intensity = 10.0f, float duration = 0.3f);
    void stopShake();
    bool isShaking() const { return m_shakeTimer > 0; }

    // 获取震动偏移
    void getShakeOffset(int& dx, int& dy) const;

    // --- 通用 ---
    void update(float dt);
    void render();

    // 是否有任何活跃效果
    bool hasActiveEffects() const;

private:
    Renderer* m_renderer;

    // 视差
    std::vector<ParallaxLayer> m_parallaxLayers;
    float m_parallaxSpeed = 1.0f;

    // 天气
    WeatherType m_weather = WeatherType::NONE;
    float m_weatherIntensity = 1.0f;
    float m_weatherTimer = 0.0f;

    // 天气粒子
    struct WeatherParticle {
        float x, y;
        float vx, vy;
        float size;
        float alpha;
    };
    std::vector<WeatherParticle> m_weatherParticles;

    // 震动
    float m_shakeIntensity = 0.0f;
    float m_shakeTimer = 0.0f;
    float m_shakeDuration = 0.0f;
    int m_shakeOffsetX = 0;
    int m_shakeOffsetY = 0;

    // 闪电
    float m_lightningTimer = 0.0f;
    float m_lightningFlash = 0.0f;

    void updateWeather(float dt);
    void renderWeather();
    void renderParallax();
    void initWeatherParticles();
    void updateLightning(float dt);
};
