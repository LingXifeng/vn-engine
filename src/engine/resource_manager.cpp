#include "resource_manager.h"
#include <iostream>
#include <fstream>
#include <filesystem>

namespace fs = std::filesystem;

ResourceManager::ResourceManager(Renderer* renderer)
    : m_renderer(renderer) {}

ResourceManager::~ResourceManager() {
    clearCache();
    unloadPackage();
}

std::string ResourceManager::resolvePath(const std::string& relativePath) const {
    if (relativePath.empty()) return m_basePath;
    if (relativePath[0] == '/' || relativePath[1] == ':') return relativePath; // 绝对路径
    if (m_basePath.empty()) return relativePath;
    if (m_basePath.back() == '/') return m_basePath + relativePath;
    return m_basePath + "/" + relativePath;
}

// === 资源包功能 ===

bool ResourceManager::loadPackage(const std::string& path) {
    if (!m_package.open(path)) {
        std::cerr << "ResourceManager: Failed to load package " << path << std::endl;
        return false;
    }
    std::cout << "ResourceManager: Package loaded (" << m_package.getFileCount() << " files)" << std::endl;
    return true;
}

void ResourceManager::unloadPackage() {
    m_package.close();
}

bool ResourceManager::isInPackage(const std::string& path) const {
    if (!m_package.isOpen()) return false;
    return m_package.has(path);
}

std::vector<std::string> ResourceManager::listPackageResources() const {
    if (!m_package.isOpen()) return {};
    return m_package.list();
}

std::vector<uint8_t> ResourceManager::readResourceData(const std::string& path) {
    // 优先从包读取
    if (m_package.isOpen() && m_package.has(path)) {
        return m_package.read(path);
    }

    // 回退到文件系统
    std::string fullPath = resolvePath(path);
    std::ifstream f(fullPath, std::ios::binary);
    if (!f.is_open()) {
        return {};
    }
    f.seekg(0, std::ios::end);
    size_t size = static_cast<size_t>(f.tellg());
    f.seekg(0, std::ios::beg);
    std::vector<uint8_t> data(size);
    f.read(reinterpret_cast<char*>(data.data()), size);
    return data;
}

std::shared_ptr<Texture> ResourceManager::getTexture(const std::string& path) {
    auto it = m_textureCache.find(path);
    if (it != m_textureCache.end()) {
        return it->second;
    }

    // 优先从包加载
    if (m_package.isOpen() && m_package.has(path)) {
        auto data = m_package.read(path);
        if (!data.empty()) {
            auto tex = m_renderer->loadTextureFromMemory(data.data(), data.size());
            if (tex) {
                m_textureCache[path] = tex;
                return tex;
            }
        }
    }

    // 回退到文件系统
    std::string fullPath = resolvePath(path);
    auto tex = m_renderer->loadTexture(fullPath);
    if (tex) {
        m_textureCache[path] = tex;
    }
    return tex;
}

void ResourceManager::preloadTexture(const std::string& path) {
    getTexture(path);
}

void ResourceManager::unloadTexture(const std::string& path) {
    m_textureCache.erase(path);
}

TTF_Font* ResourceManager::getFont(const std::string& path, int size) {
    std::string key = path + ":" + std::to_string(size);
    auto it = m_fontCache.find(key);
    if (it != m_fontCache.end()) return it->second;

    // 优先从包加载
    if (m_package.isOpen() && m_package.has(path)) {
        auto data = m_package.read(path);
        if (!data.empty()) {
            TTF_Font* font = m_renderer->loadFontFromMemory(data.data(), data.size(), size);
            if (font) {
                m_fontCache[key] = font;
                return font;
            }
        }
    }

    // 回退到文件系统
    std::string fullPath = resolvePath(path);
    TTF_Font* font = m_renderer->loadFont(fullPath, size);
    if (font) {
        m_fontCache[key] = font;
    }
    return font;
}

void ResourceManager::clearCache() {
    m_textureCache.clear();
    for (auto& [key, font] : m_fontCache) {
        if (font) TTF_CloseFont(font);
    }
    m_fontCache.clear();
}

void ResourceManager::clearUnused() {
    for (auto it = m_textureCache.begin(); it != m_textureCache.end(); ) {
        if (it->second.use_count() == 1) {
            it = m_textureCache.erase(it);
        } else {
            ++it;
        }
    }
}

// 添加缺失的 include
#include <algorithm>
