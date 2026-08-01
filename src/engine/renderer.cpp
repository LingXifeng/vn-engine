#include "renderer.h"
#include <iostream>
#include <algorithm>

// === Texture ===

void Texture::setBlendMode(BlendMode mode) {
    if (!m_texture) return;
    switch (mode) {
        case BlendMode::ALPHA: SDL_SetTextureBlendMode(m_texture, SDL_BLENDMODE_BLEND); break;
        case BlendMode::ADD:   SDL_SetTextureBlendMode(m_texture, SDL_BLENDMODE_ADD);   break;
        case BlendMode::MOD:   SDL_SetTextureBlendMode(m_texture, SDL_BLENDMODE_MOD);   break;
        default:               SDL_SetTextureBlendMode(m_texture, SDL_BLENDMODE_BLEND); break;
    }
}

void Texture::setAlpha(Uint8 alpha) {
    if (m_texture) SDL_SetTextureAlphaMod(m_texture, alpha);
}

// === Renderer ===

Renderer::Renderer(SDL_Window* window, int width, int height)
    : m_window(window), m_width(width), m_height(height) {}

Renderer::~Renderer() {
    if (m_renderer) SDL_DestroyRenderer(m_renderer);
    for (auto& [name, font] : m_fonts) {
        if (font) TTF_CloseFont(font);
    }
    for (auto& [font, data] : m_fontMemory) {
        if (font) TTF_CloseFont(font);
    }
}

bool Renderer::init() {
    // 先尝试硬件加速渲染，失败则回退到软件渲染
    m_renderer = SDL_CreateRenderer(m_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);
    if (!m_renderer) {
        std::cerr << "Warning: Accelerated renderer unavailable, trying software: " << SDL_GetError() << std::endl;
        m_renderer = SDL_CreateRenderer(m_window, -1, SDL_RENDERER_SOFTWARE | SDL_RENDERER_TARGETTEXTURE);
    }
    if (!m_renderer) {
        std::cerr << "Renderer creation failed: " << SDL_GetError() << std::endl;
        return false;
    }
    SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_BLEND);
    SDL_RenderSetLogicalSize(m_renderer, m_width, m_height);
    return true;
}

void Renderer::clear(Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
    SDL_SetRenderDrawColor(m_renderer, r, g, b, a);
    SDL_RenderClear(m_renderer);
}

void Renderer::present() {
    SDL_RenderPresent(m_renderer);
}

std::shared_ptr<Texture> Renderer::loadTexture(const std::string& path) {
    SDL_Surface* surface = IMG_Load(path.c_str());
    if (!surface) {
        std::cerr << "Failed to load image: " << path << " - " << IMG_GetError() << std::endl;
        return nullptr;
    }
    SDL_Texture* tex = SDL_CreateTextureFromSurface(m_renderer, surface);
    int w = surface->w, h = surface->h;
    SDL_FreeSurface(surface);
    if (!tex) {
        std::cerr << "Failed to create texture: " << path << " - " << SDL_GetError() << std::endl;
        return nullptr;
    }
    return std::make_shared<Texture>(tex, w, h);
}

std::shared_ptr<Texture> Renderer::loadTextureFromMemory(const uint8_t* data, size_t size) {
    if (!data || size == 0) return nullptr;

    SDL_RWops* rw = SDL_RWFromConstMem(data, static_cast<int>(size));
    if (!rw) return nullptr;

    SDL_Surface* surface = IMG_Load_RW(rw, 1);  // 1 = auto-free rw
    if (!surface) {
        std::cerr << "Failed to load image from memory: " << IMG_GetError() << std::endl;
        return nullptr;
    }
    SDL_Texture* tex = SDL_CreateTextureFromSurface(m_renderer, surface);
    int w = surface->w, h = surface->h;
    SDL_FreeSurface(surface);
    if (!tex) {
        std::cerr << "Failed to create texture from memory: " << SDL_GetError() << std::endl;
        return nullptr;
    }
    return std::make_shared<Texture>(tex, w, h);
}

void Renderer::applyBlendMode(BlendMode mode) {
    switch (mode) {
        case BlendMode::ALPHA: SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_BLEND); break;
        case BlendMode::ADD:   SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_ADD);   break;
        case BlendMode::MOD:   SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_MOD);   break;
        default:               SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_BLEND); break;
    }
}

void Renderer::drawTexture(Texture* tex, float x, float y,
                           float scaleX, float scaleY,
                           Uint8 alpha, float rotation, BlendMode blend) {
    if (!tex || !tex->get()) return;
    int w = tex->width(), h = tex->height();
    SDL_Rect dest = {
        (int)x, (int)y,
        (int)(w * scaleX), (int)(h * scaleY)
    };
    tex->setBlendMode(blend);
    tex->setAlpha(alpha);
    if (rotation != 0.0f) {
        SDL_RenderCopyEx(m_renderer, tex->get(), nullptr, &dest,
                         rotation, nullptr, SDL_FLIP_NONE);
    } else {
        SDL_RenderCopy(m_renderer, tex->get(), nullptr, &dest);
    }
}

void Renderer::drawTextureRect(Texture* tex, const SDL_Rect& srcRect,
                               float x, float y, float scaleX, float scaleY,
                               Uint8 alpha, BlendMode blend) {
    if (!tex || !tex->get()) return;
    SDL_Rect dest = {
        (int)x, (int)y,
        (int)(srcRect.w * scaleX), (int)(srcRect.h * scaleY)
    };
    tex->setBlendMode(blend);
    tex->setAlpha(alpha);
    SDL_RenderCopy(m_renderer, tex->get(), &srcRect, &dest);
}

// === 图层管理 ===

int Renderer::addLayer(const std::shared_ptr<Texture>& tex, float x, float y, int zOrder) {
    Layer layer;
    layer.texture = tex;
    layer.x = x;
    layer.y = y;
    layer.zOrder = zOrder;
    int id = m_nextLayerId++;
    m_layers.push_back({id, layer});
    return id;
}

void Renderer::removeLayer(int id) {
    m_layers.erase(
        std::remove_if(m_layers.begin(), m_layers.end(),
            [id](const auto& pair) { return pair.first == id; }),
        m_layers.end()
    );
}

void Renderer::clearLayers() {
    m_layers.clear();
}

Layer* Renderer::getLayer(int id) {
    for (auto& [lid, layer] : m_layers) {
        if (lid == id) return &layer;
    }
    return nullptr;
}

void Renderer::renderLayers() {
    // 按 zOrder 排序
    std::vector<std::pair<int, Layer>> sorted = m_layers;
    std::sort(sorted.begin(), sorted.end(),
        [](const auto& a, const auto& b) {
            return a.second.zOrder < b.second.zOrder;
        });
    for (auto& [id, layer] : sorted) {
        if (!layer.visible || !layer.texture) continue;
        drawTexture(layer.texture.get(), layer.x, layer.y,
                    layer.scaleX, layer.scaleY,
                    layer.alpha, layer.rotation, layer.blendMode);
    }
}

// === 文字渲染 ===

TTF_Font* Renderer::loadFont(const std::string& path, int size) {
    std::string key = path + ":" + std::to_string(size);
    auto it = m_fonts.find(key);
    if (it != m_fonts.end()) return it->second;

    TTF_Font* font = TTF_OpenFont(path.c_str(), size);
    if (!font) {
        std::cerr << "Failed to load font: " << path << " - " << TTF_GetError() << std::endl;
        return nullptr;
    }
    m_fonts[key] = font;
    return font;
}

TTF_Font* Renderer::loadFontFromMemory(const uint8_t* data, size_t size, int ptSize) {
    if (!data || size == 0) return nullptr;

    // 字体需要内存保持有效，所以拷贝一份保存
    auto fontData = std::make_shared<std::vector<uint8_t>>(data, data + size);

    SDL_RWops* rw = SDL_RWFromConstMem(fontData->data(), static_cast<int>(size));
    if (!rw) return nullptr;

    TTF_Font* font = TTF_OpenFontRW(rw, 1, ptSize);  // 1 = auto-free rw
    if (!font) {
        std::cerr << "Failed to load font from memory: " << TTF_GetError() << std::endl;
        return nullptr;
    }

    // 保存字体数据，防止被释放
    m_fontMemory[font] = fontData;
    return font;
}

std::shared_ptr<Texture> Renderer::renderText(const std::string& text, TTF_Font* font, SDL_Color color) {
    if (!font || text.empty()) return nullptr;
    SDL_Surface* surface = TTF_RenderUTF8_Blended(font, text.c_str(), color);
    if (!surface) return nullptr;
    SDL_Texture* tex = SDL_CreateTextureFromSurface(m_renderer, surface);
    int w = surface->w, h = surface->h;
    SDL_FreeSurface(surface);
    return std::make_shared<Texture>(tex, w, h);
}

std::shared_ptr<Texture> Renderer::renderTextWrapped(const std::string& text, TTF_Font* font,
                                                     int maxWidth, int lineSpacing, SDL_Color color) {
    if (!font || text.empty()) return nullptr;

    // 逐字符换行（支持 UTF-8 中文）
    std::vector<std::string> lines;
    std::string currentLine;
    std::string currentWord;
    int currentW = 0;

    for (size_t i = 0; i < text.size(); ) {
        // 获取下一个 UTF-8 字符
        char c = text[i];
        size_t charLen = 1;
        if ((c & 0x80) == 0) charLen = 1;
        else if ((c & 0xE0) == 0xC0) charLen = 2;
        else if ((c & 0xF0) == 0xE0) charLen = 3;
        else if ((c & 0xF8) == 0xF0) charLen = 4;

        std::string ch = text.substr(i, charLen);
        i += charLen;

        if (ch == "\n") {
            if (!currentWord.empty()) {
                currentLine += currentWord;
                currentWord.clear();
            }
            lines.push_back(currentLine);
            currentLine.clear();
            currentW = 0;
            continue;
        }

        int charW, charH;
        TTF_SizeUTF8(font, ch.c_str(), &charW, &charH);

        if (currentW + charW > maxWidth) {
            if (!currentLine.empty()) lines.push_back(currentLine);
            currentLine = ch;
            currentW = charW;
        } else {
            currentLine += ch;
            currentW += charW;
        }
    }
    if (!currentLine.empty()) lines.push_back(currentLine);

    // 渲染多行文字到一张纹理
    int fontHeight = TTF_FontHeight(font);
    int totalHeight = lines.size() * fontHeight + (lines.size() - 1) * lineSpacing;
    int actualMaxWidth = 0;
    for (auto& line : lines) {
        int w, h;
        TTF_SizeUTF8(font, line.c_str(), &w, &h);
        if (w > actualMaxWidth) actualMaxWidth = w;
    }

    SDL_Surface* result = SDL_CreateRGBSurface(0, actualMaxWidth, totalHeight, 32,
        0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
    SDL_FillRect(result, nullptr, SDL_MapRGBA(result->format, 0, 0, 0, 0));

    int y = 0;
    for (auto& line : lines) {
        if (!line.empty()) {
            SDL_Surface* lineSurf = TTF_RenderUTF8_Blended(font, line.c_str(), color);
            if (lineSurf) {
                SDL_Rect dest = {0, y, lineSurf->w, lineSurf->h};
                SDL_BlitSurface(lineSurf, nullptr, result, &dest);
                SDL_FreeSurface(lineSurf);
            }
        }
        y += fontHeight + lineSpacing;
    }

    SDL_Texture* tex = SDL_CreateTextureFromSurface(m_renderer, result);
    SDL_FreeSurface(result);
    return std::make_shared<Texture>(tex, actualMaxWidth, totalHeight);
}

// === 便捷文字渲染 ===

void Renderer::renderText(const std::string& text, int x, int y,
                          SDL_Color color, TTF_Font* font) {
    if (!font || text.empty()) return;
    auto tex = renderText(text, font, color);
    if (tex) drawTexture(tex.get(), static_cast<float>(x), static_cast<float>(y));
}

void Renderer::renderTextWrapped(const std::string& text, int x, int y, int maxWidth,
                                 SDL_Color color, TTF_Font* font) {
    if (!font || text.empty()) return;
    auto tex = renderTextWrapped(text, font, maxWidth, 4, color);
    if (tex) drawTexture(tex.get(), static_cast<float>(x), static_cast<float>(y));
}

// === 图形绘制 ===

void Renderer::drawRect(float x, float y, int w, int h, SDL_Color color, bool filled) {
    SDL_SetRenderDrawColor(m_renderer, color.r, color.g, color.b, color.a);
    SDL_Rect rect = {(int)x, (int)y, w, h};
    if (filled) SDL_RenderFillRect(m_renderer, &rect);
    else SDL_RenderDrawRect(m_renderer, &rect);
}

void Renderer::drawLine(float x1, float y1, float x2, float y2, SDL_Color color) {
    SDL_SetRenderDrawColor(m_renderer, color.r, color.g, color.b, color.a);
    SDL_RenderDrawLine(m_renderer, (int)x1, (int)y1, (int)x2, (int)y2);
}
