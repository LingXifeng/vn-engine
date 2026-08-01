#pragma once

#include <SDL2/SDL.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include "renderer.h"
#include "resource_manager.h"
#include "tween.h"
#include "input.h"
#include "text_box.h"
#include "character.h"

// 场景背景
struct Background {
    std::shared_ptr<Texture> texture;
    float x = 0, y = 0;
    float scaleX = 1.0f, scaleY = 1.0f;
    Uint8 alpha = 255;
    bool visible = true;
};

// 选项
struct Choice {
    std::string text;
    int id = 0;
    std::function<void()> onSelect;
    SDL_Rect rect;
    bool hovered = false;
};

// 场景类 - ADV 系统核心
class Scene {
public:
    Scene(Renderer* renderer, ResourceManager* resMgr, TweenManager* tweenMgr);
    ~Scene();

    // 背景
    void setBackground(const std::string& path, float fadeDuration = 0.0f);
    void clearBackground();

    // 角色
    Character* addCharacter(const std::string& name);
    Character* getCharacter(const std::string& name);
    void removeCharacter(const std::string& name);
    void showCharacter(const std::string& name, const std::string& expression,
                       CharPosition pos = CharPosition::LEFT,
                       float fadeDuration = 0.3f);
    void hideCharacter(const std::string& name, float fadeDuration = 0.3f);
    void clearCharacters();

    // 文字
    void showDialogue(const std::string& name, const std::string& text);
    void clearDialogue();
    bool isDialogueWaiting() const;
    bool advanceDialogue();  // 返回 true 表示可以推进

    // 选项
    int addChoice(const std::string& text);
    void clearChoices();
    bool hasChoices() const { return !m_choices.empty(); }
    int getSelectedChoice() const { return m_selectedChoice; }

    // 更新与渲染
    void update(float dt, const Input& input);
    void render();

    // 文字框配置
    TextBox* getTextBox() { return m_textBox.get(); }

private:
    Renderer* m_renderer;
    ResourceManager* m_resMgr;
    TweenManager* m_tweenMgr;

    std::unique_ptr<TextBox> m_textBox;
    std::unique_ptr<Background> m_background;
    std::unordered_map<std::string, std::unique_ptr<Character>> m_characters;

    std::vector<Choice> m_choices;
    int m_selectedChoice = -1;
    bool m_choiceActive = false;

    void renderChoices(const Input& input);
    void renderCharacters();
};
