#include "transition.h"
#include "renderer.h"
#include <algorithm>
#include <cmath>

Transition::Transition(Renderer* renderer) : m_renderer(renderer) {}

Transition::~Transition() {}

void Transition::start(TransitionType type, float duration) {
    m_type = type;
    m_duration = duration;
    m_elapsed = 0.0f;
    m_progress = 0.0f;
    m_active = true;
}

void Transition::stop() {
    m_active = false;
    m_progress = 0.0f;
    m_elapsed = 0.0f;
}

void Transition::update(float dt) {
    if (!m_active) return;

    m_elapsed += dt;
    m_progress = std::min(m_elapsed / m_duration, 1.0f);

    if (m_progress >= 1.0f) {
        m_active = false;
        if (m_completeCallback) {
            m_completeCallback();
        }
    }
}

void Transition::render() {
    if (!m_active) return;

    Uint8 alpha = static_cast<Uint8>(m_progress * 255);

    switch (m_type) {
        case TransitionType::FADE:       renderFade(alpha); break;
        case TransitionType::FADE_WHITE: renderFadeWhite(alpha); break;
        case TransitionType::SLIDE_LEFT: renderSlide(0); break;
        case TransitionType::SLIDE_RIGHT: renderSlide(1); break;
        case TransitionType::SLIDE_UP:   renderSlide(2); break;
        case TransitionType::SLIDE_DOWN: renderSlide(3); break;
        case TransitionType::DISSOLVE:   renderDissolve(alpha); break;
        case TransitionType::BLIND:      renderBlind(m_progress); break;
        case TransitionType::MOSAIC:     renderMosaic(m_progress); break;
        case TransitionType::CURTAIN:    renderCurtain(m_progress); break;
        case TransitionType::ZOOM:       renderZoom(m_progress); break;
        default: break;
    }
}

void Transition::renderFade(Uint8 alpha) {
    int w = m_renderer->getWidth();
    int h = m_renderer->getHeight();
    // 前半段淡出黑，后半段淡入黑
    Uint8 a = (m_progress < 0.5f) ? alpha : (255 - alpha);
    m_renderer->drawRect(0, 0, w, h, {0, 0, 0, a});
}

void Transition::renderFadeWhite(Uint8 alpha) {
    int w = m_renderer->getWidth();
    int h = m_renderer->getHeight();
    Uint8 a = (m_progress < 0.5f) ? alpha : (255 - alpha);
    m_renderer->drawRect(0, 0, w, h, {255, 255, 255, a});
}

void Transition::renderSlide(int direction) {
    int w = m_renderer->getWidth();
    int h = m_renderer->getHeight();
    // 遮挡条从一侧滑入
    int offset = static_cast<int>(m_progress * w);
    switch (direction) {
        case 0: m_renderer->drawRect(0, 0, offset, h, {0, 0, 0, 255}); break;          // left
        case 1: m_renderer->drawRect(w - offset, 0, offset, h, {0, 0, 0, 255}); break;  // right
        case 2: { int o = static_cast<int>(m_progress * h);
                  m_renderer->drawRect(0, 0, w, o, {0, 0, 0, 255}); break; }           // up
        case 3: { int o = static_cast<int>(m_progress * h);
                  m_renderer->drawRect(0, h - o, w, o, {0, 0, 0, 255}); break; }       // down
    }
}

void Transition::renderDissolve(Uint8 alpha) {
    int w = m_renderer->getWidth();
    int h = m_renderer->getHeight();
    // 简化：用半透明黑色覆盖
    Uint8 a = (m_progress < 0.5f) ? alpha : (255 - alpha);
    m_renderer->drawRect(0, 0, w, h, {0, 0, 0, a});
}

void Transition::renderBlind(float progress) {
    int w = m_renderer->getWidth();
    int h = m_renderer->getHeight();
    int blindH = h / m_blindCount;
    int coverH = static_cast<int>(blindH * progress);
    for (int i = 0; i < m_blindCount; ++i) {
        m_renderer->drawRect(0, i * blindH, w, coverH, {0, 0, 0, 255});
    }
}

void Transition::renderMosaic(float progress) {
    int w = m_renderer->getWidth();
    int h = m_renderer->getHeight();
    // 马赛克逐渐覆盖
    int size = m_mosaicSize;
    int cols = (w + size - 1) / size;
    int rows = (h + size - 1) / size;
    int totalBlocks = cols * rows;
    int visibleBlocks = static_cast<int>(totalBlocks * progress);

    int count = 0;
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            if (count < visibleBlocks) {
                m_renderer->drawRect(c * size, r * size, size, size, {0, 0, 0, 255});
            }
            ++count;
        }
    }
}

void Transition::renderCurtain(float progress) {
    int w = m_renderer->getWidth();
    int h = m_renderer->getHeight();
    // 左右窗帘向中间合拢
    int curtainW = static_cast<int>(w * 0.5f * progress);
    m_renderer->drawRect(0, 0, curtainW, h, {0, 0, 0, 255});
    m_renderer->drawRect(w - curtainW, 0, curtainW, h, {0, 0, 0, 255});
}

void Transition::renderZoom(float progress) {
    int w = m_renderer->getWidth();
    int h = m_renderer->getHeight();
    // 中心黑色圆形逐渐扩大
    int maxRadius = static_cast<int>(std::sqrt(w * w + h * h) * 0.5f);
    int radius = static_cast<int>(maxRadius * progress);

    // 简化：画四个角的黑色矩形
    int cx = w / 2, cy = h / 2;
    int halfSize = radius;

    // 上
    if (cy - halfSize > 0)
        m_renderer->drawRect(0, 0, w, cy - halfSize, {0, 0, 0, 255});
    // 下
    if (cy + halfSize < h)
        m_renderer->drawRect(0, cy + halfSize, w, h - (cy + halfSize), {0, 0, 0, 255});
    // 左
    if (cx - halfSize > 0)
        m_renderer->drawRect(0, 0, cx - halfSize, h, {0, 0, 0, 255});
    // 右
    if (cx + halfSize < w)
        m_renderer->drawRect(cx + halfSize, 0, w - (cx + halfSize), h, {0, 0, 0, 255});
}
