#include "music_room.h"
#include <algorithm>
#include <sstream>

MusicRoom::MusicRoom(Renderer* renderer, Audio* audio)
    : m_renderer(renderer), m_audio(audio) {}
MusicRoom::~MusicRoom() {}

void MusicRoom::addBGM(const std::string& id, const std::string& title,
                       const std::string& artist, const std::string& filePath) {
    BGMEntry entry;
    entry.id = id;
    entry.title = title;
    entry.artist = artist;
    entry.filePath = filePath;
    entry.unlocked = false;
    m_bgms.push_back(entry);
}

void MusicRoom::unlockBGM(const std::string& id) {
    for (auto& bgm : m_bgms) if (bgm.id == id) { bgm.unlocked = true; return; }
}

bool MusicRoom::isBGMUnlocked(const std::string& id) const {
    for (const auto& bgm : m_bgms) if (bgm.id == id) return bgm.unlocked;
    return false;
}

void MusicRoom::addVoice(const std::string& id, const std::string& speaker,
                         const std::string& text, const std::string& filePath) {
    VoiceEntry entry;
    entry.id = id;
    entry.speaker = speaker;
    entry.text = text;
    entry.filePath = filePath;
    entry.unlocked = false;
    m_voices.push_back(entry);
}

void MusicRoom::unlockVoice(const std::string& id) {
    for (auto& v : m_voices) if (v.id == id) { v.unlocked = true; return; }
}

std::vector<std::string> MusicRoom::getUnlockedBGMIDs() const {
    std::vector<std::string> ids;
    for (const auto& bgm : m_bgms) if (bgm.unlocked) ids.push_back(bgm.id);
    return ids;
}

std::vector<std::string> MusicRoom::getUnlockedVoiceIDs() const {
    std::vector<std::string> ids;
    for (const auto& v : m_voices) if (v.unlocked) ids.push_back(v.id);
    return ids;
}

void MusicRoom::setUnlockedBGMIDs(const std::vector<std::string>& ids) {
    for (const auto& id : ids) unlockBGM(id);
}

void MusicRoom::setUnlockedVoiceIDs(const std::vector<std::string>& ids) {
    for (const auto& id : ids) unlockVoice(id);
}

void MusicRoom::show() {
    m_visible = true;
    m_fadeAlpha = 0.0f;
    m_currentTab = Tab::BGM;
    m_bgmHoverIndex = -1;
    m_bgmSelectedIndex = -1;
    m_voiceHoverIndex = -1;
    m_voiceSelectedIndex = -1;
    m_bgmScrollOffset = 0;
    m_voiceScrollOffset = 0;
}

void MusicRoom::hide() {
    m_visible = false;
    if (m_bgmPlaying) { m_audio->stopBGM(); m_bgmPlaying = false; }
    if (m_voicePlaying) { m_audio->stopVoice(); m_voicePlaying = false; }
}

int MusicRoom::getItemIndexAt(int x, int y) const {
    if (x < m_listX || x > m_listX + m_listW) return -1;
    if (y < m_listY || y > m_listY + m_visibleItems * m_itemH) return -1;
    int idx = (y - m_listY) / m_itemH;
    if (m_currentTab == Tab::BGM)
        idx += m_bgmScrollOffset;
    else
        idx += m_voiceScrollOffset;
    return idx;
}

std::string MusicRoom::formatTime(int seconds) const {
    int m = seconds / 60;
    int s = seconds % 60;
    std::ostringstream oss;
    oss << m << ":";
    if (s < 10) oss << "0";
    oss << s;
    return oss.str();
}

void MusicRoom::update(float dt, const Input& input) {
    if (!m_visible) return;
    m_fadeAlpha = std::min(m_fadeAlpha + 3.0f * dt, 1.0f);

    int mx, my;
    input.getMousePosition(mx, my);

    // 标签页切换
    if (input.isMouseButtonPressed(1)) {
        int tabY = 40;
        if (my >= tabY && my < tabY + 30) {
            if (mx >= m_listX && mx < m_listX + 100) m_currentTab = Tab::BGM;
            else if (mx >= m_listX + 110 && mx < m_listX + 220) m_currentTab = Tab::VOICE;
        }
    }

    // 列表交互
    int hoverIdx = getItemIndexAt(mx, my);

    if (m_currentTab == Tab::BGM) {
        m_bgmHoverIndex = hoverIdx;
        int totalItems = (int)m_bgms.size();

        if (input.isMouseButtonPressed(1) && hoverIdx >= 0 && hoverIdx < totalItems) {
            if (m_bgms[hoverIdx].unlocked) {
                m_bgmSelectedIndex = hoverIdx;
                m_audio->playBGM(m_bgms[hoverIdx].filePath, true);
                m_bgmPlaying = true;
                m_playTime = 0.0f;
            }
        }

        // 滚轮
        int wheel = input.getMouseWheelY();
        if (wheel != 0) {
            m_bgmScrollOffset = std::clamp(m_bgmScrollOffset - wheel, 0,
                                            std::max(0, totalItems - m_visibleItems));
        }

        // 停止播放
        if (input.isKeyPressed(SDL_SCANCODE_SPACE) && m_bgmPlaying) {
            m_audio->stopBGM();
            m_bgmPlaying = false;
        }

        if (m_bgmPlaying) m_playTime += dt;

    } else {
        m_voiceHoverIndex = hoverIdx;
        int totalItems = (int)m_voices.size();

        if (input.isMouseButtonPressed(1) && hoverIdx >= 0 && hoverIdx < totalItems) {
            if (m_voices[hoverIdx].unlocked) {
                m_voiceSelectedIndex = hoverIdx;
                m_audio->playVoice(m_voices[hoverIdx].filePath);
                m_voicePlaying = true;
                m_playTime = 0.0f;
            }
        }

        int wheel = input.getMouseWheelY();
        if (wheel != 0) {
            m_voiceScrollOffset = std::clamp(m_voiceScrollOffset - wheel, 0,
                                              std::max(0, totalItems - m_visibleItems));
        }

        if (m_voicePlaying) m_playTime += dt;
    }

    // ESC 退出
    if (input.isKeyPressed(SDL_SCANCODE_ESCAPE)) hide();
}

void MusicRoom::render() {
    if (!m_visible) return;
    Uint8 alpha = (Uint8)(m_fadeAlpha * 255);

    // 背景
    m_renderer->drawRect(0, 0, m_renderer->getWidth(), m_renderer->getHeight(),
                         {15, 15, 25, alpha}, true);

    renderTabs();

    if (m_currentTab == Tab::BGM) renderBGMList();
    else renderVoiceList();

    renderPlaybackBar();
}

void MusicRoom::renderTabs() {
    if (!m_font) return;
    Uint8 alpha = (Uint8)(m_fadeAlpha * 255);

    // BGM 标签
    SDL_Color bgmColor = (m_currentTab == Tab::BGM) ? SDL_Color{255, 220, 100, alpha} : SDL_Color{120, 120, 120, alpha};
    auto bgmTex = m_renderer->renderText("BGM", m_font, bgmColor);
    if (bgmTex) m_renderer->drawTexture(bgmTex.get(), m_listX, 40, 1.0f, 1.0f, alpha);

    // Voice 标签
    SDL_Color voiceColor = (m_currentTab == Tab::VOICE) ? SDL_Color{255, 220, 100, alpha} : SDL_Color{120, 120, 120, alpha};
    auto voiceTex = m_renderer->renderText("Voice", m_font, voiceColor);
    if (voiceTex) m_renderer->drawTexture(voiceTex.get(), m_listX + 110, 40, 1.0f, 1.0f, alpha);
}

void MusicRoom::renderBGMList() {
    if (!m_font) return;
    Uint8 alpha = (Uint8)(m_fadeAlpha * 255);

    int sw = m_renderer->getWidth();

    for (int i = 0; i < m_visibleItems; i++) {
        int idx = i + m_bgmScrollOffset;
        if (idx >= (int)m_bgms.size()) break;

        const auto& bgm = m_bgms[idx];
        int y = m_listY + i * m_itemH;

        // 选中/悬停高亮
        if (idx == m_bgmSelectedIndex) {
            m_renderer->drawRect(m_listX, y, m_listW, m_itemH - 2, {60, 80, 120, alpha}, true);
        } else if (idx == m_bgmHoverIndex && bgm.unlocked) {
            m_renderer->drawRect(m_listX, y, m_listW, m_itemH - 2, {40, 40, 60, alpha}, true);
        }

        if (bgm.unlocked) {
            // 曲名
            auto titleTex = m_renderer->renderText(bgm.title, m_font, {220, 220, 220, alpha});
            if (titleTex) m_renderer->drawTexture(titleTex.get(), m_listX + 10, y + 8, 1.0f, 1.0f, alpha);

            // 艺术家
            if (!bgm.artist.empty()) {
                auto artistTex = m_renderer->renderText(bgm.artist, m_font, {150, 150, 150, alpha});
                if (artistTex) m_renderer->drawTexture(artistTex.get(), m_listX + 350, y + 8, 1.0f, 1.0f, alpha);
            }

            // 时长
            if (bgm.duration > 0) {
                auto durTex = m_renderer->renderText(formatTime(bgm.duration), m_font, {150, 150, 150, alpha});
                if (durTex) m_renderer->drawTexture(durTex.get(), m_listX + m_listW - 60, y + 8, 1.0f, 1.0f, alpha);
            }
        } else {
            auto lockTex = m_renderer->renderText("???", m_font, {60, 60, 60, alpha});
            if (lockTex) m_renderer->drawTexture(lockTex.get(), m_listX + 10, y + 8, 1.0f, 1.0f, alpha);
        }
    }
}

void MusicRoom::renderVoiceList() {
    if (!m_font) return;
    Uint8 alpha = (Uint8)(m_fadeAlpha * 255);

    for (int i = 0; i < m_visibleItems; i++) {
        int idx = i + m_voiceScrollOffset;
        if (idx >= (int)m_voices.size()) break;

        const auto& v = m_voices[idx];
        int y = m_listY + i * m_itemH;

        if (idx == m_voiceSelectedIndex) {
            m_renderer->drawRect(m_listX, y, m_listW, m_itemH - 2, {60, 80, 120, alpha}, true);
        } else if (idx == m_voiceHoverIndex && v.unlocked) {
            m_renderer->drawRect(m_listX, y, m_listW, m_itemH - 2, {40, 40, 60, alpha}, true);
        }

        if (v.unlocked) {
            // 说话者
            auto speakerTex = m_renderer->renderText(v.speaker, m_font, {255, 200, 100, alpha});
            if (speakerTex) m_renderer->drawTexture(speakerTex.get(), m_listX + 10, y + 8, 1.0f, 1.0f, alpha);

            // 文本（截断显示）
            std::string displayText = v.text;
            if (displayText.length() > 40) displayText = displayText.substr(0, 37) + "...";
            auto textTex = m_renderer->renderText(displayText, m_font, {200, 200, 200, alpha});
            if (textTex) m_renderer->drawTexture(textTex.get(), m_listX + 120, y + 8, 1.0f, 1.0f, alpha);
        } else {
            auto lockTex = m_renderer->renderText("???", m_font, {60, 60, 60, alpha});
            if (lockTex) m_renderer->drawTexture(lockTex.get(), m_listX + 10, y + 8, 1.0f, 1.0f, alpha);
        }
    }
}

void MusicRoom::renderPlaybackBar() {
    if (!m_font) return;
    Uint8 alpha = (Uint8)(m_fadeAlpha * 255);
    int sh = m_renderer->getHeight();
    int sw = m_renderer->getWidth();

    int barY = sh - 60;

    // 播放状态
    bool playing = (m_currentTab == Tab::BGM) ? m_bgmPlaying : m_voicePlaying;
    std::string statusText = playing ? "Now Playing..." : "Stopped";
    auto statusTex = m_renderer->renderText(statusText, m_font, {180, 180, 180, alpha});
    if (statusTex) m_renderer->drawTexture(statusTex.get(), m_listX, barY, 1.0f, 1.0f, alpha);

    // 播放时间
    if (playing) {
        std::string timeStr = formatTime((int)m_playTime);
        auto timeTex = m_renderer->renderText(timeStr, m_font, {180, 180, 180, alpha});
        if (timeTex) m_renderer->drawTexture(timeTex.get(), sw - 100, barY, 1.0f, 1.0f, alpha);
    }

    // 提示
    auto hintTex = m_renderer->renderText("ESC: Exit  SPACE: Stop  Wheel: Scroll", m_font, {100, 100, 100, alpha});
    if (hintTex) m_renderer->drawTexture(hintTex.get(), m_listX, barY + 25, 1.0f, 1.0f, alpha);
}
