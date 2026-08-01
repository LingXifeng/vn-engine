#include "character_expression.h"
#include <iostream>

CharacterExpression::CharacterExpression(Renderer* renderer)
    : m_renderer(renderer) {}

CharacterExpression::~CharacterExpression() {}

void CharacterExpression::setBaseTexture(std::shared_ptr<Texture> tex) {
    m_baseTexture = tex;
}

void CharacterExpression::addDiff(const std::string& name, std::shared_ptr<Texture> tex,
                                  DiffBlendMode blendMode,
                                  float offsetX, float offsetY) {
    ExpressionDiff diff;
    diff.name = name;
    diff.texture = tex;
    diff.blendMode = blendMode;
    diff.offsetX = offsetX;
    diff.offsetY = offsetY;
    diff.alpha = 255;
    diff.visible = true;
    m_diffs[name] = diff;
}

void CharacterExpression::setExpression(const std::string& name, float transitionDuration) {
    if (name == m_currentExpr && !m_transition.active) return;

    // 启动过渡动画
    m_transition.fromExpr = m_currentExpr;
    m_transition.toExpr = name;
    m_transition.duration = transitionDuration;
    m_transition.elapsed = 0.0f;
    m_transition.active = true;

    m_currentExpr = name;

    // 更新激活的差分列表
    m_activeDiffs.clear();
    if (!name.empty() && m_diffs.find(name) != m_diffs.end()) {
        m_activeDiffs.push_back(name);
    }
}

void CharacterExpression::setExpressionInstant(const std::string& name) {
    m_transition.active = false;
    m_currentExpr = name;
    m_activeDiffs.clear();
    if (!name.empty() && m_diffs.find(name) != m_diffs.end()) {
        m_activeDiffs.push_back(name);
    }
}

std::vector<std::string> CharacterExpression::getAvailableExpressions() const {
    std::vector<std::string> names;
    names.reserve(m_diffs.size());
    for (const auto& [name, diff] : m_diffs) {
        names.push_back(name);
    }
    return names;
}

void CharacterExpression::setDiffVisible(const std::string& name, bool visible) {
    auto it = m_diffs.find(name);
    if (it != m_diffs.end()) {
        it->second.visible = visible;
    }
}

bool CharacterExpression::isDiffVisible(const std::string& name) const {
    auto it = m_diffs.find(name);
    if (it == m_diffs.end()) return false;
    return it->second.visible;
}

void CharacterExpression::setDiffAlpha(const std::string& name, Uint8 alpha) {
    auto it = m_diffs.find(name);
    if (it != m_diffs.end()) {
        it->second.alpha = alpha;
    }
}

Uint8 CharacterExpression::getDiffAlpha(const std::string& name) const {
    auto it = m_diffs.find(name);
    if (it == m_diffs.end()) return 0;
    return it->second.alpha;
}

void CharacterExpression::setDiffOffset(const std::string& name, float offsetX, float offsetY) {
    auto it = m_diffs.find(name);
    if (it != m_diffs.end()) {
        it->second.offsetX = offsetX;
        it->second.offsetY = offsetY;
    }
}

void CharacterExpression::setBlinking(bool enabled, float interval) {
    m_blinking = enabled;
    m_blinkInterval = interval;
    m_blinkTimer = 0.0f;
    m_blinkState = false;
}

void CharacterExpression::setLipSync(bool enabled) {
    m_lipSync = enabled;
    m_lipSyncTimer = 0.0f;
    m_lipSyncIndex = 0;
}

void CharacterExpression::setLipSyncPattern(const std::vector<std::string>& pattern) {
    m_lipSyncPattern = pattern;
    m_lipSyncIndex = 0;
}

bool CharacterExpression::loadDiffs(Renderer* renderer, const std::vector<DiffLoadInfo>& diffs) {
    bool allOk = true;
    for (const auto& info : diffs) {
        auto tex = renderer->loadTexture(info.path);
        if (tex) {
            addDiff(info.name, tex, info.blendMode, info.offsetX, info.offsetY);
        } else {
            std::cerr << "CharacterExpression: Failed to load diff: " << info.path << std::endl;
            allOk = false;
        }
    }
    return allOk;
}

void CharacterExpression::applyBlendMode(DiffBlendMode mode) {
    switch (mode) {
        case DiffBlendMode::ALPHA:
            SDL_SetRenderDrawBlendMode(m_renderer->getSDLRenderer(), SDL_BLENDMODE_BLEND);
            break;
        case DiffBlendMode::ADD:
            SDL_SetRenderDrawBlendMode(m_renderer->getSDLRenderer(), SDL_BLENDMODE_ADD);
            break;
        case DiffBlendMode::SUBTRACT:
            SDL_SetRenderDrawBlendMode(m_renderer->getSDLRenderer(), SDL_BLENDMODE_BLEND);
            break;
        case DiffBlendMode::OVERWRITE:
            SDL_SetRenderDrawBlendMode(m_renderer->getSDLRenderer(), SDL_BLENDMODE_NONE);
            break;
    }
}

void CharacterExpression::update(float dt) {
    // 更新过渡动画
    if (m_transition.active) {
        m_transition.elapsed += dt;
        float t = m_transition.elapsed / m_transition.duration;
        if (t >= 1.0f) {
            t = 1.0f;
            m_transition.active = false;
        }
        // 淡入淡出: 旧表情 alpha 从 255->0, 新表情 alpha 从 0->255
        m_oldAlpha = static_cast<Uint8>(255.0f * (1.0f - t));
        m_newAlpha = static_cast<Uint8>(255.0f * t);
    }

    // 更新闪烁
    if (m_blinking) {
        m_blinkTimer += dt;
        if (!m_blinkState) {
            // 睁眼状态
            if (m_blinkTimer >= m_blinkInterval) {
                m_blinkState = true;
                m_blinkTimer = 0.0f;
            }
        } else {
            // 闭眼状态
            if (m_blinkTimer >= m_blinkDuration) {
                m_blinkState = false;
                m_blinkTimer = 0.0f;
            }
        }
    }

    // 更新唇形同步
    if (m_lipSync && !m_lipSyncPattern.empty()) {
        m_lipSyncTimer += dt;
        if (m_lipSyncTimer >= m_lipSyncInterval) {
            m_lipSyncTimer = 0.0f;
            m_lipSyncIndex = (m_lipSyncIndex + 1) % static_cast<int>(m_lipSyncPattern.size());
        }
    }
}

void CharacterExpression::render(Renderer* renderer) {
    if (!m_baseTexture) return;

    // 渲染基础立绘
    renderer->drawTexture(m_baseTexture.get(), m_x, m_y,
                          m_scaleX, m_scaleY, m_alpha);

    // 渲染旧表情（过渡中淡出）
    if (m_transition.active && !m_transition.fromExpr.empty()) {
        auto it = m_diffs.find(m_transition.fromExpr);
        if (it != m_diffs.end() && it->second.texture) {
            auto& diff = it->second;
            Uint8 alpha = static_cast<Uint8>(
                static_cast<float>(diff.alpha) * static_cast<float>(m_oldAlpha) / 255.0f);
            renderer->drawTexture(diff.texture.get(),
                                  m_x + diff.offsetX, m_y + diff.offsetY,
                                  m_scaleX, m_scaleY, alpha);
        }
    }

    // 渲染当前激活的差分
    for (const auto& diffName : m_activeDiffs) {
        auto it = m_diffs.find(diffName);
        if (it == m_diffs.end()) continue;

        auto& diff = it->second;
        if (!diff.visible || !diff.texture) continue;

        // 计算透明度
        Uint8 alpha = diff.alpha;
        if (m_transition.active) {
            alpha = static_cast<Uint8>(
                static_cast<float>(alpha) * static_cast<float>(m_newAlpha) / 255.0f);
        }
        alpha = static_cast<Uint8>(
            static_cast<float>(alpha) * static_cast<float>(m_alpha) / 255.0f);

        // 闪烁处理：闭眼时渲染闭眼差分，否则跳过眼部差分
        if (m_blinking && m_blinkState && diffName.find("eye") != std::string::npos) {
            // 闭眼状态，跳过睁眼差分
            auto blinkIt = m_diffs.find(diffName + "_closed");
            if (blinkIt != m_diffs.end() && blinkIt->second.texture) {
                renderer->drawTexture(blinkIt->second.texture.get(),
                                      m_x + blinkIt->second.offsetX,
                                      m_y + blinkIt->second.offsetY,
                                      m_scaleX, m_scaleY, alpha);
                continue;
            }
        }

        // 唇形同步处理
        if (m_lipSync && !m_lipSyncPattern.empty() && diffName.find("mouth") != std::string::npos) {
            std::string lipExpr = m_lipSyncPattern[m_lipSyncIndex];
            auto lipIt = m_diffs.find(lipExpr);
            if (lipIt != m_diffs.end() && lipIt->second.texture) {
                renderer->drawTexture(lipIt->second.texture.get(),
                                      m_x + lipIt->second.offsetX,
                                      m_y + lipIt->second.offsetY,
                                      m_scaleX, m_scaleY, alpha);
                continue;
            }
        }

        // 设置混合模式并渲染
        applyBlendMode(diff.blendMode);
        renderer->drawTexture(diff.texture.get(),
                              m_x + diff.offsetX, m_y + diff.offsetY,
                              m_scaleX, m_scaleY, alpha);
        // 恢复默认混合模式
        SDL_SetRenderDrawBlendMode(renderer->getSDLRenderer(), SDL_BLENDMODE_BLEND);
    }
}
