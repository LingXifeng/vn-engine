#pragma once

#include <SDL2/SDL.h>
#include <string>
#include <functional>
#include <memory>

// 自动/跳过模式控制器
class AutoSkip {
public:
    AutoSkip();
    ~AutoSkip();

    // --- 自动模式 ---
    void setAutoEnabled(bool enabled);
    bool isAutoEnabled() const { return m_autoEnabled; }

    // 自动模式速度（每句等待秒数，0=立即）
    void setAutoSpeed(float seconds) { m_autoSpeed = seconds; }
    float getAutoSpeed() const { return m_autoSpeed; }

    // 自动模式计时器（每帧更新，到达阈值时触发回调）
    void resetAutoTimer();
    bool autoTimerExpired(float dt);

    // --- 跳过模式 ---
    void setSkipEnabled(bool enabled);
    bool isSkipEnabled() const { return m_skipEnabled; }

    // 跳过模式：只跳过已读文本
    void setSkipOnlyRead(bool onlyRead) { m_skipOnlyRead = onlyRead; }
    bool getSkipOnlyRead() const { return m_skipOnlyRead; }

    // 跳过速度（每句等待毫秒）
    void setSkipSpeed(float ms) { m_skipSpeed = ms; }
    float getSkipSpeed() const { return m_skipSpeed; }

    // --- 通用 ---
    // 当文本显示完成时调用，返回是否应该自动推进
    bool shouldAdvance(bool textShown, bool isRead);

    // 切换自动模式
    void toggleAuto();
    // 切换跳过模式
    void toggleSkip();

    // 全部停止
    void stopAll();

    // 设置推进回调
    void setAdvanceCallback(std::function<void()> cb) { m_advanceCallback = cb; }

    // 更新
    void update(float dt);

    // 获取状态显示文本（用于UI）
    std::string getStatusText() const;

private:
    bool m_autoEnabled = false;
    float m_autoSpeed = 2.0f;      // 每句等待2秒
    float m_autoTimer = 0.0f;

    bool m_skipEnabled = false;
    bool m_skipOnlyRead = true;    // 默认只跳过已读
    float m_skipSpeed = 50.0f;     // 50ms per line

    std::function<void()> m_advanceCallback;
};
