#pragma once

#include <SDL2/SDL.h>
#include <string>
#include <vector>
#include <memory>
#include "renderer.h"

// 视频播放器 - FMV (Full Motion Video) 支持
// 使用逐帧图像序列实现视频播放，兼容无视频解码库的环境
class VideoPlayer {
public:
    VideoPlayer(Renderer* renderer);
    ~VideoPlayer();

    // 加载视频（帧序列目录或单个视频文件）
    // 帧序列模式: path 为目录名，帧文件命名格式为 frame_0001.png, frame_0002.png, ...
    // 单帧模式: path 为单张图片路径（用于测试/占位）
    bool load(const std::string& path, int fps = 30);

    // 播放控制
    void play();
    void pause();
    void stop();
    bool isPlaying() const { return m_playing; }
    bool isPaused() const { return m_paused; }
    bool isDone() const { return m_done; }

    // 循环设置
    void setLoop(bool loop) { m_loop = loop; }
    bool getLoop() const { return m_loop; }

    // 音频关联（可选：播放视频时同时播放音频）
    void setAudioPath(const std::string& path) { m_audioPath = path; }
    const std::string& getAudioPath() const { return m_audioPath; }

    // 渲染区域
    void setRegion(int x, int y, int w, int h);
    void setFullscreen(bool fs) { m_fullscreen = fs; }
    bool isFullscreen() const { return m_fullscreen; }

    // 跳转
    void seek(int frame);
    int getCurrentFrame() const { return m_currentFrame; }
    int getTotalFrames() const { return m_totalFrames; }

    // 速度控制 (0.5x ~ 2.0x)
    void setSpeed(float speed) { m_speed = speed; }
    float getSpeed() const { return m_speed; }

    // 更新与渲染
    void update(float dt);
    void render(Renderer* renderer);

    // 跳过按键检测
    void setSkippable(bool skip) { m_skippable = skip; }
    bool isSkippable() const { return m_skippable; }

private:
    Renderer* m_renderer;
    std::vector<std::shared_ptr<Texture>> m_frames;
    int m_totalFrames = 0;
    int m_currentFrame = 0;

    bool m_playing = false;
    bool m_paused = false;
    bool m_done = false;
    bool m_loop = false;
    bool m_skippable = true;
    bool m_fullscreen = false;

    float m_speed = 1.0f;
    float m_frameTimer = 0.0f;
    float m_frameInterval = 1.0f / 30.0f;  // 默认 30fps

    // 渲染区域
    int m_x = 0, m_y = 0, m_w = 1280, m_h = 720;

    std::string m_audioPath;

    // 加载帧序列
    bool loadFrameSequence(const std::string& dir);
    bool loadSingleImage(const std::string& path);
};
