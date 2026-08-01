#include "transition.h"
#include <algorithm>
#include <cmath>

Transition::Transition(Renderer* renderer) : m_renderer(renderer) {
}

Transition::~Transition() {
}

void Transition::start(TransitionType type, float duration, SDL_Color color) {
    m_type = type;
    m_duration = std::max(0.01f, duration);
    m_elapsed = 0.0f;
    m_active = true;
    m_complete = false;
    m_midpointCalled = false;
    m_color = color;
}

void Transition::fadeIn(float duration) {
    start(TransitionType::FADE, duration, {0, 0, 0, 255});
}

void Transition::fadeOut(float duration) {
    start(TransitionType::FADE, duration, {0, 0, 0, 255});
}

void Transition::fadeToColor(SDL_Color color, float duration) {
    start(TransitionType::FADE, duration, color);
}

void Transition::slide(TransitionType direction, float duration) {
    start(direction, duration, {0, 0, 0, 255});
}

void Transition::dissolve(float duration) {
    start(TransitionType::DISSOLVE, duration, {0, 0, 0, 255});
}

void Transition::blind(float duration) {
    start(TransitionType::BLIND, duration, {0, 0, 0, 255});
}

void Transition::mosaic(float duration) {
    start(TransitionType::MOSAIC, duration, {0, 0, 0, 255});
}

void Transition::zoom(float duration) {
    start(TransitionType::ZOOM, duration, {0, 0, 0, 255});
}

void Transition::curtain(float duration) {
    start(TransitionType::CURTAIN, duration, {0, 0, 0, 255});
}

float Transition::getProgress() const {
    if (!m_active) return 1.0f;
    return std::clamp(m_elapsed / m_duration, 0.0f, 1.0f);
}

void Transition::update(float dt) {
    if (!m_active) return;

    m_elapsed += dt;
    float progress = getProgress();

    // 中点回调
    if (!m_midpointCalled && progress >= 0.5f) {
        m_midpointCalled = true;
        if (m_midpointCallback) m_midpointCallback();
    }

    if (progress >= 1.0f) {
        m_active = false;
        m_complete = true;
        if (m_completeCallback) m_completeCallback();
    }
}

void Transition::render() {
    if (!m_active) return;

    float progress = getProgress();

    switch (m_type) {
        case TransitionType::FADE:
        case TransitionType::FADE_WHITE:
            renderFade(progress);
            break;
        case TransitionType::SLIDE_LEFT:
        case TransitionType::SLIDE_RIGHT:
        case TransitionType::SLIDE_UP:
        case TransitionType::SLIDE_DOWN:
            renderSlide(progress);
            break;
        case TransitionType::DISSOLVE:
            renderDissolve(progress);
            break;
        case TransitionType::BLIND:
            renderBlind(progress);
            break;
        case TransitionType::MOSAIC:
            renderMosaic(progress);
            break;
        case TransitionType::ZOOM:
            renderZoom(progress);
            break;
        case TransitionType::ROTATION:
            renderRotation(progress);
            break;
        case TransitionType::CURTAIN:
            renderCurtain(progress);
            break;
        default:
            break;
    }
}

void Transition::renderFade(float progress) {
    int screenW = m_renderer->getWidth();
    int screenH = m_renderer->getHeight();

    // 0.0~0.5: 遮罩变深; 0.5~1.0: 遮罩变浅
    float alpha;
    if (progress < 0.5f) {
        alpha = progress * 2.0f;
    } else {
        alpha = (1.0f - progress) * 2.0f;
    }
    Uint8 a = static_cast<Uint8>(alpha * m_color.a);
    m_renderer->drawRect(0, 0, screenW, screenH, {m_color.r, m_color.g, m_color.b, a});
}

void Transition::renderSlide(float progress) {
    int screenW = m_renderer->getWidth();
    int screenH = m_renderer->getHeight();

    // 遮罩从一侧滑入覆盖屏幕，再滑出
    float offset;
    if (progress < 0.5f) {
        offset = progress * 2.0f;  // 0 -> 1
    } else {
        offset = (1.0f - progress) * 2.0f;  // 1 -> 0
    }

    int w = screenW;
    int h = screenH;
    int x = 0, y = 0;

    switch (m_type) {
        case TransitionType::SLIDE_LEFT:
            x = static_cast<int>((1.0f - offset) * screenW);
            w = static_cast<int>(offset * screenW);
            x = 0;
            w = static_cast<int>(offset * screenW);
            break;
        case TransitionType::SLIDE_RIGHT:
            x = screenW - static_cast<int>(offset * screenW);
            w = static_cast<int>(offset * screenW);
            break;
        case TransitionType::SLIDE_UP:
            y = 0;
            h = static_cast<int>(offset * screenH);
            break;
        case TransitionType::SLIDE_DOWN:
            y = screenH - static_cast<int>(offset * screenH);
            h = static_cast<int>(offset * screenH);
            break;
        default:
            break;
    }

    m_renderer->drawRect(x, y, w, h, {m_color.r, m_color.g, m_color.b, m_color.a});
}

void Transition::renderDissolve(float progress) {
    int screenW = m_renderer->getWidth();
    int screenH = m_renderer->getHeight();

    // 使用噪声块模拟溶解效果
    float alpha;
    if (progress < 0.5f) {
        alpha = progress * 2.0f;
    } else {
        alpha = (1.0f - progress) * 2.0f;
    }

    int blockSize = 20;
    Uint8 a = static_cast<Uint8>(alpha * 200);

    for (int y = 0; y < screenH; y += blockSize) {
        for (int x = 0; x < screenW; x += blockSize) {
            // 伪随机决定该块是否显示
            int hash = (x * 73856093 + y * 19349663) & 0x7FFFFFFF;
            float threshold = static_cast<float>(hash % 1000) / 1000.0f;
            if (threshold < alpha) {
                m_renderer->drawRect(x, y, blockSize, blockSize,
                                     {m_color.r, m_color.g, m_color.b, a});
            }
        }
    }
}

void Transition::renderBlind(float progress) {
    int screenW = m_renderer->getWidth();
    int screenH = m_renderer->getHeight();

    float alpha;
    if (progress < 0.5f) {
        alpha = progress * 2.0f;
    } else {
        alpha = (1.0f - progress) * 2.0f;
    }

    int blindCount = 10;
    int blindH = screenH / blindCount;
    Uint8 a = static_cast<Uint8>(alpha * m_color.a);

    for (int i = 0; i < blindCount; ++i) {
        int y = i * blindH;
        int h = static_cast<int>(blindH * alpha);
        m_renderer->drawRect(0, y, screenW, h, {m_color.r, m_color.g, m_color.b, a});
    }
}

void Transition::renderMosaic(float progress) {
    int screenW = m_renderer->getWidth();
    int screenH = m_renderer->getHeight();

    float alpha;
    if (progress < 0.5f) {
        alpha = progress * 2.0f;
    } else {
        alpha = (1.0f - progress) * 2.0f;
    }

    // 马赛克块逐渐变大再变小
    int maxBlock = 60;
    int blockSize = static_cast<int>(maxBlock * alpha);
    if (blockSize < 2) blockSize = 2;

    Uint8 a = static_cast<Uint8>(alpha * m_color.a);

    for (int y = 0; y < screenH; y += blockSize) {
        for (int x = 0; x < screenW; x += blockSize) {
            m_renderer->drawRect(x, y, blockSize, blockSize,
                                 {m_color.r, m_color.g, m_color.b, a});
        }
    }
}

void Transition::renderZoom(float progress) {
    int screenW = m_renderer->getWidth();
    int screenH = m_renderer->getHeight();

    float alpha;
    if (progress < 0.5f) {
        alpha = progress * 2.0f;
    } else {
        alpha = (1.0f - progress) * 2.0f;
    }

    // 从中心向外扩散的矩形遮罩
    int margin = static_cast<int>((1.0f - alpha) * std::max(screenW, screenH) / 2);
    Uint8 a = static_cast<Uint8>(alpha * m_color.a);

    m_renderer->drawRect(margin, margin, screenW - 2 * margin, screenH - 2 * margin,
                         {m_color.r, m_color.g, m_color.b, a});
}

void Transition::renderRotation(float progress) {
    // 简化：用对角线扫描
    int screenW = m_renderer->getWidth();
    int screenH = m_renderer->getHeight();

    float alpha;
    if (progress < 0.5f) {
        alpha = progress * 2.0f;
    } else {
        alpha = (1.0f - progress) * 2.0f;
    }

    int w = static_cast<int>(screenW * alpha);
    int h = static_cast<int>(screenH * alpha);
    int x = (screenW - w) / 2;
    int y = (screenH - h) / 2;

    m_renderer->drawRect(x, y, w, h, {m_color.r, m_color.g, m_color.b, m_color.a});
}

void Transition::renderCurtain(float progress) {
    int screenW = m_renderer->getWidth();
    int screenH = m_renderer->getHeight();

    float alpha;
    if (progress < 0.5f) {
        alpha = progress * 2.0f;
    } else {
        alpha = (1.0f - progress) * 2.0f;
    }

    // 从左右两侧向中间合拢
    int curtainW = static_cast<int>(screenW * alpha / 2);
    Uint8 a = static_cast<Uint8>(m_color.a);

    m_renderer->drawRect(0, 0, curtainW, screenH, {m_color.r, m_color.g, m_color.b, a});
    m_renderer->drawRect(screenW - curtainW, 0, curtainW, screenH,
                         {m_color.r, m_color.g, m_color.b, a});
}

int Transition::lerp(int a, int b, float t) {
    return static_cast<int>(a + (b - a) * t);
}

Uint8 Transition::lerpAlpha(Uint8 a, Uint8 b, float t) {
    return static_cast<Uint8>(a + (b - a) * t);
}
