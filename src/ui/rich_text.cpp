#include "rich_text.h"
#include <algorithm>
#include <sstream>

RichTextRenderer::RichTextRenderer(Renderer* renderer) : m_renderer(renderer) {}
RichTextRenderer::~RichTextRenderer() {}

void RichTextRenderer::setFont(TTF_Font* normal, TTF_Font* bold) {
    m_fontNormal = normal;
    m_fontBold = bold ? bold : normal;
}

SDL_Color RichTextRenderer::parseColor(const std::string& hex) const {
    SDL_Color c = {255, 255, 255, 255};
    if (hex.length() >= 6) {
        c.r = (Uint8)std::stoi(hex.substr(0, 2), nullptr, 16);
        c.g = (Uint8)std::stoi(hex.substr(2, 2), nullptr, 16);
        c.b = (Uint8)std::stoi(hex.substr(4, 2), nullptr, 16);
    }
    return c;
}

std::vector<RichSegment> RichTextRenderer::parse(const std::string& markup) const {
    std::vector<RichSegment> segments;
    TextStyle currentStyle = TextStyle::NORMAL;
    SDL_Color currentColor = {255, 255, 255, 255};
    bool hasShadow = false;
    bool hasOutline = false;
    std::string pendingRuby;

    size_t i = 0;
    std::string buffer;

    auto flushBuffer = [&]() {
        if (!buffer.empty()) {
            RichSegment seg;
            seg.text = buffer;
            seg.style = currentStyle;
            seg.color = currentColor;
            if (!pendingRuby.empty()) {
                seg.ruby = pendingRuby;
                seg.style = seg.style | TextStyle::RUBY;
                pendingRuby.clear();
            }
            seg.shadowColor = {0, 0, 0, 128};
            seg.outlineColor = {0, 0, 0, 255};
            segments.push_back(seg);
            buffer.clear();
        }
    };

    while (i < markup.size()) {
        if (markup[i] == '{') {
            size_t close = markup.find('}', i);
            if (close == std::string::npos) {
                buffer += markup[i];
                i++;
                continue;
            }

            std::string tag = markup.substr(i + 1, close - i - 1);
            flushBuffer();

            if (tag == "b") currentStyle = currentStyle | TextStyle::BOLD;
            else if (tag == "/b") currentStyle = currentStyle & (TextStyle)(~(int)TextStyle::BOLD);
            else if (tag == "i") currentStyle = currentStyle | TextStyle::ITALIC;
            else if (tag == "/i") currentStyle = currentStyle & (TextStyle)(~(int)TextStyle::ITALIC);
            else if (tag == "u") currentStyle = currentStyle | TextStyle::UNDERLINE;
            else if (tag == "/u") currentStyle = currentStyle & (TextStyle)(~(int)TextStyle::UNDERLINE);
            else if (tag == "shadow") { hasShadow = true; currentStyle = currentStyle | TextStyle::SHADOW; }
            else if (tag == "/shadow") { hasShadow = false; currentStyle = currentStyle & (TextStyle)(~(int)TextStyle::SHADOW); }
            else if (tag == "outline") { hasOutline = true; currentStyle = currentStyle | TextStyle::OUTLINE; }
            else if (tag == "/outline") { hasOutline = false; currentStyle = currentStyle & (TextStyle)(~(int)TextStyle::OUTLINE); }
            else if (tag.substr(0, 6) == "color:") currentColor = parseColor(tag.substr(6));
            else if (tag == "/color") currentColor = {255, 255, 255, 255};
            else if (tag.substr(0, 5) == "ruby:") pendingRuby = tag.substr(5);
            else if (tag == "/ruby") { /* ruby applies to pending buffer */ }

            i = close + 1;
        } else {
            buffer += markup[i];
            i++;
        }
    }
    flushBuffer();
    return segments;
}

void RichTextRenderer::segmentSize(const RichSegment& seg, int& w, int& h, int& rubyH,
                                    const RichTextConfig& config) const {
    TTF_Font* font = hasStyle(seg.style, TextStyle::BOLD) ? m_fontBold : m_fontNormal;
    if (!font) { w = 0; h = 0; rubyH = 0; return; }

    int w1 = 0, h1 = 0;
    TTF_SizeText(font, seg.text.c_str(), &w1, &h1);
    w = w1 + config.charSpacing;
    h = h1;
    rubyH = 0;

    if (!seg.ruby.empty()) {
        // 注音使用小字号，但我们没有单独的小字体，用当前字体估算
        rubyH = config.rubySize + config.rubyOffset;
        h += rubyH;
    }
}

std::shared_ptr<Texture> RichTextRenderer::renderStyledText(
    const std::string& text, TTF_Font* font, SDL_Color color, TextStyle style,
    SDL_Color shadowColor, SDL_Color outlineColor) const {

    if (!font) return nullptr;

    // 先渲染基础文字
    auto baseTex = m_renderer->renderText(text, font, color);
    if (!baseTex) return nullptr;

    // 如果有阴影或边缘，需要合成
    bool needShadow = hasStyle(style, TextStyle::SHADOW);
    bool needOutline = hasStyle(style, TextStyle::OUTLINE);

    if (!needShadow && !needOutline) return baseTex;

    // 创建合成纹理
    int w = baseTex->width();
    int h = baseTex->height();
    int pad = needOutline ? 2 : 1;
    int totalW = w + pad * 2;
    int totalH = h + pad * 2;

    // 用 SDL_RenderTarget 合成
    SDL_Renderer* sdlRenderer = m_renderer->getSDLRenderer();
    SDL_Texture* target = SDL_CreateTexture(sdlRenderer,
        SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, totalW, totalH);
    if (!target) return baseTex;

    SDL_SetRenderTarget(sdlRenderer, target);
    SDL_SetRenderDrawColor(sdlRenderer, 0, 0, 0, 0);
    SDL_RenderClear(sdlRenderer);

    // 渲染边缘（8方向偏移）
    if (needOutline) {
        auto outlineTex = m_renderer->renderText(text, font, outlineColor);
        if (outlineTex) {
            SDL_SetTextureBlendMode(outlineTex->get(), SDL_BLENDMODE_BLEND);
            for (int dx = -1; dx <= 1; dx++) {
                for (int dy = -1; dy <= 1; dy++) {
                    if (dx == 0 && dy == 0) continue;
                    SDL_Rect dst = {pad + dx, pad + dy, w, h};
                    SDL_RenderCopy(sdlRenderer, outlineTex->get(), nullptr, &dst);
                }
            }
        }
    }

    // 渲染阴影
    if (needShadow) {
        auto shadowTex = m_renderer->renderText(text, font, shadowColor);
        if (shadowTex) {
            SDL_SetTextureBlendMode(shadowTex->get(), SDL_BLENDMODE_BLEND);
            SDL_Rect dst = {pad + 2, pad + 2, w, h};
            SDL_RenderCopy(sdlRenderer, shadowTex->get(), nullptr, &dst);
        }
    }

    // 渲染主文字
    SDL_SetTextureBlendMode(baseTex->get(), SDL_BLENDMODE_BLEND);
    SDL_Rect dst = {pad, pad, w, h};
    SDL_RenderCopy(sdlRenderer, baseTex->get(), nullptr, &dst);

    SDL_SetRenderTarget(sdlRenderer, nullptr);

    return std::make_shared<Texture>(target, totalW, totalH);
}

std::vector<RichLine> RichTextRenderer::layout(const std::vector<RichSegment>& segments,
                                                 int maxWidth, const RichTextConfig& config) const {
    std::vector<RichLine> lines;
    if (segments.empty()) return lines;

    RichLine currentLine;
    int currentX = 0;
    int maxBaseHeight = 0;
    int maxRubyHeight = 0;

    for (const auto& seg : segments) {
        int segW = 0, segH = 0, rubyH = 0;
        segmentSize(seg, segW, segH, rubyH, config);

        // 检查是否需要换行
        if (currentX + segW > maxWidth && !currentLine.segments.empty()) {
            currentLine.width = currentX;
            currentLine.baseHeight = maxBaseHeight;
            currentLine.rubyHeight = maxRubyHeight;
            currentLine.height = maxBaseHeight + maxRubyHeight;
            lines.push_back(currentLine);
            currentLine.segments.clear();
            currentX = 0;
            maxBaseHeight = 0;
            maxRubyHeight = 0;
        }

        currentLine.segments.push_back(seg);
        currentX += segW;
        maxBaseHeight = std::max(maxBaseHeight, segH - rubyH);
        maxRubyHeight = std::max(maxRubyHeight, rubyH);
    }

    // 最后一行
    if (!currentLine.segments.empty()) {
        currentLine.width = currentX;
        currentLine.baseHeight = maxBaseHeight;
        currentLine.rubyHeight = maxRubyHeight;
        currentLine.height = maxBaseHeight + maxRubyHeight;
        lines.push_back(currentLine);
    }

    return lines;
}

void RichTextRenderer::renderSegment(const RichSegment& seg, int x, int y,
                                      const RichTextConfig& config) const {
    TTF_Font* font = hasStyle(seg.style, TextStyle::BOLD) ? m_fontBold : m_fontNormal;
    if (!font) return;

    // 渲染注音
    if (!seg.ruby.empty()) {
        // 使用小字号渲染注音（这里简化：用当前字体，实际可缩放）
        auto rubyTex = m_renderer->renderText(seg.ruby, font, seg.color);
        if (rubyTex) {
            int rubyX = x + (seg.text.size() > 0 ? 0 : 0);
            m_renderer->drawTexture(rubyTex.get(), rubyX, y, 1.0f, 1.0f, seg.color.a);
            y += config.rubySize + config.rubyOffset;
        }
    }

    // 渲染主文字
    auto tex = renderStyledText(seg.text, font, seg.color, seg.style,
                                 seg.shadowColor, seg.outlineColor);
    if (tex) {
        m_renderer->drawTexture(tex.get(), x, y, 1.0f, 1.0f, seg.color.a);
    }

    // 下划线
    if (hasStyle(seg.style, TextStyle::UNDERLINE)) {
        int w1 = 0, h1 = 0;
        TTF_SizeText(font, seg.text.c_str(), &w1, &h1);
        SDL_Color ulColor = seg.color;
        m_renderer->drawLine(x, y + h1 + 1, x + w1, y + h1 + 1, ulColor);
    }
}

void RichTextRenderer::render(const std::vector<RichLine>& lines, int x, int y,
                               const RichTextConfig& config) const {
    int cy = y;
    for (const auto& line : lines) {
        int cx = x;
        for (const auto& seg : line.segments) {
            renderSegment(seg, cx, cy, config);
            int segW = 0, segH = 0, rubyH = 0;
            segmentSize(seg, segW, segH, rubyH, config);
            cx += segW;
        }
        cy += line.height + config.lineSpacing;
    }
}

void RichTextRenderer::renderText(const std::string& markup, int x, int y, int maxWidth,
                                   const RichTextConfig& config) const {
    auto segments = parse(markup);
    auto lines = layout(segments, maxWidth, config);
    render(lines, x, y, config);
}

void RichTextRenderer::measure(const std::vector<RichLine>& lines, int& totalW, int& totalH,
                                const RichTextConfig& config) const {
    totalW = 0;
    totalH = 0;
    for (const auto& line : lines) {
        totalW = std::max(totalW, line.width);
        totalH += line.height + config.lineSpacing;
    }
    if (!lines.empty()) totalH -= config.lineSpacing;
}
