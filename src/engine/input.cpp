#include "input.h"

Input::Input() {
    m_keyboardState = SDL_GetKeyboardState(nullptr);
}

Input::~Input() {}

void Input::update() {
    m_keysPressed.clear();
    m_keysReleased.clear();
    m_mousePressed = 0;
    m_mouseReleased = 0;
    m_wheelY = 0;
    m_anyClick = false;
}

void Input::handleEvent(const SDL_Event& event) {
    switch (event.type) {
        case SDL_KEYDOWN:
            if (!event.key.repeat) {
                m_keysPressed.insert(event.key.keysym.scancode);
                m_anyClick = true;
            }
            break;
        case SDL_KEYUP:
            m_keysReleased.insert(event.key.keysym.scancode);
            break;
        case SDL_MOUSEBUTTONDOWN:
            m_mousePressed |= (1 << (event.button.button - 1));
            m_mouseState |= (1 << (event.button.button - 1));
            if (event.button.button == 1) m_anyClick = true;
            break;
        case SDL_MOUSEBUTTONUP:
            m_mouseReleased |= (1 << (event.button.button - 1));
            m_mouseState &= ~(1 << (event.button.button - 1));
            break;
        case SDL_MOUSEMOTION:
            m_mouseX = event.motion.x;
            m_mouseY = event.motion.y;
            break;
        case SDL_MOUSEWHEEL:
            m_wheelY = event.wheel.y;
            break;
    }
}

bool Input::isKeyDown(SDL_Scancode key) const {
    return m_keyboardState[key];
}

bool Input::isKeyPressed(SDL_Scancode key) const {
    return m_keysPressed.count(key) > 0;
}

bool Input::isKeyReleased(SDL_Scancode key) const {
    return m_keysReleased.count(key) > 0;
}

bool Input::isMouseButtonDown(int button) const {
    return (m_mouseState & (1 << (button - 1))) != 0;
}

bool Input::isMouseButtonPressed(int button) const {
    return (m_mousePressed & (1 << (button - 1))) != 0;
}

bool Input::isMouseButtonReleased(int button) const {
    return (m_mouseReleased & (1 << (button - 1))) != 0;
}

void Input::getMousePosition(int& x, int& y) const {
    x = m_mouseX;
    y = m_mouseY;
}

bool Input::isMouseInRect(int x, int y, int w, int h) const {
    return m_mouseX >= x && m_mouseX < x + w &&
           m_mouseY >= y && m_mouseY < y + h;
}
