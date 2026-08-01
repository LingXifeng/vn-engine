#include "background_animation.h"
#include "renderer.h"
#include <algorithm>
#include <cmath>
#include <cstdlib>

BackgroundAnimation::BackgroundAnimation(Renderer* renderer) : m_renderer(renderer) {}

BackgroundAnimation::~BackgroundAnimation() {}

// --- 视差滚动 ---

void BackgroundAnimation::addParallaxLayer(const ParallaxLayer& layer) {
    m_parallaxLayers.push_back(layer);
}

void BackgroundAnimation::clearParallaxLayers() {
    m_parallaxLayers.clear();
}

// --- 天气效果 ---

void BackgroundAnimation::setWeather(WeatherType type) {
    m_weather = type;
    m_weatherTimer = 0.0f;
    m_lightningFlash = 0.0f;
    initWeatherParticles();
}

void BackgroundAnimation::initWeatherParticles() {
    m_weatherParticles.clear();
    if (m_weather == WeatherType::NONE) return;

    int count = 0;
    switch (m_weather) {
        case WeatherType::RAIN:     count = 200; break;
        case WeatherType::SNOW:     count = 150; break;
        case WeatherType::FOG:      count = 30;  break;
        case WeatherType::STARS:    count = 100; break;
        case WeatherType::LIGHTNING: count = 200; break;
        default: return;
    }

    int w = m_renderer->getWidth();
    int h = m_renderer->getHeight();

    for (int i = 0; i < count; ++i) {
        WeatherParticle p;
        p.x = static_cast<float>(rand() % w);
        p.y = static_cast<float>(rand() % h);
        p.alpha = 1.0f;

        switch (m_weather) {
            case WeatherType::RAIN:
            case WeatherType::LIGHTNING:
                p.vx = -50.0f;
                p.vy = 400.0f + (rand() % 200);
                p.size = 1.0f + (rand() % 2);
                break;
            case WeatherType::SNOW:
                p.vx = (rand() % 40 - 20);
                p.vy = 30.0f + (rand() % 50);
                p.size = 2.0f + (rand() % 4);
                p.alpha = 0.5f + (rand() % 100) / 200.0f;
                break;
            case WeatherType::FOG:
                p.vx = 10.0f + (rand() % 20);
                p.vy = 0.0f;
                p.size = 80.0f + (rand() % 100);
                p.alpha = 0.05f + (rand() % 100) / 1000.0f;
                break;
            case WeatherType::STARS:
                p.vx = 0.0f;
                p.vy = 0.0f;
                p.size = 1.0f + (rand() % 3);
                p.alpha = 0.3f + (rand() % 70) / 100.0f;
                break;
            default: break;
        }
        m_weatherParticles.push_back(p);
    }
}

void BackgroundAnimation::updateWeather(float dt) {
    if (m_weather == WeatherType::NONE) return;

    int w = m_renderer->getWidth();
    int h = m_renderer->getHeight();
    m_weatherTimer += dt;

    for (auto& p : m_weatherParticles) {
        p.x += p.vx * dt * m_weatherIntensity;
        p.y += p.vy * dt * m_weatherIntensity;

        // 回绕
        if (p.y > h) { p.y = 0; p.x = rand() % w; }
        if (p.y < 0) { p.y = h; }
        if (p.x > w) { p.x = 0; }
        if (p.x < 0) { p.x = w; }

        // 星星闪烁
        if (m_weather == WeatherType::STARS) {
            p.alpha = 0.3f + 0.4f * (0.5f + 0.5f * std::sin(m_weatherTimer * 2.0f + p.x));
        }
    }

    if (m_weather == WeatherType::LIGHTNING) {
        updateLightning(dt);
    }
}

void BackgroundAnimation::updateLightning(float dt) {
    m_lightningTimer += dt;
    if (m_lightningFlash > 0) {
        m_lightningFlash -= dt * 5.0f;
        if (m_lightningFlash < 0) m_lightningFlash = 0;
    }
    // 随机闪电
    if (m_lightningTimer > 3.0f + (rand() % 500) / 100.0f) {
        m_lightningFlash = 1.0f;
        m_lightningTimer = 0;
    }
}

void BackgroundAnimation::renderWeather() {
    if (m_weather == WeatherType::NONE) return;

    for (const auto& p : m_weatherParticles) {
        Uint8 a = static_cast<Uint8>(p.alpha * 255);
        int x = static_cast<int>(p.x);
        int y = static_cast<int>(p.y);
        int s = static_cast<int>(p.size);

        switch (m_weather) {
            case WeatherType::RAIN:
            case WeatherType::LIGHTNING:
                m_renderer->drawRect(x, y, 1, s * 4, {180, 200, 220, a});
                break;
            case WeatherType::SNOW:
                m_renderer->drawRect(x, y, s, s, {255, 255, 255, a});
                break;
            case WeatherType::FOG:
                m_renderer->drawRect(x - s/2, y - s/4, s, s/2, {200, 200, 210, a});
                break;
            case WeatherType::STARS:
                m_renderer->drawRect(x, y, s, s, {255, 255, 220, a});
                break;
            default: break;
        }
    }

    // 闪电闪光
    if (m_lightningFlash > 0) {
        int w = m_renderer->getWidth();
        int h = m_renderer->getHeight();
        Uint8 a = static_cast<Uint8>(m_lightningFlash * 180);
        m_renderer->drawRect(0, 0, w, h, {255, 255, 255, a});
    }
}

void BackgroundAnimation::renderParallax() {
    // 视差层渲染由场景背景系统处理
    // 此处仅更新偏移量
    for (auto& layer : m_parallaxLayers) {
        if (layer.autoScroll) {
            layer.currentOffset += m_parallaxSpeed * layer.scrollSpeed;
        }
    }
}

// --- 屏幕震动 ---

void BackgroundAnimation::shake(float intensity, float duration) {
    m_shakeIntensity = intensity;
    m_shakeDuration = duration;
    m_shakeTimer = duration;
}

void BackgroundAnimation::stopShake() {
    m_shakeTimer = 0;
    m_shakeOffsetX = 0;
    m_shakeOffsetY = 0;
}

void BackgroundAnimation::getShakeOffset(int& dx, int& dy) const {
    dx = m_shakeOffsetX;
    dy = m_shakeOffsetY;
}

// --- 通用 ---

void BackgroundAnimation::update(float dt) {
    // 视差
    renderParallax();

    // 天气
    updateWeather(dt);

    // 震动
    if (m_shakeTimer > 0) {
        m_shakeTimer -= dt;
        float factor = m_shakeTimer / m_shakeDuration;
        float range = m_shakeIntensity * factor;
        m_shakeOffsetX = static_cast<int>((rand() % 200 - 100) / 100.0f * range);
        m_shakeOffsetY = static_cast<int>((rand() % 200 - 100) / 100.0f * range);
        if (m_shakeTimer <= 0) {
            m_shakeOffsetX = 0;
            m_shakeOffsetY = 0;
        }
    }
}

void BackgroundAnimation::render() {
    renderWeather();
}

bool BackgroundAnimation::hasActiveEffects() const {
    return m_weather != WeatherType::NONE ||
           !m_parallaxLayers.empty() ||
           m_shakeTimer > 0;
}
