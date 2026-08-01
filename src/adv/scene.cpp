#include "scene.h"
#include <algorithm>
#include <iostream>

Scene::Scene(Renderer* renderer, ResourceManager* resMgr, TweenManager* tweenMgr)
    : m_renderer(renderer), m_resMgr(resMgr), m_tweenMgr(tweenMgr) {
    m_textBox = std::make_unique<TextBox>(renderer);
    // 默认文字框位置：底部居中
    int sw = renderer->getWidth();
    int sh = renderer->getHeight();
    m_textBox->setPosition((sw - 800) / 2, sh - 180);
    m_textBox->setSize(800, 160);
}

Scene::~Scene() {}

void Scene::setBackground(const std::string& path, float fadeDuration) {
    auto tex = m_resMgr->getTexture(path);
    if (!tex) return;

    if (!m_background) {
        m_background = std::make_unique<Background>();
    }

    if (fadeDuration > 0 && m_background->texture) {
        // 淡入新背景
        auto oldTex = m_background->texture;
        Uint8 oldAlpha = m_background->alpha;
        m_background->texture = tex;
        m_background->alpha = 0;
        m_tweenMgr->add(fadeDuration,
            [this](float t) { m_background->alpha = (Uint8)(255 * t); },
            EaseType::EASE_IN_OUT_SINE
        );
    } else {
        m_background->texture = tex;
        m_background->alpha = 255;
    }
}

void Scene::clearBackground() {
    m_background = nullptr;
}

Character* Scene::addCharacter(const std::string& name) {
    auto ch = std::make_unique<Character>(m_renderer, name);
    ch->setBaseY(m_renderer->getHeight());
    Character* ptr = ch.get();
    m_characters[name] = std::move(ch);
    return ptr;
}

Character* Scene::getCharacter(const std::string& name) {
    auto it = m_characters.find(name);
    if (it != m_characters.end()) return it->second.get();
    return nullptr;
}

void Scene::removeCharacter(const std::string& name) {
    m_characters.erase(name);
}

void Scene::showCharacter(const std::string& name, const std::string& expression,
                          CharPosition pos, float fadeDuration) {
    Character* ch = getCharacter(name);
    if (!ch) ch = addCharacter(name);
    ch->setPosition(pos);
    ch->setExpression(expression);
    ch->show(m_tweenMgr, fadeDuration);
}

void Scene::hideCharacter(const std::string& name, float fadeDuration) {
    Character* ch = getCharacter(name);
    if (ch) ch->hide(m_tweenMgr, fadeDuration);
}

void Scene::clearCharacters() {
    for (auto& [name, ch] : m_characters) {
        ch->hide(m_tweenMgr, 0.3f);
    }
}

void Scene::showDialogue(const std::string& name, const std::string& text) {
    m_textBox->show(name, text);
}

void Scene::clearDialogue() {
    m_textBox->clear();
}

bool Scene::isDialogueWaiting() const {
    return m_textBox->isWaitingClick();
}

bool Scene::advanceDialogue() {
    return m_textBox->advance();
}

int Scene::addChoice(const std::string& text) {
    Choice choice;
    choice.text = text;
    choice.id = m_choices.size();
    m_choices.push_back(choice);
    m_choiceActive = true;
    return choice.id;
}

void Scene::clearChoices() {
    m_choices.clear();
    m_selectedChoice = -1;
    m_choiceActive = false;
}

void Scene::update(float dt, const Input& input) {
    m_textBox->update(dt);

    // 处理选项点击
    if (m_choiceActive && !m_choices.empty()) {
        int mx, my;
        input.getMousePosition(mx, my);

        int startY = m_renderer->getHeight() / 2 - m_choices.size() * 30;
        for (size_t i = 0; i < m_choices.size(); i++) {
            int choiceW = 400;
            int choiceH = 50;
            m_choices[i].rect = {
                (m_renderer->getWidth() - choiceW) / 2,
                startY + (int)i * 60,
                choiceW, choiceH
            };
            m_choices[i].hovered = input.isMouseInRect(
                m_choices[i].rect.x, m_choices[i].rect.y,
                m_choices[i].rect.w, m_choices[i].rect.h);

            if (m_choices[i].hovered && input.isMouseButtonPressed(1)) {
                m_selectedChoice = m_choices[i].id;
                m_choiceActive = false;
                if (m_choices[i].onSelect) m_choices[i].onSelect();
            }
        }
    }
}

void Scene::renderCharacters() {
    // 收集所有可见角色并按 zOrder 排序
    std::vector<Character*> chars;
    for (auto& [name, ch] : m_characters) {
        if (ch->isVisible()) chars.push_back(ch.get());
    }
    std::sort(chars.begin(), chars.end(),
        [](Character* a, Character* b) { return a->getZOrder() < b->getZOrder(); });
    for (auto* ch : chars) {
        ch->render(m_renderer);
    }
}

void Scene::renderChoices(const Input& input) {
    if (!m_choiceActive || m_choices.empty()) return;

    for (auto& choice : m_choices) {
        SDL_Color bgColor = choice.hovered
            ? SDL_Color{80, 80, 120, 220}
            : SDL_Color{40, 40, 60, 200};
        m_renderer->drawRect(choice.rect.x, choice.rect.y,
                             choice.rect.w, choice.rect.h, bgColor, true);
        SDL_Color borderColor = {120, 120, 160, 255};
        m_renderer->drawRect(choice.rect.x, choice.rect.y,
                             choice.rect.w, choice.rect.h, borderColor, false);
        // 文字渲染需要字体，这里简化处理
        // 实际使用时通过 TextBox 的字体渲染
    }
}

void Scene::render() {
    // 1. 背景
    if (m_background && m_background->visible && m_background->texture) {
        m_renderer->drawTexture(m_background->texture.get(),
                                m_background->x, m_background->y,
                                m_background->scaleX, m_background->scaleY,
                                m_background->alpha);
    }

    // 2. 角色立绘
    renderCharacters();

    // 3. 文字框
    m_textBox->render();

    // 4. 选项
    if (m_choiceActive) {
        // 选项文字需要单独渲染（需要字体）
        // 在完整实现中会在这里渲染选项文字
    }
}
