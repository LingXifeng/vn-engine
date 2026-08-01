#include "window_resize.h"
#include <iostream>

WindowResize::WindowResize(SDL_Window* window)
    : m_window(window) {
    if (m_window) {
        SDL_GetWindowSize(m_window, &m_windowedWidth, &m_windowedHeight);
        SDL_GetWindowPosition(m_window, &m_savedX, &m_savedY);
        m_savedW = m_windowedWidth;
        m_savedH = m_windowedHeight;
        m_logicalWidth = m_windowedWidth;
        m_logicalHeight = m_windowedHeight;
    }
}

WindowResize::~WindowResize() {}

void WindowResize::toggleFullscreen() {
    setFullscreen(!m_fullscreen);
}

void WindowResize::setFullscreen(bool fullscreen) {
    if (!m_window) return;
    if (fullscreen == m_fullscreen) return;

    if (fullscreen) {
        // 保存窗口模式信息
        SDL_GetWindowSize(m_window, &m_savedW, &m_savedH);
        SDL_GetWindowPosition(m_window, &m_savedX, &m_savedY);

        if (m_fullscreenMode == 2) {
            // 无边框全屏
            SDL_SetWindowFullscreen(m_window, SDL_WINDOW_FULLSCREEN_DESKTOP);
        } else {
            // 真全屏
            SDL_SetWindowFullscreen(m_window, SDL_WINDOW_FULLSCREEN);
        }
        m_fullscreen = true;
    } else {
        // 恢复窗口模式
        SDL_SetWindowFullscreen(m_window, 0);
        SDL_SetWindowSize(m_window, m_savedW, m_savedH);
        SDL_SetWindowPosition(m_window, m_savedX, m_savedY);
        m_fullscreen = false;
    }

    std::cout << "WindowResize: Fullscreen " << (m_fullscreen ? "ON" : "OFF") << std::endl;
}

void WindowResize::setFullscreenMode(int mode) {
    bool wasFullscreen = m_fullscreen;
    if (wasFullscreen) {
        setFullscreen(false);  // 先退出全屏
    }
    m_fullscreenMode = mode;
    if (wasFullscreen) {
        setFullscreen(true);  // 重新进入全屏
    }
}

void WindowResize::setResolution(int width, int height) {
    if (!m_window) return;
    m_windowedWidth = width;
    m_windowedHeight = height;

    if (!m_fullscreen) {
        SDL_SetWindowSize(m_window, width, height);
        centerWindow();
    }
    std::cout << "WindowResize: Resolution set to " << width << "x" << height << std::endl;
}

void WindowResize::getResolution(int& width, int& height) const {
    if (m_fullscreen) {
        width = m_savedW;
        height = m_savedH;
    } else {
        width = m_windowedWidth;
        height = m_windowedHeight;
    }
}

std::vector<WindowResize::Resolution> WindowResize::getAvailableResolutions() const {
    std::vector<Resolution> resolutions;

    // 常见分辨率列表
    resolutions.push_back({640, 480, "640x480 (4:3)"});
    resolutions.push_back({800, 600, "800x600 (4:3)"});
    resolutions.push_back({1024, 768, "1024x768 (4:3)"});
    resolutions.push_back({1280, 720, "1280x720 (16:9)"});
    resolutions.push_back({1366, 768, "1366x768 (16:9)"});
    resolutions.push_back({1600, 900, "1600x900 (16:9)"});
    resolutions.push_back({1920, 1080, "1920x1080 (16:9)"});
    resolutions.push_back({2560, 1440, "2560x1440 (16:9)"});

    // 尝试获取显示器分辨率
    int displayCount = SDL_GetNumVideoDisplays();
    for (int i = 0; i < displayCount; i++) {
        SDL_DisplayMode mode;
        if (SDL_GetDesktopDisplayMode(i, &mode) == 0) {
            Resolution res = {mode.w, mode.h,
                std::to_string(mode.w) + "x" + std::to_string(mode.h) + " (Display " + std::to_string(i) + ")"};
            // 避免重复
            bool found = false;
            for (const auto& r : resolutions) {
                if (r.width == res.width && r.height == res.height) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                resolutions.push_back(res);
            }
        }
    }

    return resolutions;
}

void WindowResize::setRenderScale(float scale) {
    m_renderScale = scale;
    if (m_renderScale < 0.25f) m_renderScale = 0.25f;
    if (m_renderScale > 4.0f) m_renderScale = 4.0f;
}

void WindowResize::setLogicalSize(int width, int height) {
    m_logicalWidth = width;
    m_logicalHeight = height;
}

void WindowResize::getLogicalSize(int& width, int& height) const {
    width = m_logicalWidth;
    height = m_logicalHeight;
}

void WindowResize::resizeWindow(int width, int height) {
    if (!m_window) return;
    SDL_SetWindowSize(m_window, width, height);
    m_windowedWidth = width;
    m_windowedHeight = height;
}

void WindowResize::maximize() {
    if (!m_window) return;
    SDL_MaximizeWindow(m_window);
}

void WindowResize::minimize() {
    if (!m_window) return;
    SDL_MinimizeWindow(m_window);
}

void WindowResize::restore() {
    if (!m_window) return;
    SDL_RestoreWindow(m_window);
}

void WindowResize::setPosition(int x, int y) {
    if (!m_window) return;
    SDL_SetWindowPosition(m_window, x, y);
}

void WindowResize::getPosition(int& x, int& y) const {
    if (!m_window) { x = 0; y = 0; return; }
    SDL_GetWindowPosition(m_window, &x, &y);
}

void WindowResize::centerWindow() {
    if (!m_window) return;
    SDL_SetWindowPosition(m_window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
}

void WindowResize::getWindowSize(int& width, int& height) const {
    if (!m_window) { width = m_windowedWidth; height = m_windowedHeight; return; }
    SDL_GetWindowSize(m_window, &width, &height);
}

void WindowResize::setVSync(bool enabled) {
    m_vsync = enabled;
    // SDL2 中 VSync 通过渲染器标志设置，这里仅记录状态
    // 实际切换需要重建渲染器
}

void WindowResize::update() {
    // 窗口事件由 SDL 事件循环处理
    // 这里可以添加窗口大小变化时的回调逻辑
}
