#include "tween.h"

bool Tween::update(float dt) {
    if (m_done) return true;
    m_elapsed += dt;
    float t = std::min(m_elapsed / m_duration, 1.0f);
    float eased = ease(m_easeType, t);
    if (m_updateCallback) m_updateCallback(eased);
    if (t >= 1.0f) {
        m_done = true;
        if (m_completeCallback) m_completeCallback();
        return true;
    }
    return false;
}

std::shared_ptr<Tween> TweenManager::add(float duration,
                                         std::function<void(float)> updateCb,
                                         EaseType ease,
                                         Tween::Callback completeCb) {
    auto tween = std::make_shared<Tween>(duration, ease);
    tween->onUpdate(updateCb);
    if (completeCb) tween->onComplete(completeCb);
    m_tweens.push_back(tween);
    return tween;
}

void TweenManager::update(float dt) {
    for (auto& tween : m_tweens) {
        tween->update(dt);
    }
    m_tweens.erase(
        std::remove_if(m_tweens.begin(), m_tweens.end(),
            [](const std::shared_ptr<Tween>& t) { return t->isDone(); }),
        m_tweens.end()
    );
}
