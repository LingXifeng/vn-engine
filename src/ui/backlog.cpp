#include "backlog.h"
#include <algorithm>
#include <cmath>

Backlog::Backlog(Renderer* renderer, Audio* audio)
    : m_renderer(renderer), m_audio(audio) {
}

Backlog::~Backlog() {
}

void Backlog::addEntry(const std::string& speaker, const std::string& text,
                       const std::string& voiceFile,
                       const std::string& scriptName, int scriptLine) {
    BacklogEntry entry;
    entry.speaker = speaker;
    entry.text = text;
    entry.voiceFile = voiceFile;
    entry.scriptName = scriptName;
    entry.scriptLine = scriptLine;
    entry.read = true;

    m_entries.push_back(entry);

    // 限制最大数量
    if (static_cast<int>(m_entries.size()) > m_maxEntries) {
        m_entries.erase(m_entries.begin());
    }
}

void Backlog::clear() {
    m_entries.clear();
    m_scrollY = 0.0f;
    m_targetScrollY = 0.0f;
}

void Backlog::show() {
    m_visible = true;
    m_fadeAlpha = 0.0f;
    // 滚动到最新
    m_targetScrollY = m_contentHeight;
    m_scrollY = m_targetScrollY;
    m_playingIndex = -1;
}

void Backlog::hide() {
    m_visible = false;
    m_audio->stopVoice();
    m_playingIndex = -1;
}

void Backlog::updateScroll(float dt) {
    m_scrollY += (m_targetScrollY - m_scrollY) * std::min(dt * 10.0f, 1.0f);
}

float Backlog::getEntryHeight(int index) const {
    if (index < 0 || index >= static_cast<int>(m_entries.size())) return 0.0f;
    // 估算高度：文字行数 * 行高 + 间距
    const auto& entry = m_entries[index];
    int charsPerLine = 40;
    int lines = std::max(1, static_cast<int>(entry.text.length()) / charsPerLine + 1);
    float h = lines * 24.0f + 40.0f;  // 行高24 + 间距
    if (!entry.speaker.empty()) h += 24.0f;
    return h;
}

void Backlog::update(float dt, const Input& input) {
    if (!m_visible) return;

    m_fadeAlpha = std::min(m_fadeAlpha + dt * 8.0f, 1.0f);

    // 计算内容总高度
    m_contentHeight = 0.0f;
    for (int i = 0; i < static_cast<int>(m_entries.size()); ++i) {
        m_contentHeight += getEntryHeight(i);
    }

    int screenH = m_renderer->getHeight();
    float maxScroll = std::max(0.0f, m_contentHeight - screenH + 100.0f);

    // 滚动
    int wheel = input.getMouseWheelY();
    if (wheel != 0) {
        m_targetScrollY -= wheel * 50;
    }
    m_targetScrollY = std::clamp(m_targetScrollY, 0.0f, maxScroll);
    updateScroll(dt);

    // ESC 关闭
    if (input.isKeyPressed(SDL_SCANCODE_ESCAPE)) {
        hide();
        return;
    }

    // 检测 hover 和点击
    m_hoverIndex = -1;
    int mx, my;
    input.getMousePosition(mx, my);

    float y = 50.0f - m_scrollY;
    for (int i = 0; i < static_cast<int>(m_entries.size()); ++i) {
        float h = getEntryHeight(i);
        if (my >= y && my < y + h && mx < m_renderer->getWidth() - 50) {
            m_hoverIndex = i;
            break;
        }
        y += h;
    }

    // 点击条目
    if (input.isMouseButtonPressed(SDL_BUTTON_LEFT) && m_hoverIndex >= 0) {
        int idx = m_hoverIndex;
        const auto& entry = m_entries[idx];

        // 如果有语音文件，播放语音
        if (!entry.voiceFile.empty()) {
            m_audio->playVoice(entry.voiceFile);
            m_playingIndex = idx;
        }

        // 双击跳转回该句
        // 这里简化为：如果有跳转回调就调用
        if (m_jumpCallback) {
            m_jumpCallback(entry.scriptName, entry.scriptLine);
        }
    }
}

void Backlog::render() {
    if (!m_visible) return;

    Uint8 alpha = static_cast<Uint8>(m_fadeAlpha * 255);
    int screenW = m_renderer->getWidth();
    int screenH = m_renderer->getHeight();

    // 半透明背景
    m_renderer->drawRect(0, 0, screenW, screenH, {0, 0, 0, alpha});

    // 标题栏
    m_renderer->drawRect(0, 0, screenW, 40, {20, 20, 30, alpha});
    m_renderer->renderText("履 历", 20, 10, {255, 220, 100, alpha}, m_font);
    m_renderer->renderText("ESC: 关闭  |  点击: 回放语音/跳转", screenW - 350, 12,
                          {150, 150, 160, alpha}, m_smallFont);

    // 条目
    float y = 50.0f - m_scrollY;
    for (int i = 0; i < static_cast<int>(m_entries.size()); ++i) {
        float h = getEntryHeight(i);
        if (y + h >= 40 && y < screenH) {
            renderEntry(m_entries[i], i, y, alpha);
        }
        y += h;
    }

    // 滚动条
    if (m_contentHeight > screenH - 50) {
        int barH = static_cast<int>((screenH - 50.0f) * (screenH - 50.0f) / m_contentHeight);
        int barY = 50 + static_cast<int>((screenH - 50.0f - barH) * m_scrollY / (m_contentHeight - screenH + 50));
        m_renderer->drawRect(screenW - 8, barY, 6, barH, {100, 100, 120, alpha});
    }
}

void Backlog::renderEntry(const BacklogEntry& entry, int index, float y, Uint8 alpha) {
    int screenW = m_renderer->getWidth();
    bool hover = (m_hoverIndex == index);
    bool playing = (m_playingIndex == index);

    // hover 背景
    if (hover) {
        m_renderer->drawRect(10, static_cast<int>(y), screenW - 30,
                             static_cast<int>(getEntryHeight(index)),
                             {40, 40, 60, static_cast<Uint8>(alpha * 0.5f)});
    }

    int textX = 30;
    int currentY = static_cast<int>(y) + 10;

    // 说话者
    if (!entry.speaker.empty()) {
        SDL_Color speakerColor = playing ? SDL_Color{100, 200, 255, alpha}
                                          : SDL_Color{255, 220, 100, alpha};
        m_renderer->renderText(entry.speaker, textX, currentY, speakerColor, m_font);
        currentY += 28;
    }

    // 对话内容（自动换行）
    SDL_Color textColor = {220, 220, 230, alpha};
    int maxW = screenW - 80;
    m_renderer->renderTextWrapped(entry.text, textX, currentY, maxW, textColor, m_smallFont);

    // 脚本信息
    SDL_Color scriptColor = {80, 80, 90, alpha};
    std::string scriptInfo = entry.scriptName + ":" + std::to_string(entry.scriptLine);
    m_renderer->renderText(scriptInfo, screenW - 200, static_cast<int>(y) + 5,
                          scriptColor, m_smallFont);

    // 语音图标
    if (!entry.voiceFile.empty()) {
        SDL_Color voiceColor = playing ? SDL_Color{100, 200, 255, alpha}
                                        : SDL_Color{150, 150, 160, alpha};
        m_renderer->renderText("♪", screenW - 30, static_cast<int>(y) + 5,
                              voiceColor, m_font);
    }
}
