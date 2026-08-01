#pragma once

#include <SDL2/SDL.h>
#include <string>
#include <vector>
#include <random>
#include "renderer.h"

// 天气类型
enum class WeatherType {
    NONE,
    RAIN,
    SNOW,
    FOG,
    PETALS,    // 樱花花瓣
    LEAVES     // 落叶
};

// 视差层
struct ParallaxLayer {
    std::string texturePath;
    float scrollSpeed = 1.0f;    // 滚动速度倍率
    float offsetX = 0.0f;
    float offsetY = 0.0f;
    int width = 0;
    int height = 0;
    bool horizontalScroll = true;
    bool verticalScroll = false;
};

// 粒子（用于天气效果）
struct WeatherParticle {
    float x, y;
    float vx, vy;
    float size;
    float alpha;
    float rotation;
    float rotationSpeed;
};

// 背景效果系统
class BackgroundEffect {
public:
    BackgroundEffect(Renderer* renderer);
    ~BackgroundEffect();

    // 视差滚动
    void addParallaxLayer(const ParallaxLayer& layer);
    void clearParallaxLayers();
    void setParallaxOffset(float x, float y);
    void updateParallax(float dt, float moveX = 0.0f, float moveY = 0.0f);
    void renderParallax();

    // 天气效果
    void setWeather(WeatherType type, int particleCount = 200);
    WeatherType getWeather() const { return m_weatherType; }
    void setWeatherIntensity(float intensity) { m_weatherIntensity = intensity; }
    void updateWeather(float dt);
    void renderWeather();

    // 屏幕震动
    void shake(float intensity = 10.0f, float duration = 0.3f);
    void stopShake();
    void updateShake(float dt);
    bool isShaking() const { return m_shakeActive; }
    void getShakeOffset(int& dx, int& dy) const;

    // 总更新
    void update(float dt);

    // 设置屏幕尺寸
    void setScreenSize(int w, int h) { m_screenW = w; m_screenH = h; }

private:
    Renderer* m_renderer;
    int m_screenW = 1280;
    int m_screenH = 720;

    // 视差
    std::vector<ParallaxLayer> m_parallaxLayers;
    float m_parallaxX = 0.0f;
    float m_parallaxY = 0.0f;

    // 天气
    WeatherType m_weatherType = WeatherType::NONE;
    float m_weatherIntensity = 1.0f;
    std::vector<WeatherParticle> m_particles;
    std::mt19937 m_rng;

    void initWeatherParticles(int count);
    void updateParticle(WeatherParticle& p, float dt);

    // 震动
    bool m_shakeActive = false;
    float m_shakeIntensity = 0.0f;
    float m_shakeDuration = 0.0f;
    float m_shakeElapsed = 0.0f;
    int m_shakeOffsetX = 0;
    int m_shakeOffsetY = 0;
};
