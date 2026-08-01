#pragma once

#include <SDL2/SDL.h>
#include <string>
#include <vector>
#include <memory>
#include "renderer.h"

// 粒子类型
enum class ParticleType {
    RAIN,       // 雨
    SNOW,       // 雪
    CONFETTI,   // 纸屑
    FIRE,       // 火焰
    SAKURA,     // 樱花
    SPARK,      // 火花
    NONE
};

// 单个粒子
struct Particle {
    float x, y;          // 位置
    float vx, vy;        // 速度
    float life;          // 剩余生命
    float maxLife;       // 最大生命
    float size;          // 大小
    float rotation;      // 旋转角度
    float rotSpeed;      // 旋转速度
    Uint8 r, g, b, a;    // 颜色
    float scale;         // 缩放
};

// 粒子发射器参数
struct ParticleParams {
    int count = 100;             // 粒子数量
    float speed = 100.0f;        // 基础速度
    float scale = 1.0f;          // 缩放
    float alpha = 1.0f;          // 透明度
    float life = 3.0f;           // 生命周期（秒）
    Uint32 color = 0xFFFFFFFF;   // 颜色 (ARGB)
    float turbulence = 0.0f;     // 湍流强度
    float cx = -1.0f;            // 中心 X（-1 = 屏幕中心）
    float cy = -1.0f;            // 中心 Y
};

// 粒子系统
class ParticleSystem {
public:
    ParticleSystem(Renderer* renderer);
    ~ParticleSystem();

    // 发射特效
    void emit(ParticleType type, const ParticleParams& params = ParticleParams());

    // 停止所有特效
    void stop();
    void stopType(ParticleType type);

    // 状态
    bool isActive() const { return !m_emitters.empty(); }
    bool isTypeActive(ParticleType type) const;

    // 更新与渲染
    void update(float dt);
    void render();

private:
    Renderer* m_renderer;

    // 发射器
    struct Emitter {
        ParticleType type;
        ParticleParams params;
        std::vector<Particle> particles;
        float spawnTimer = 0.0f;
        bool active = true;
    };

    std::vector<Emitter> m_emitters;

    // 粒子初始化
    void initParticle(Particle& p, ParticleType type, const ParticleParams& params, int screenW, int screenH);
    void updateEmitter(Emitter& emitter, float dt, int screenW, int screenH);
    void renderEmitter(const Emitter& emitter);

    // 随机数
    float randomFloat(float min, float max);
    Uint32 m_seed = 12345;
};
