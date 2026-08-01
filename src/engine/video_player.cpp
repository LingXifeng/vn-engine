#include "video_player.h"
#include <SDL2/SDL_image.h>
#include <iostream>
#include <filesystem>
#include <algorithm>

namespace fs = std::filesystem;

VideoPlayer::VideoPlayer(Renderer* renderer)
    : m_renderer(renderer) {}

VideoPlayer::~VideoPlayer() {
    stop();
    m_frames.clear();
}

bool VideoPlayer::load(const std::string& path, int fps) {
    stop();
    m_frames.clear();
    m_currentFrame = 0;
    m_totalFrames = 0;
    m_done = false;

    m_frameInterval = 1.0f / static_cast<float>(fps > 0 ? fps : 30);

    // 检查路径是目录还是文件
    if (fs::is_directory(path)) {
        return loadFrameSequence(path);
    } else {
        return loadSingleImage(path);
    }
}

bool VideoPlayer::loadFrameSequence(const std::string& dir) {
    // 收集目录中的帧文件
    std::vector<std::string> framePaths;

    for (const auto& entry : fs::directory_iterator(dir)) {
        if (!entry.is_regular_file()) continue;
        std::string ext = entry.path().extension().string();
        // 转小写
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
        if (ext == ".png" || ext == ".jpg" || ext == ".bmp") {
            framePaths.push_back(entry.path().string());
        }
    }

    if (framePaths.empty()) {
        std::cerr << "VideoPlayer: No frame files found in " << dir << std::endl;
        return false;
    }

    // 按文件名排序
    std::sort(framePaths.begin(), framePaths.end());

    // 加载所有帧
    for (const auto& framePath : framePaths) {
        auto tex = m_renderer->loadTexture(framePath);
        if (tex) {
            m_frames.push_back(tex);
        } else {
            std::cerr << "VideoPlayer: Failed to load frame: " << framePath << std::endl;
        }
    }

    m_totalFrames = static_cast<int>(m_frames.size());
    std::cout << "VideoPlayer: Loaded " << m_totalFrames << " frames from " << dir << std::endl;
    return m_totalFrames > 0;
}

bool VideoPlayer::loadSingleImage(const std::string& path) {
    auto tex = m_renderer->loadTexture(path);
    if (!tex) {
        std::cerr << "VideoPlayer: Failed to load image: " << path << std::endl;
        return false;
    }
    m_frames.push_back(tex);
    m_totalFrames = 1;
    std::cout << "VideoPlayer: Loaded single image: " << path << std::endl;
    return true;
}

void VideoPlayer::play() {
    if (m_totalFrames == 0) return;
    if (m_done) {
        m_currentFrame = 0;
        m_done = false;
    }
    m_playing = true;
    m_paused = false;
    m_frameTimer = 0.0f;
}

void VideoPlayer::pause() {
    if (m_playing) {
        m_paused = true;
        m_playing = false;
    }
}

void VideoPlayer::stop() {
    m_playing = false;
    m_paused = false;
    m_done = false;
    m_currentFrame = 0;
    m_frameTimer = 0.0f;
}

void VideoPlayer::setRegion(int x, int y, int w, int h) {
    m_x = x;
    m_y = y;
    m_w = w;
    m_h = h;
}

void VideoPlayer::seek(int frame) {
    if (m_totalFrames == 0) return;
    m_currentFrame = std::clamp(frame, 0, m_totalFrames - 1);
    m_done = false;
}

void VideoPlayer::update(float dt) {
    if (!m_playing || m_paused || m_totalFrames == 0) return;

    m_frameTimer += dt * m_speed;

    while (m_frameTimer >= m_frameInterval) {
        m_frameTimer -= m_frameInterval;
        m_currentFrame++;

        if (m_currentFrame >= m_totalFrames) {
            if (m_loop) {
                m_currentFrame = 0;
            } else {
                m_currentFrame = m_totalFrames - 1;
                m_playing = false;
                m_done = true;
                break;
            }
        }
    }
}

void VideoPlayer::render(Renderer* renderer) {
    if (m_totalFrames == 0 || m_currentFrame >= m_totalFrames) return;

    auto& tex = m_frames[m_currentFrame];
    if (!tex) return;

    // 计算渲染区域
    int renderX = m_x;
    int renderY = m_y;
    int renderW = m_w;
    int renderH = m_h;

    if (m_fullscreen) {
        renderX = 0;
        renderY = 0;
        renderW = renderer->getWidth();
        renderH = renderer->getHeight();
    }

    // 计算缩放比，保持宽高比
    float texW = static_cast<float>(tex->width());
    float texH = static_cast<float>(tex->height());
    float scaleX = static_cast<float>(renderW) / texW;
    float scaleY = static_cast<float>(renderH) / texH;
    float scale = std::min(scaleX, scaleY);  // 等比缩放

    float drawW = texW * scale;
    float drawH = texH * scale;
    float drawX = renderX + (renderW - drawW) / 2.0f;
    float drawY = renderY + (renderH - drawH) / 2.0f;

    renderer->drawTexture(tex.get(), drawX, drawY, scale, scale, 255);
}
