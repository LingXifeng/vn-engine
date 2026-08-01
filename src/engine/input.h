#pragma once

#include <SDL2/SDL.h>
#include <unordered_set>

// 输入系统类
class Input {
public:
    Input();
    ~Input();

    // 每帧开始时调用
    void update();
    void handleEvent(const SDL_Event& event);

    // 键盘
    bool isKeyDown(SDL_Scancode key) const;
    bool isKeyPressed(SDL_Scancode key) const;  // 当前帧按下
    bool isKeyReleased(SDL_Scancode key) const; // 当前帧释放

    // 鼠标
    bool isMouseButtonDown(int button) const;   // 1=左 2=中 3=右
    bool isMouseButtonPressed(int button) const;
    bool isMouseButtonReleased(int button) const;
    void getMousePosition(int& x, int& y) const;
    bool isMouseInRect(int x, int y, int w, int h) const;

    // 鼠标滚轮
    int getMouseWheelY() const { return m_wheelY; }

    // 通用：是否有任何点击/按键（用于文字推进）
    bool hasClick() const { return m_anyClick; }

private:
    const Uint8* m_keyboardState = nullptr;
    std::unordered_set<SDL_Scancode> m_keysPressed;
    std::unordered_set<SDL_Scancode> m_keysReleased;

    Uint32 m_mouseState = 0;
    Uint32 m_mousePressed = 0;
    Uint32 m_mouseReleased = 0;
    int m_mouseX = 0, m_mouseY = 0;
    int m_wheelY = 0;

    bool m_anyClick = false;
};
