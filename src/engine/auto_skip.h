#pragma once

#include <string>
#include <functional>
#include <vector>

// 自动模式 / 跳过模式控制器
class AutoSkipController {
public:
    AutoSkipController();
    ~AutoSkipController();

    // 模式枚举
    enum Mode {
        NONE = 0,    // 无模式
        AUTO,        // 自动模式
        SKIP         // 跳过模式
    };

    // 设置模式
    void setAutoMode(bool enabled);
    void setSkipMode(bool enabled);
    void toggleAuto();
    void toggleSkip();
    void stop();  // 停止所有模式

    Mode getMode() const { return m_mode; }
    bool isAutoMode() const { return m_mode == AUTO; }
    bool isSkipMode() const { return m_mode == SKIP; }
    bool isActive() const { return m_mode != NONE; }

    // 速度控制
    void setAutoSpeed(float speed);  // 1.0=默认, 2.0=两倍速
    float getAutoSpeed() const { return m_autoSpeed; }

    void setSkipSpeed(float speed);
    float getSkipSpeed() const { return m_skipSpeed; }

    // 自动模式等待时间（文字显示完后等待多少秒）
    void setAutoWaitTime(float seconds) { m_autoWaitTime = seconds; }
    float getAutoWaitTime() const { return m_autoWaitTime; }

    // 更新（返回 true 表示应该推进到下一句）
    bool update(float dt, bool textFullyShown, bool isAlreadyRead);

    // 重置计时器
    void resetTimer();

    // 推进回调
    void setAdvanceCallback(std::function<void()> cb) { m_advanceCallback = cb; }

    // 已读文本记录
    void markRead(const std::string& scriptName, int line);
    bool isRead(const std::string& scriptName, int line) const;
    void clearReadHistory();

    // 获取模式名称（用于UI显示）
    std::string getModeName() const;

private:
    Mode m_mode = NONE;
    float m_autoSpeed = 1.0f;
    float m_skipSpeed = 5.0f;
    float m_autoWaitTime = 2.0f;  // 文字显示完后等待秒数

    // 计时器
    float m_autoTimer = 0.0f;
    float m_skipTimer = 0.0f;
    bool m_textShown = false;

    // 已读记录
    std::vector<std::pair<std::string, int>> m_readHistory;

    // 回调
    std::function<void()> m_advanceCallback;
};
