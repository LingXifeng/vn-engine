#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <string>
#include <unordered_map>
#include <memory>
#include "renderer.h"
#include "resource_package.h"

// 资源管理器 - 缓存纹理和字体
// 支持双模式：开发时从文件系统读取，发布时从 .pak 包读取
class ResourceManager {
public:
    ResourceManager(Renderer* renderer);
    ~ResourceManager();

    // 设置资源根目录
    void setBasePath(const std::string& path) { m_basePath = path; }
    std::string getBasePath() const { return m_basePath; }

    // 拼接完整路径
    std::string resolvePath(const std::string& relativePath) const;

    // === 资源包功能 ===
    // 加载 .pak 资源包
    bool loadPackage(const std::string& path);
    // 卸载资源包
    void unloadPackage();
    // 是否已加载资源包
    bool hasPackage() const { return m_package.isOpen(); }
    // 检查资源是否在包中
    bool isInPackage(const std::string& path) const;
    // 列出包中所有资源
    std::vector<std::string> listPackageResources() const;

    // 纹理缓存
    std::shared_ptr<Texture> getTexture(const std::string& path);
    void preloadTexture(const std::string& path);
    void unloadTexture(const std::string& path);

    // 字体缓存
    TTF_Font* getFont(const std::string& path, int size);

    // 读取原始资源数据（优先从包读取）
    std::vector<uint8_t> readResourceData(const std::string& path);

    // 清理所有缓存
    void clearCache();
    void clearUnused();  // 清理引用计数为1的（仅缓存持有）

    // 统计
    size_t getTextureCount() const { return m_textureCache.size(); }
    size_t getFontCount() const { return m_fontCache.size(); }

private:
    Renderer* m_renderer;
    std::string m_basePath;

    PackageReader m_package;

    std::unordered_map<std::string, std::shared_ptr<Texture>> m_textureCache;
    std::unordered_map<std::string, TTF_Font*> m_fontCache;
};
