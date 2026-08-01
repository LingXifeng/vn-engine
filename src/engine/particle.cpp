#include "particle.h"
#include <algorithm>
#include <cmath>

#ifndef M_PI
    #define M_PI 3.14159265358979323846
#endif

ParticleSystem::ParticleSystem(Renderer* renderer) : m_renderer(renderer) {}
ParticleSystem::~ParticleSystem() {}

float ParticleSystem::randomFloat(float min, float max) {
    // 简单的线性同余随机数
    m_seed = m_seed * 1103515245 + 12345;
    float r = (float)((m_seed / 65536) % 32768) / 32767.0f;
    return min + r * (max - min);
}

void ParticleSystem::initParticle(Particle& p, ParticleType type, const ParticleParams& params, int screenW, int screenH) {
    float cx = params.cx < 0 ? screenW / 2.0f : params.cx;
    float cy = params.cy < 0 ? screenH / 2.0f : params.cy;
    float speed = params.speed;
    float life = params.life;

    // 提取颜色
    Uint8 baseR = (params.color >> 16) & 0xFF;
    Uint8 baseG = (params.color >> 8) & 0xFF;
    Uint8 baseB = params.color & 0xFF;
    Uint8 baseA = (Uint8)(params.alpha * 255);

    p.r = baseR; p.g = baseG; p.b = baseB; p.a = baseA;
    p.rotation = randomFloat(0, 360);
    p.rotSpeed = randomFloat(-180, 180);
    p.scale = params.scale;
    p.maxLife = life * randomFloat(0.7f, 1.3f);
    p.life = p.maxLife;

    switch (type) {
        case ParticleType::RAIN:
            p.x = randomFloat(-50, screenW + 50);
            p.y = randomFloat(-screenH, 0);
            p.vx = speed * 0.3f;
            p.vy = speed * randomFloat(1.5f, 2.5f);
            p.size = randomFloat(1.0f, 3.0f) * params.scale;
            p.r = 150; p.g = 180; p.b = 220;
            break;

        case ParticleType::SNOW:
            p.x = randomFloat(0, screenW);
            p.y = randomFloat(-screenH, 0);
            p.vx = randomFloat(-0.3f, 0.3f) * speed;
            p.vy = speed * randomFloat(0.3f, 0.6f);
            p.size = randomFloat(2.0f, 5.0f) * params.scale;
            p.r = 240; p.g = 245; p.b = 255;
            break;

        case ParticleType::CONFETTI:
            p.x = cx;
            p.y = cy;
            p.vx = randomFloat(-1, 1) * speed;
            p.vy = randomFloat(-1.5f, -0.5f) * speed;
            p.size = randomFloat(3.0f, 8.0f) * params.scale;
            p.r = randomFloat(0, 255);
            p.g = randomFloat(0, 255);
            p.b = randomFloat(0, 255);
            p.rotSpeed = randomFloat(-360, 360);
            break;

        case ParticleType::FIRE:
            p.x = cx + randomFloat(-20, 20) * params.scale;
            p.y = cy;
            p.vx = randomFloat(-0.2f, 0.2f) * speed;
            p.vy = -speed * randomFloat(0.5f, 1.2f);
            p.size = randomFloat(4.0f, 10.0f) * params.scale;
            p.r = 255;
            p.g = randomFloat(100, 200);
            p.b = randomFloat(0, 50);
            p.maxLife = life * randomFloat(0.5f, 1.0f);
            p.life = p.maxLife;
            break;

        case ParticleType::SAKURA:
            p.x = randomFloat(-50, screenW + 50);
            p.y = randomFloat(-screenH, 0);
            p.vx = randomFloat(-0.5f, 0.5f) * speed;
            p.vy = speed * randomFloat(0.2f, 0.5f);
            p.size = randomFloat(4.0f, 8.0f) * params.scale;
            p.r = 255; p.g = 180; p.b = 200;
            p.rotSpeed = randomFloat(-90, 90);
            break;

        case ParticleType::SPARK:
            p.x = cx;
            p.y = cy;
            float angle = randomFloat(0, 2 * M_PI);
            float spd = speed * randomFloat(0.5f, 1.5f);
            p.vx = cos(angle) * spd;
            p.vy = sin(angle) * spd;
            p.size = randomFloat(1.0f, 3.0f) * params.scale;
            p.r = 255; p.g = 220; p.b = 100;
            break;
    }
}

void ParticleSystem::emit(ParticleType type, const ParticleParams& params) {
    Emitter emitter;
    emitter.type = type;
    emitter.params = params;
    emitter.particles.resize(params.count);
    emitter.active = true;

    int sw = m_renderer->getWidth();
    int sh = m_renderer->getHeight();

    for (int i = 0; i < params.count; i++) {
        initParticle(emitter.particles[i], type, params, sw, sh);
        // 错开初始生命，避免同时出现
        emitter.particles[i].life = randomFloat(0, emitter.particles[i].maxLife);
    }

    m_emitters.push_back(std::move(emitter));
}

void ParticleSystem::stop() {
    m_emitters.clear();
}

void ParticleSystem::stopType(ParticleType type) {
    m_emitters.erase(
        std::remove_if(m_emitters.begin(), m_emitters.end(),
                       [type](const Emitter& e) { return e.type == type; }),
        m_emitters.end());
}

bool ParticleSystem::isTypeActive(ParticleType type) const {
    for (const auto& e : m_emitters) if (e.type == type && e.active) return true;
    return false;
}

void ParticleSystem::updateEmitter(Emitter& emitter, float dt, int screenW, int screenH) {
    if (!emitter.active) return;

    float turb = emitter.params.turbulence;

    for (auto& p : emitter.particles) {
        p.life -= dt;
        if (p.life <= 0) {
            // 重生
            initParticle(p, emitter.type, emitter.params, screenW, screenH);
            continue;
        }

        // 湍流
        if (turb > 0) {
            p.vx += randomFloat(-turb, turb) * dt * 50;
            p.vy += randomFloat(-turb, turb) * dt * 50;
        }

        // 重力（火焰和火花有浮力）
        switch (emitter.type) {
            case ParticleType::CONFETTI:
                p.vy += 200 * dt;  // 重力
                break;
            case ParticleType::SPARK:
                p.vy += 150 * dt;  // 轻重力
                p.vx *= (1.0f - 0.5f * dt);  // 阻力
                p.vy *= (1.0f - 0.5f * dt);
                break;
            case ParticleType::FIRE:
                p.vy -= 30 * dt;  // 上升加速
                break;
            case ParticleType::SAKURA:
                // 樱花飘荡
                p.vx += sin(p.life * 3.0f) * 20 * dt;
                break;
            default: break;
        }

        p.x += p.vx * dt;
        p.y += p.vy * dt;
        p.rotation += p.rotSpeed * dt;

        // 屏幕外回收
        if (p.y > screenH + 50 || p.x < -100 || p.x > screenW + 100) {
            initParticle(p, emitter.type, emitter.params, screenW, screenH);
        }
    }
}

void ParticleSystem::update(float dt) {
    int sw = m_renderer->getWidth();
    int sh = m_renderer->getHeight();

    for (auto& emitter : m_emitters) {
        updateEmitter(emitter, dt, sw, sh);
    }
}

void ParticleSystem::renderEmitter(const Emitter& emitter) {
    for (const auto& p : emitter.particles) {
        if (p.life <= 0) continue;

        // 生命衰减影响透明度
        float lifeRatio = p.life / p.maxLife;
        Uint8 alpha = (Uint8)(p.a * lifeRatio);

        SDL_Color color = {p.r, p.g, p.b, alpha};

        switch (emitter.type) {
            case ParticleType::RAIN:
                // 雨用线条
                m_renderer->drawLine(p.x, p.y, p.x - p.vx * 0.05f, p.y - p.vy * 0.05f, color);
                break;

            case ParticleType::SNOW:
            case ParticleType::SAKURA:
                // 雪和樱花用小方块
                m_renderer->drawRect(p.x - p.size / 2, p.y - p.size / 2,
                                     p.size, p.size, color, true);
                break;

            case ParticleType::CONFETTI:
                // 纸屑用旋转的矩形（简化为方块）
                m_renderer->drawRect(p.x - p.size / 2, p.y - p.size / 2,
                                     p.size, p.size * 0.5f, color, true);
                break;

            case ParticleType::FIRE:
            case ParticleType::SPARK:
                // 火焰和火花用加算混合的方块
                {
                    int s = (int)(p.size * lifeRatio);
                    if (s > 0) {
                        m_renderer->drawRect(p.x - s / 2, p.y - s / 2, s, s, color, true);
                    }
                }
                break;

            default:
                m_renderer->drawRect(p.x - p.size / 2, p.y - p.size / 2,
                                     p.size, p.size, color, true);
                break;
        }
    }
}

void ParticleSystem::render() {
    for (const auto& emitter : m_emitters) {
        renderEmitter(emitter);
    }
}
