#include "bg_effect.h"
#include <algorithm>
#include <cmath>

BackgroundEffect::BackgroundEffect(Renderer* renderer)
    : m_renderer(renderer), m_rng(std::random_device()()) {
}

BackgroundEffect::~BackgroundEffect() {
}

// === 视差滚动 ===

void BackgroundEffect::addParallaxLayer(const ParallaxLayer& layer) {
    m_parallaxLayers.push_back(layer);
}

void BackgroundEffect::clearParallaxLayers() {
    m_parallaxLayers.clear();
}

void BackgroundEffect::setParallaxOffset(float x, float y) {
    m_parallaxX = x;
    m_parallaxY = y;
}

void BackgroundEffect::updateParallax(float dt, float moveX, float moveY) {
    m_parallaxX += moveX * dt;
    m_parallaxY += moveY * dt;
}

void BackgroundEffect::renderParallax() {
    for (const auto& layer : m_parallaxLayers) {
        float x = layer.offsetX;
        float y = layer.offsetY;

        if (layer.horizontalScroll) {
            x -= m_parallaxX * layer.scrollSpeed;
            // 平铺
            if (layer.width > 0) {
                x = fmod(x, layer.width);
                if (x > 0) x -= layer.width;
            }
        }
        if (layer.verticalScroll) {
            y -= m_parallaxY * layer.scrollSpeed;
            if (layer.height > 0) {
                y = fmod(y, layer.height);
                if (y > 0) y -= layer.height;
            }
        }

        // 渲染平铺层
        int ix = static_cast<int>(x);
        int iy = static_cast<int>(y);
        int w = layer.width > 0 ? layer.width : m_screenW;
        int h = layer.height > 0 ? layer.height : m_screenH;

        // 平铺渲染
        for (int py = iy; py < m_screenH; py += h) {
            for (int px = ix; px < m_screenW; px += w) {
                m_renderer->drawRect(px, py, w, h, {50, 50, 60, 30});
            }
        }
    }
}

// === 天气效果 ===

void BackgroundEffect::setWeather(WeatherType type, int particleCount) {
    m_weatherType = type;
    if (type == WeatherType::NONE) {
        m_particles.clear();
        return;
    }
    initWeatherParticles(particleCount);
}

void BackgroundEffect::initWeatherParticles(int count) {
    m_particles.clear();
    m_particles.resize(count);

    std::uniform_real_distribution<float> distX(0.0f, static_cast<float>(m_screenW));
    std::uniform_real_distribution<float> distY(0.0f, static_cast<float>(m_screenH));

    for (auto& p : m_particles) {
        p.x = distX(m_rng);
        p.y = distY(m_rng);
        p.rotation = 0.0f;
        p.rotationSpeed = 0.0f;

        switch (m_weatherType) {
            case WeatherType::RAIN:
                p.vx = -2.0f;
                p.vy = 15.0f + distX(m_rng) * 0.01f;
                p.size = 2.0f;
                p.alpha = 0.6f;
                break;
            case WeatherType::SNOW:
                p.vx = (distX(m_rng) - m_screenW / 2.0f) * 0.001f;
                p.vy = 1.5f + distX(m_rng) * 0.005f;
                p.size = 3.0f + distX(m_rng) * 0.01f;
                p.alpha = 0.8f;
                break;
            case WeatherType::FOG:
                p.vx = 0.5f + distX(m_rng) * 0.003f;
                p.vy = 0.0f;
                p.size = 80.0f + distX(m_rng) * 0.05f;
                p.alpha = 0.15f;
                break;
            case WeatherType::PETALS:
                p.vx = -1.0f + distX(m_rng) * 0.003f;
                p.vy = 1.0f + distX(m_rng) * 0.005f;
                p.size = 5.0f;
                p.alpha = 0.7f;
                p.rotationSpeed = (distX(m_rng) - m_screenW / 2.0f) * 0.001f;
                break;
            case WeatherType::LEAVES:
                p.vx = -1.5f + distX(m_rng) * 0.004f;
                p.vy = 2.0f + distX(m_rng) * 0.005f;
                p.size = 6.0f;
                p.alpha = 0.6f;
                p.rotationSpeed = (distX(m_rng) - m_screenW / 2.0f) * 0.002f;
                break;
            default:
                break;
        }
    }
}

void BackgroundEffect::updateParticle(WeatherParticle& p, float dt) {
    p.x += p.vx * dt * 60.0f * m_weatherIntensity;
    p.y += p.vy * dt * 60.0f * m_weatherIntensity;
    p.rotation += p.rotationSpeed * dt * 60.0f;

    // 飘摆效果（雪/花瓣）
    if (m_weatherType == WeatherType::SNOW || m_weatherType == WeatherType::PETALS) {
        p.x += sinf(p.y * 0.02f) * 0.5f * dt * 60.0f;
    }

    // 回绕
    if (p.y > m_screenH) {
        p.y = -10.0f;
        std::uniform_real_distribution<float> distX(0.0f, static_cast<float>(m_screenW));
        p.x = distX(m_rng);
    }
    if (p.x < -20.0f) p.x = m_screenW + 10.0f;
    if (p.x > m_screenW + 20.0f) p.x = -10.0f;
}

void BackgroundEffect::updateWeather(float dt) {
    if (m_weatherType == WeatherType::NONE) return;
    for (auto& p : m_particles) {
        updateParticle(p, dt);
    }
}

void BackgroundEffect::renderWeather() {
    if (m_weatherType == WeatherType::NONE) return;

    for (const auto& p : m_particles) {
        Uint8 a = static_cast<Uint8>(p.alpha * 255 * m_weatherIntensity);
        int x = static_cast<int>(p.x);
        int y = static_cast<int>(p.y);
        int s = static_cast<int>(p.size);

        switch (m_weatherType) {
            case WeatherType::RAIN:
                // 雨线
                m_renderer->drawLine(x, y, x - 2, y + 12, {150, 180, 220, a});
                break;
            case WeatherType::SNOW:
                // 雪花
                m_renderer->drawRect(x - s/2, y - s/2, s, s, {255, 255, 255, a});
                break;
            case WeatherType::FOG:
                // 雾团
                m_renderer->drawRect(x - s/2, y - s/4, s, s/2, {200, 200, 210, a});
                break;
            case WeatherType::PETALS:
                // 花瓣（小粉色方块）
                m_renderer->drawRect(x - s/2, y - s/2, s, s, {255, 180, 200, a});
                break;
            case WeatherType::LEAVES:
                // 落叶（小橙色方块）
                m_renderer->drawRect(x - s/2, y - s/2, s, s, {200, 150, 80, a});
                break;
            default:
                break;
        }
    }
}

// === 屏幕震动 ===

void BackgroundEffect::shake(float intensity, float duration) {
    m_shakeIntensity = intensity;
    m_shakeDuration = duration;
    m_shakeElapsed = 0.0f;
    m_shakeActive = true;
}

void BackgroundEffect::stopShake() {
    m_shakeActive = false;
    m_shakeOffsetX = 0;
    m_shakeOffsetY = 0;
}

void BackgroundEffect::updateShake(float dt) {
    if (!m_shakeActive) return;

    m_shakeElapsed += dt;
    if (m_shakeElapsed >= m_shakeDuration) {
        stopShake();
        return;
    }

    // 衰减
    float remaining = 1.0f - m_shakeElapsed / m_shakeDuration;
    float currentIntensity = m_shakeIntensity * remaining;

    std::uniform_real_distribution<float> dist(-1.0f, 1.0f);
    m_shakeOffsetX = static_cast<int>(dist(m_rng) * currentIntensity);
    m_shakeOffsetY = static_cast<int>(dist(m_rng) * currentIntensity);
}

void BackgroundEffect::getShakeOffset(int& dx, int& dy) const {
    dx = m_shakeOffsetX;
    dy = m_shakeOffsetY;
}

// === 总更新 ===

void BackgroundEffect::update(float dt) {
    updateWeather(dt);
    updateShake(dt);
}
