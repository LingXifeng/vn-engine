#pragma once

#include <SDL2/SDL.h>
#include <string>
#include <vector>

// 窗口缩放与全屏管理
class WindowResize {
public:
    WindowResize(SDL_Window* window);
    ~WindowResize();

    // 全屏切换
    void toggleFullscreen();
    void setFullscreen(bool fullscreen);
    bool isFullscreen() const { return m_fullscreen; }

    // 窗口模式切换
    // fullscreenMode: 0=窗口, 1=全屏, 2=无边框全屏
    void setFullscreenMode(int mode);
    int getFullscreenMode() const { return m_fullscreenMode; }

    // 分辨率设置
    void setResolution(int width, int height);
    void getResolution(int& width, int& height) const;

    // 获取可用分辨率列表
    struct Resolution {
        int width;
        int height;
        std::string label;
    };
    std::vector<Resolution> getAvailableResolutions() const;

    // 缩放因子（渲染缩放，不改变窗口大小）
    void setRenderScale(float scale);
    float getRenderScale() const { return m_renderScale; }

    // 逻辑分辨率（游戏内容渲染的实际分辨率）
    void setLogicalSize(int width, int height);
    void getLogicalSize(int& width, int& height) const;

    // 窗口缩放（改变窗口大小）
    void resizeWindow(int width, int height);

    // 最大化/最小化
    void maximize();
    void minimize();
    void restore();

    // 窗口位置
    void setPosition(int x, int y);
    void getPosition(int& x, int& y) const;
    void centerWindow();

    // 获取当前窗口信息
    void getWindowSize(int& width, int& height) const;

    // VSync 控制
    void setVSync(bool enabled);
    bool getVSync() const { return m_vsync; }

    // 更新（处理窗口事件）
    void update();

private:
    SDL_Window* m_window;
    bool m_fullscreen = false;
    int m_fullscreenMode = 0;  // 0=窗口, 1=全屏, 2=无边框

    int m_windowedWidth = 1280;
    int m_windowedHeight = 720;
    int m_logicalWidth = 1280;
    int m_logicalHeight = 720;

    float m_renderScale = 1.0f;
    bool m_vsync = true;

    // 保存窗口模式下的位置和大小
    int m_savedX = 0, m_savedY = 0;
    int m_savedW = 1280, m_savedH = 720;
};
