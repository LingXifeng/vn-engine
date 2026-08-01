#include "auto_skip.h"
#include <iostream>

AutoSkip::AutoSkip() {}

AutoSkip::~AutoSkip() {}

void AutoSkip::setAutoEnabled(bool enabled) {
    m_autoEnabled = enabled;
    if (enabled) {
        m_skipEnabled = false;  // 互斥
        resetAutoTimer();
    }
}

void AutoSkip::resetAutoTimer() {
    m_autoTimer = 0.0f;
}

bool AutoSkip::autoTimerExpired(float dt) {
    if (!m_autoEnabled) return false;
    m_autoTimer += dt;
    if (m_autoTimer >= m_autoSpeed) {
        m_autoTimer = 0.0f;
        return true;
    }
    return false;
}

void AutoSkip::setSkipEnabled(bool enabled) {
    m_skipEnabled = enabled;
    if (enabled) {
        m_autoEnabled = false;  // 互斥
    }
}

bool AutoSkip::shouldAdvance(bool textShown, bool isRead) {
    // 自动模式：文本显示完后自动推进
    if (m_autoEnabled && textShown) {
        return true;
    }
    // 跳过模式：只跳过已读文本（如果设置了onlyRead）
    if (m_skipEnabled) {
        if (!m_skipOnlyRead || isRead) {
            return true;
        }
    }
    return false;
}

void AutoSkip::toggleAuto() {
    setAutoEnabled(!m_autoEnabled);
}

void AutoSkip::toggleSkip() {
    setSkipEnabled(!m_skipEnabled);
}

void AutoSkip::stopAll() {
    m_autoEnabled = false;
    m_skipEnabled = false;
    m_autoTimer = 0.0f;
}

void AutoSkip::update(float dt) {
    if (m_autoEnabled) {
        if (autoTimerExpired(dt)) {
            if (m_advanceCallback) {
                m_advanceCallback();
            }
        }
    }
}

std::string AutoSkip::getStatusText() const {
    if (m_autoEnabled) {
        return "Auto";
    }
    if (m_skipEnabled) {
        return "Skip";
    }
    return "";
}
