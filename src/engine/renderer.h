#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <string>
#include <unordered_map>
#include <vector>
#include <memory>

// 混合模式
enum class BlendMode {
    NONE,       // 正常
    ALPHA,      // Alpha 混合
    ADD,        // 加算
    MOD,        // 乘算
    SCREEN      // 滤色
};

// 纹理包装类
class Texture {
public:
    Texture(SDL_Texture* tex = nullptr, int w = 0, int h = 0)
        : m_texture(tex), m_width(w), m_height(h) {}
    ~Texture() { if (m_texture) SDL_DestroyTexture(m_texture); }

    SDL_Texture* get() const { return m_texture; }
    int width() const { return m_width; }
    int height() const { return m_height; }

    void setBlendMode(BlendMode mode);
    void setAlpha(Uint8 alpha);

private:
    SDL_Texture* m_texture;
    int m_width, m_height;
};

// 图层结构
struct Layer {
    std::shared_ptr<Texture> texture;
    float x = 0.0f, y = 0.0f;        // 位置
    float scaleX = 1.0f, scaleY = 1.0f; // 缩放
    float rotation = 0.0f;            // 旋转（度）
    Uint8 alpha = 255;                // 透明度
    BlendMode blendMode = BlendMode::ALPHA;
    bool visible = true;
    int zOrder = 0;                   // 层级（越大越上）
};

// 渲染器类
class Renderer {
public:
    Renderer(SDL_Window* window, int width, int height);
    ~Renderer();

    bool init();

    // 基本渲染
    void clear(Uint8 r = 0, Uint8 g = 0, Uint8 b = 0, Uint8 a = 255);
    void present();

    // 纹理加载
    std::shared_ptr<Texture> loadTexture(const std::string& path);
    // 从内存加载纹理 (用于资源包)
    std::shared_ptr<Texture> loadTextureFromMemory(const uint8_t* data, size_t size);

    // 纹理渲染
    void drawTexture(Texture* tex, float x, float y,
                     float scaleX = 1.0f, float scaleY = 1.0f,
                     Uint8 alpha = 255, float rotation = 0.0f,
                     BlendMode blend = BlendMode::ALPHA);

    void drawTextureRect(Texture* tex, const SDL_Rect& srcRect,
                         float x, float y, float scaleX = 1.0f, float scaleY = 1.0f,
                         Uint8 alpha = 255, BlendMode blend = BlendMode::ALPHA);

    // 图层管理
    int addLayer(const std::shared_ptr<Texture>& tex, float x, float y, int zOrder = 0);
    void removeLayer(int id);
    void clearLayers();
    Layer* getLayer(int id);
    void renderLayers();

    // 文字渲染
    TTF_Font* loadFont(const std::string& path, int size);
    // 从内存加载字体 (用于资源包)
    TTF_Font* loadFontFromMemory(const uint8_t* data, size_t size, int ptSize);
    std::shared_ptr<Texture> renderText(const std::string& text, TTF_Font* font,
                                        SDL_Color color = {255, 255, 255, 255});
    std::shared_ptr<Texture> renderTextWrapped(const std::string& text, TTF_Font* font,
                                               int maxWidth, int lineSpacing = 4,
                                               SDL_Color color = {255, 255, 255, 255});

    // 便捷文字渲染（直接绘制到屏幕）
    void renderText(const std::string& text, int x, int y,
                    SDL_Color color, TTF_Font* font);
    void renderTextWrapped(const std::string& text, int x, int y, int maxWidth,
                           SDL_Color color, TTF_Font* font);

    // 图形绘制
    void drawRect(float x, float y, int w, int h, SDL_Color color, bool filled = true);
    void drawLine(float x1, float y1, float x2, float y2, SDL_Color color);

    // 获取尺寸
    int getWidth() const { return m_width; }
    int getHeight() const { return m_height; }
    SDL_Renderer* getSDLRenderer() { return m_renderer; }
    SDL_Window* getWindow() { return m_window; }

private:
    SDL_Window* m_window;
    SDL_Renderer* m_renderer = nullptr;
    int m_width, m_height;

    std::vector<std::pair<int, Layer>> m_layers;
    int m_nextLayerId = 0;

    std::unordered_map<std::string, TTF_Font*> m_fonts;
    std::unordered_map<TTF_Font*, std::shared_ptr<std::vector<uint8_t>>> m_fontMemory;  // 内存字体的数据保持

    void applyBlendMode(BlendMode mode);
};
