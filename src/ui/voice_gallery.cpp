#include "voice_gallery.h"
#include <algorithm>

VoiceGallery::VoiceGallery(Renderer* renderer, Audio* audio)
    : m_renderer(renderer), m_audio(audio) {}
VoiceGallery::~VoiceGallery() {}

void VoiceGallery::addVoice(const std::string& id, const std::string& speaker,
                             const std::string& text, const std::string& filePath,
                             const std::string& sceneName) {
    VoiceRecallEntry entry;
    entry.id = id;
    entry.speaker = speaker;
    entry.text = text;
    entry.filePath = filePath;
    entry.sceneName = sceneName;
    entry.unlocked = false;
    m_voices.push_back(entry);

    // 收集说话者列表
    if (std::find(m_speakers.begin(), m_speakers.end(), speaker) == m_speakers.end()) {
        m_speakers.push_back(speaker);
    }
}

void VoiceGallery::unlockVoice(const std::string& id) {
    for (auto& v : m_voices) if (v.id == id) { v.unlocked = true; return; }
}

void VoiceGallery::unlockAll() {
    for (auto& v : m_voices) v.unlocked = true;
}

std::vector<std::string> VoiceGallery::getUnlockedIDs() const {
    std::vector<std::string> ids;
    for (const auto& v : m_voices) if (v.unlocked) ids.push_back(v.id);
    return ids;
}

void VoiceGallery::setUnlockedIDs(const std::vector<std::string>& ids) {
    for (const auto& id : ids) unlockVoice(id);
}

void VoiceGallery::show() {
    m_visible = true;
    m_fadeAlpha = 0.0f;
    m_hoverIndex = -1;
    m_selectedIndex = -1;
    m_scrollOffset = 0;
    m_filterSpeaker.clear();
}

void VoiceGallery::hide() {
    m_visible = false;
    m_audio->stopVoice();
}

std::string VoiceGallery::truncate(const std::string& s, size_t maxLen) const {
    if (s.length() <= maxLen) return s;
    return s.substr(0, maxLen - 3) + "...";
}

std::vector<int> VoiceGallery::getFilteredIndices() const {
    std::vector<int> indices;
    for (int i = 0; i < (int)m_voices.size(); i++) {
        if (!m_filterSpeaker.empty() && m_voices[i].speaker != m_filterSpeaker) continue;
        indices.push_back(i);
    }
    return indices;
}

int VoiceGallery::getFilteredIndexAt(int x, int y) const {
    if (x < m_listX || x > m_listX + m_listW) return -1;
    if (y < m_listY || y > m_listY + m_visibleItems * m_itemH) return -1;

    auto filtered = getFilteredIndices();
    int localIdx = (y - m_listY) / m_itemH;
    int globalIdx = localIdx + m_scrollOffset;
    if (globalIdx < 0 || globalIdx >= (int)filtered.size()) return -1;
    return filtered[globalIdx];
}

void VoiceGallery::update(float dt, const Input& input) {
    if (!m_visible) return;
    m_fadeAlpha = std::min(m_fadeAlpha + 3.0f * dt, 1.0f);

    int mx, my;
    input.getMousePosition(mx, my);

    m_hoverIndex = getFilteredIndexAt(mx, my);

    // 点击播放
    if (input.isMouseButtonPressed(1) && m_hoverIndex >= 0) {
        if (m_voices[m_hoverIndex].unlocked) {
            m_selectedIndex = m_hoverIndex;
            m_audio->playVoice(m_voices[m_hoverIndex].filePath);
        }
    }

    // 滚轮
    auto filtered = getFilteredIndices();
    int totalItems = (int)filtered.size();
    int wheel = input.getMouseWheelY();
    if (wheel != 0) {
        m_scrollOffset = std::clamp(m_scrollOffset - wheel, 0,
                                    std::max(0, totalItems - m_visibleItems));
    }

    // 切换说话者过滤（左右键）
    if (input.isKeyPressed(SDL_SCANCODE_RIGHT) || input.isKeyPressed(SDL_SCANCODE_LEFT)) {
        if (m_speakers.empty()) return;
        size_t currentIdx = 0;
        for (size_t i = 0; i < m_speakers.size(); i++) {
            if (m_speakers[i] == m_filterSpeaker) { currentIdx = i; break; }
        }
        if (input.isKeyPressed(SDL_SCANCODE_RIGHT))
            currentIdx = (currentIdx + 1) % (m_speakers.size() + 1);
        else
            currentIdx = (currentIdx + m_speakers.size()) % (m_speakers.size() + 1);
        m_filterSpeaker = (currentIdx < m_speakers.size()) ? m_speakers[currentIdx] : "";
        m_scrollOffset = 0;
    }

    // ESC 退出
    if (input.isKeyPressed(SDL_SCANCODE_ESCAPE)) hide();
}

void VoiceGallery::render() {
    if (!m_visible) return;
    Uint8 alpha = (Uint8)(m_fadeAlpha * 255);

    m_renderer->drawRect(0, 0, m_renderer->getWidth(), m_renderer->getHeight(),
                         {15, 15, 25, alpha}, true);

    if (m_font) {
        // 标题
        std::string title = "Voice Recall";
        if (!m_filterSpeaker.empty()) title += " - " + m_filterSpeaker;
        auto titleTex = m_renderer->renderText(title, m_font, {255, 220, 100, alpha});
        if (titleTex) m_renderer->drawTexture(titleTex.get(),
            (m_renderer->getWidth() - titleTex->width()) / 2, 20, 1.0f, 1.0f, alpha);
    }

    renderList();
    renderDetail();
}

void VoiceGallery::renderList() {
    if (!m_font) return;
    Uint8 alpha = (Uint8)(m_fadeAlpha * 255);

    auto filtered = getFilteredIndices();

    for (int i = 0; i < m_visibleItems; i++) {
        int filteredIdx = i + m_scrollOffset;
        if (filteredIdx >= (int)filtered.size()) break;

        int idx = filtered[filteredIdx];
        const auto& v = m_voices[idx];
        int y = m_listY + i * m_itemH;

        // 高亮
        if (idx == m_selectedIndex) {
            m_renderer->drawRect(m_listX, y, m_listW, m_itemH - 4, {60, 80, 120, alpha}, true);
        } else if (idx == m_hoverIndex && v.unlocked) {
            m_renderer->drawRect(m_listX, y, m_listW, m_itemH - 4, {40, 40, 60, alpha}, true);
        }

        if (v.unlocked) {
            // 说话者
            auto speakerTex = m_renderer->renderText(v.speaker, m_font, {255, 200, 100, alpha});
            if (speakerTex) m_renderer->drawTexture(speakerTex.get(), m_listX + 10, y + 5, 1.0f, 1.0f, alpha);

            // 台词
            auto textTex = m_renderer->renderText(truncate(v.text, 45), m_font, {200, 200, 200, alpha});
            if (textTex) m_renderer->drawTexture(textTex.get(), m_listX + 120, y + 5, 1.0f, 1.0f, alpha);

            // 场景名
            if (!v.sceneName.empty()) {
                auto sceneTex = m_renderer->renderText(v.sceneName, m_font, {120, 120, 140, alpha});
                if (sceneTex) m_renderer->drawTexture(sceneTex.get(), m_listX + 10, y + 28, 1.0f, 1.0f, alpha);
            }
        } else {
            auto lockTex = m_renderer->renderText("???", m_font, {60, 60, 60, alpha});
            if (lockTex) m_renderer->drawTexture(lockTex.get(), m_listX + 10, y + 15, 1.0f, 1.0f, alpha);
        }
    }
}

void VoiceGallery::renderDetail() {
    if (!m_font || m_selectedIndex < 0 || m_selectedIndex >= (int)m_voices.size()) return;
    Uint8 alpha = (Uint8)(m_fadeAlpha * 255);

    const auto& v = m_voices[m_selectedIndex];
    int sh = m_renderer->getHeight();
    int sw = m_renderer->getWidth();

    int detailY = sh - 120;

    // 背景条
    m_renderer->drawRect(0, detailY, sw, 120, {30, 30, 40, alpha}, true);

    // 说话者
    auto speakerTex = m_renderer->renderText(v.speaker, m_font, {255, 220, 100, alpha});
    if (speakerTex) m_renderer->drawTexture(speakerTex.get(), 50, detailY + 10, 1.0f, 1.0f, alpha);

    // 完整台词
    auto textTex = m_renderer->renderTextWrapped(v.text, m_font, sw - 100, 4, {220, 220, 220, alpha});
    if (textTex) m_renderer->drawTexture(textTex.get(), 50, detailY + 40, 1.0f, 1.0f, alpha);
}
