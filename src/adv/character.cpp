#include "character.h"
#include <iostream>

Character::Character(Renderer* renderer, const std::string& name)
    : m_renderer(renderer), m_name(name) {}

Character::~Character() {}

void Character::addExpression(const std::string& exprName, std::shared_ptr<Texture> tex) {
    CharState state;
    state.expression = exprName;
    state.texture = tex;
    m_expressions[exprName] = state;
}

void Character::setExpression(const std::string& exprName) {
    auto it = m_expressions.find(exprName);
    if (it != m_expressions.end()) {
        m_currentExpr = exprName;
        m_texture = it->second.texture;
    } else {
        std::cerr << "Expression not found: " << exprName << " for character " << m_name << std::endl;
    }
}

void Character::setPosition(CharPosition pos) {
    m_position = pos;
    // 根据窗口宽度设置 X 坐标
    int screenW = m_renderer->getWidth();
    switch (pos) {
        case CharPosition::LEFT:   m_x = screenW * 0.2f; break;
        case CharPosition::CENTER: m_x = screenW * 0.5f; break;
        case CharPosition::RIGHT:  m_x = screenW * 0.8f; break;
        default: break;
    }
}

void Character::setCustomPosition(float x, float y) {
    m_position = CharPosition::CUSTOM;
    m_x = x;
    m_y = y;
}

void Character::show(TweenManager* tweens, float duration) {
    m_visible = true;
    m_hidden = false;
    if (tweens && duration > 0) {
        Uint8 targetAlpha = m_alpha;
        m_alpha = 0;
        tweens->add(duration,
            [this, targetAlpha](float t) {
                m_alpha = (Uint8)(targetAlpha * t);
            },
            EaseType::EASE_OUT_QUAD,
            [this, targetAlpha]() { m_alpha = targetAlpha; }
        );
    }
}

void Character::hide(TweenManager* tweens, float duration) {
    if (tweens && duration > 0) {
        Uint8 startAlpha = m_alpha;
        tweens->add(duration,
            [this, startAlpha](float t) {
                m_alpha = (Uint8)(startAlpha * (1.0f - t));
            },
            EaseType::EASE_OUT_QUAD,
            [this]() { m_visible = false; m_hidden = true; m_alpha = 0; }
        );
    } else {
        m_visible = false;
        m_hidden = true;
        m_alpha = 0;
    }
}

void Character::render(Renderer* renderer) {
    if (!m_visible || !m_texture) return;

    // 计算绘制位置（以底部为锚点）
    int texW = m_texture->width();
    int texH = m_texture->height();
    float drawX = m_x - (texW * m_scaleX) / 2.0f;  // 水平居中
    float drawY = m_baseY - texH * m_scaleY;       // 底部对齐

    renderer->drawTexture(m_texture.get(), drawX, drawY,
                          m_scaleX, m_scaleY, m_alpha, 0.0f, BlendMode::ALPHA);
}
