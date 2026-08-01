#include "auto_skip.h"
#include <algorithm>

AutoSkipController::AutoSkipController() {
}

AutoSkipController::~AutoSkipController() {
}

void AutoSkipController::setAutoMode(bool enabled) {
    if (enabled) {
        m_mode = AUTO;
    } else if (m_mode == AUTO) {
        m_mode = NONE;
    }
    resetTimer();
}

void AutoSkipController::setSkipMode(bool enabled) {
    if (enabled) {
        m_mode = SKIP;
    } else if (m_mode == SKIP) {
        m_mode = NONE;
    }
    resetTimer();
}

void AutoSkipController::toggleAuto() {
    if (m_mode == AUTO) {
        m_mode = NONE;
    } else {
        m_mode = AUTO;
    }
    resetTimer();
}

void AutoSkipController::toggleSkip() {
    if (m_mode == SKIP) {
        m_mode = NONE;
    } else {
        m_mode = SKIP;
    }
    resetTimer();
}

void AutoSkipController::stop() {
    m_mode = NONE;
    resetTimer();
}

void AutoSkipController::setAutoSpeed(float speed) {
    m_autoSpeed = std::max(0.1f, speed);
}

void AutoSkipController::setSkipSpeed(float speed) {
    m_skipSpeed = std::max(1.0f, speed);
}

bool AutoSkipController::update(float dt, bool textFullyShown, bool isAlreadyRead) {
    if (m_mode == NONE) return false;

    if (m_mode == AUTO) {
        // 自动模式：文字显示完后等待一段时间再推进
        if (textFullyShown) {
            m_autoTimer += dt * m_autoSpeed;
            if (m_autoTimer >= m_autoWaitTime) {
                m_autoTimer = 0.0f;
                if (m_advanceCallback) m_advanceCallback();
                return true;
            }
        }
        return false;
    }

    if (m_mode == SKIP) {
        // 跳过模式：只跳过已读文本，快速推进
        if (!isAlreadyRead) {
            // 遇到未读文本，自动停止跳过
            stop();
            return false;
        }
        m_skipTimer += dt * m_skipSpeed;
        if (m_skipTimer >= 0.1f) {  // 每0.1秒推进一次（受速度影响）
            m_skipTimer = 0.0f;
            if (m_advanceCallback) m_advanceCallback();
            return true;
        }
        return false;
    }

    return false;
}

void AutoSkipController::resetTimer() {
    m_autoTimer = 0.0f;
    m_skipTimer = 0.0f;
    m_textShown = false;
}

void AutoSkipController::markRead(const std::string& scriptName, int line) {
    // 检查是否已存在
    for (const auto& entry : m_readHistory) {
        if (entry.first == scriptName && entry.second == line) {
            return;  // 已存在
        }
    }
    m_readHistory.emplace_back(scriptName, line);
}

bool AutoSkipController::isRead(const std::string& scriptName, int line) const {
    for (const auto& entry : m_readHistory) {
        if (entry.first == scriptName && entry.second == line) {
            return true;
        }
    }
    return false;
}

void AutoSkipController::clearReadHistory() {
    m_readHistory.clear();
}

std::string AutoSkipController::getModeName() const {
    switch (m_mode) {
        case AUTO: return "Auto";
        case SKIP: return "Skip";
        default: return "";
    }
}
