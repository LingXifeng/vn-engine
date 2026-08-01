#pragma once

#include <SDL2/SDL.h>
#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include "renderer.h"
#include "tween.h"

// 角色表情差分系统
// 支持基础立绘 + 表情差分图层叠加
// 差分模式: 加算(ADD) / Alpha混合(ALPHA) / 覆盖(OVERWRITE)

enum class DiffBlendMode {
    ALPHA,      // Alpha 混合
    ADD,        // 加算
    SUBTRACT,   // 减算
    OVERWRITE   // 覆盖
};

// 表情差分图层
struct ExpressionDiff {
    std::string name;           // 差分名 (如 "smile", "angry", "surprised")
    std::shared_ptr<Texture> texture;
    DiffBlendMode blendMode = DiffBlendMode::ALPHA;
    Uint8 alpha = 255;          // 透明度
    float offsetX = 0.0f;       // X 偏移
    float offsetY = 0.0f;       // Y 偏移
    bool visible = true;
};

// 差分过渡动画
struct DiffTransition {
    std::string fromExpr;
    std::string toExpr;
    float duration = 0.3f;
    float elapsed = 0.0f;
    bool active = false;
};

// 角色表情管理器
class CharacterExpression {
public:
    CharacterExpression(Renderer* renderer);
    ~CharacterExpression();

    // 设置角色基础立绘
    void setBaseTexture(std::shared_ptr<Texture> tex);
    std::shared_ptr<Texture> getBaseTexture() const { return m_baseTexture; }

    // 添加表情差分
    void addDiff(const std::string& name, std::shared_ptr<Texture> tex,
                 DiffBlendMode blendMode = DiffBlendMode::ALPHA,
                 float offsetX = 0.0f, float offsetY = 0.0f);

    // 设置当前表情（带过渡动画）
    void setExpression(const std::string& name, float transitionDuration = 0.3f);

    // 瞬间切换表情（无动画）
    void setExpressionInstant(const std::string& name);

    // 获取当前表情
    const std::string& getCurrentExpression() const { return m_currentExpr; }

    // 获取所有可用表情
    std::vector<std::string> getAvailableExpressions() const;

    // 表情差分可见性
    void setDiffVisible(const std::string& name, bool visible);
    bool isDiffVisible(const std::string& name) const;

    // 差分透明度
    void setDiffAlpha(const std::string& name, Uint8 alpha);
    Uint8 getDiffAlpha(const std::string& name) const;

    // 差分偏移
    void setDiffOffset(const std::string& name, float offsetX, float offsetY);

    // 位置与变换
    void setPosition(float x, float y) { m_x = x; m_y = y; }
    void getPosition(float& x, float& y) const { x = m_x; y = m_y; }
    void setScale(float sx, float sy) { m_scaleX = sx; m_scaleY = sy; }
    void setAlpha(Uint8 alpha) { m_alpha = alpha; }
    Uint8 getAlpha() const { return m_alpha; }

    // 闪烁效果（用于角色说话时）
    void setBlinking(bool enabled, float interval = 3.0f);
    bool isBlinking() const { return m_blinking; }

    // 唇形同步（用于语音播放时口型动画）
    void setLipSync(bool enabled);
    bool isLipSyncEnabled() const { return m_lipSync; }
    void setLipSyncPattern(const std::vector<std::string>& pattern);

    // 更新与渲染
    void update(float dt);
    void render(Renderer* renderer);

    // 批量加载表情差分（从文件路径列表）
    struct DiffLoadInfo {
        std::string name;
        std::string path;
        DiffBlendMode blendMode = DiffBlendMode::ALPHA;
        float offsetX = 0.0f;
        float offsetY = 0.0f;
    };
    bool loadDiffs(Renderer* renderer, const std::vector<DiffLoadInfo>& diffs);

private:
    Renderer* m_renderer;
    std::shared_ptr<Texture> m_baseTexture;

    std::unordered_map<std::string, ExpressionDiff> m_diffs;
    std::string m_currentExpr;
    std::vector<std::string> m_activeDiffs;  // 当前激活的差分名列表

    float m_x = 0.0f, m_y = 0.0f;
    float m_scaleX = 1.0f, m_scaleY = 1.0f;
    Uint8 m_alpha = 255;

    // 过渡动画
    DiffTransition m_transition;
    Uint8 m_oldAlpha = 255;
    Uint8 m_newAlpha = 0;

    // 闪烁
    bool m_blinking = false;
    float m_blinkInterval = 3.0f;
    float m_blinkTimer = 0.0f;
    bool m_blinkState = false;  // false=睁眼, true=闭眼
    float m_blinkDuration = 0.1f;

    // 唇形同步
    bool m_lipSync = false;
    std::vector<std::string> m_lipSyncPattern;
    int m_lipSyncIndex = 0;
    float m_lipSyncTimer = 0.0f;
    float m_lipSyncInterval = 0.08f;

    void applyBlendMode(DiffBlendMode mode);
};
