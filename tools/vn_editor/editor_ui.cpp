#include "editor_ui.h"
#include "lua_exporter.h"

#include <SDL2/SDL_image.h>
#include <cstring>
#include <algorithm>

// ============================================================================
// UI 辅助函数实现
// ============================================================================

namespace UI {

void fillRect(UICtx& ctx, int x, int y, int w, int h, SDL_Color color) {
    SDL_SetRenderDrawColor(ctx.renderer, color.r, color.g, color.b, color.a);
    SDL_Rect r = {x, y, w, h};
    SDL_RenderFillRect(ctx.renderer, &r);
}

void drawRect(UICtx& ctx, int x, int y, int w, int h, SDL_Color color) {
    SDL_SetRenderDrawColor(ctx.renderer, color.r, color.g, color.b, color.a);
    SDL_Rect r = {x, y, w, h};
    SDL_RenderDrawRect(ctx.renderer, &r);
}

void text(UICtx& ctx, const std::string& s, int x, int y, SDL_Color color) {
    if (!ctx.font || s.empty()) return;
    SDL_Surface* surf = TTF_RenderUTF8_Blended(ctx.font, s.c_str(), color);
    if (!surf) return;
    SDL_Texture* tex = SDL_CreateTextureFromSurface(ctx.renderer, surf);
    if (tex) {
        SDL_Rect dst = {x, y, surf->w, surf->h};
        SDL_RenderCopy(ctx.renderer, tex, nullptr, &dst);
        SDL_DestroyTexture(tex);
    }
    SDL_FreeSurface(surf);
}

void textSmall(UICtx& ctx, const std::string& s, int x, int y, SDL_Color color) {
    if (!ctx.fontSmall || s.empty()) return;
    SDL_Surface* surf = TTF_RenderUTF8_Blended(ctx.fontSmall, s.c_str(), color);
    if (!surf) return;
    SDL_Texture* tex = SDL_CreateTextureFromSurface(ctx.renderer, surf);
    if (tex) {
        SDL_Rect dst = {x, y, surf->w, surf->h};
        SDL_RenderCopy(ctx.renderer, tex, nullptr, &dst);
        SDL_DestroyTexture(tex);
    }
    SDL_FreeSurface(surf);
}

int textWidth(UICtx& ctx, const std::string& s) {
    if (!ctx.font || s.empty()) return 0;
    int w = 0, h = 0;
    TTF_SizeUTF8(ctx.font, s.c_str(), &w, &h);
    return w;
}

int textHeight(UICtx& ctx) {
    if (!ctx.font) return 16;
    return TTF_FontHeight(ctx.font);
}

bool mouseInRect(UICtx& ctx, int x, int y, int w, int h) {
    return ctx.mouseX >= x && ctx.mouseX < x + w &&
           ctx.mouseY >= y && ctx.mouseY < y + h;
}

bool button(UICtx& ctx, const std::string& label, int x, int y, int w, int h) {
    bool hover = mouseInRect(ctx, x, y, w, h);
    bool clicked = false;

    SDL_Color bg = hover ? ctx.colBtnHover : ctx.colBtn;
    fillRect(ctx, x, y, w, h, bg);
    drawRect(ctx, x, y, w, h, ctx.colBorder);

    // 居中文本
    int tw = textWidth(ctx, label);
    int th = textHeight(ctx);
    text(ctx, label, x + (w - tw) / 2, y + (h - th) / 2, ctx.colText);

    if (hover && ctx.mouseReleased) {
        clicked = true;
    }

    return clicked;
}

bool listItem(UICtx& ctx, const std::string& label, int x, int y, int w, int h,
              bool selected) {
    bool hover = mouseInRect(ctx, x, y, w, h);
    bool clicked = false;

    SDL_Color bg = selected ? ctx.colSelect : (hover ? ctx.colHover : ctx.colPanel);
    fillRect(ctx, x, y, w, h, bg);

    int th = textHeight(ctx);
    text(ctx, label, x + 8, y + (h - th) / 2, ctx.colText);

    if (hover && ctx.mouseReleased) {
        clicked = true;
    }

    return clicked;
}

void panel(UICtx& ctx, int x, int y, int w, int h, const std::string& title) {
    fillRect(ctx, x, y, w, h, ctx.colPanel);
    drawRect(ctx, x, y, w, h, ctx.colBorder);

    if (!title.empty()) {
        // 标题栏
        fillRect(ctx, x, y, w, 24, ctx.colBg);
        drawRect(ctx, x, y, w, 24, ctx.colBorder);
        int th = textHeight(ctx);
        text(ctx, title, x + 8, y + (24 - th) / 2, ctx.colAccent);
    }
}

bool textInput(UICtx& ctx, std::string& value, int x, int y, int w, int h,
               const std::string& hint) {
    bool hover = mouseInRect(ctx, x, y, w, h);
    bool changed = false;

    // 点击激活/取消激活
    if (ctx.mouseReleased) {
        ctx.textInputActive = hover;
        if (hover) {
            ctx.textInput = value;
            ctx.textCursor = (int)value.size();
        }
    }

    SDL_Color bg = ctx.textInputActive ? ctx.colHover : ctx.colPanel;
    fillRect(ctx, x, y, w, h, bg);
    drawRect(ctx, x, y, w, h, ctx.textInputActive ? ctx.colAccent : ctx.colBorder);

    std::string display = ctx.textInputActive ? ctx.textInput : value;
    if (display.empty() && !hint.empty()) {
        text(ctx, hint, x + 6, y + 4, ctx.colTextDim);
    } else {
        text(ctx, display, x + 6, y + 4, ctx.colText);
    }

    // 光标
    if (ctx.textInputActive) {
        int cw = textWidth(ctx, ctx.textInput.substr(0, ctx.textCursor));
        fillRect(ctx, x + 6 + cw, y + 4, 2, h - 8, ctx.colText);
    }

    // 提交
    if (ctx.textInputActive) {
        value = ctx.textInput;
        changed = true;
    }

    return changed;
}

bool dropdown(UICtx& ctx, const std::vector<std::string>& options, int& selected,
              int x, int y, int w, int h) {
    if (options.empty()) return false;
    if (selected < 0 || selected >= (int)options.size()) selected = 0;

    bool hover = mouseInRect(ctx, x, y, w, h);
    fillRect(ctx, x, y, w, h, hover ? ctx.colBtnHover : ctx.colBtn);
    drawRect(ctx, x, y, w, h, ctx.colBorder);

    text(ctx, options[selected], x + 6, y + 4, ctx.colText);
    text(ctx, "v", x + w - 16, y + 4, ctx.colTextDim);

    // 简易下拉：点击时循环选择
    if (hover && ctx.mouseReleased) {
        selected = (selected + 1) % (int)options.size();
        return true;
    }

    return false;
}

} // namespace UI

// ============================================================================
// EditorUI 实现
// ============================================================================

EditorUI::EditorUI() {}

EditorUI::~EditorUI() {
    shutdown();
}

bool EditorUI::init(int width, int height) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return false;
    }

    if (TTF_Init() < 0) {
        fprintf(stderr, "TTF_Init failed: %s\n", TTF_GetError());
        return false;
    }

    m_window = SDL_CreateWindow(
        "VN Editor",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        width, height,
        SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
    );
    if (!m_window) {
        fprintf(stderr, "Window creation failed: %s\n", SDL_GetError());
        return false;
    }

    m_renderer = SDL_CreateRenderer(m_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);
    if (!m_renderer) {
        m_renderer = SDL_CreateRenderer(m_window, -1, SDL_RENDERER_SOFTWARE);
    }
    if (!m_renderer) {
        fprintf(stderr, "Renderer creation failed: %s\n", SDL_GetError());
        return false;
    }

    // 加载字体
    const char* fontPaths[] = {
        "assets/fonts/DroidSansJapanese.ttf",
        "/root/Escue/vn_engine/assets/fonts/DroidSansJapanese.ttf",
        "/usr/share/fonts/google-droid-fonts/DroidSansJapanese.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        nullptr
    };
    for (int i = 0; fontPaths[i]; i++) {
        m_font = TTF_OpenFont(fontPaths[i], 16);
        if (m_font) {
            m_fontSmall = TTF_OpenFont(fontPaths[i], 12);
            break;
        }
    }
    if (!m_font) {
        fprintf(stderr, "Font loading failed: %s\n", TTF_GetError());
        return false;
    }

    // 初始化 UI 上下文
    m_ctx.renderer = m_renderer;
    m_ctx.font = m_font;
    m_ctx.fontSmall = m_fontSmall;
    m_ctx.width = width;
    m_ctx.height = height;

    SDL_StartTextInput();
    return true;
}

void EditorUI::shutdown() {
    SDL_StopTextInput();
    if (m_font) { TTF_CloseFont(m_font); m_font = nullptr; }
    if (m_fontSmall) { TTF_CloseFont(m_fontSmall); m_fontSmall = nullptr; }
    if (m_renderer) { SDL_DestroyRenderer(m_renderer); m_renderer = nullptr; }
    if (m_window) { SDL_DestroyWindow(m_window); m_window = nullptr; }
    TTF_Quit();
    SDL_Quit();
}

// 旧版 run(EditorProject&) 已移除，使用 run(EditorProject&, const std::string&) 代替

void EditorUI::handleEvent(SDL_Event& event, EditorProject& project) {
    switch (event.type) {
        case SDL_QUIT:
            m_running = false;
            break;

        case SDL_MOUSEBUTTONDOWN:
            if (event.button.button == SDL_BUTTON_LEFT) {
                m_ctx.mousePressed = true;
                m_ctx.clickX = event.button.x;
                m_ctx.clickY = event.button.y;
            }
            break;

        case SDL_MOUSEBUTTONUP:
            if (event.button.button == SDL_BUTTON_LEFT) {
                m_ctx.mouseReleased = true;
            }
            break;

        case SDL_MOUSEMOTION:
            m_ctx.mouseX = event.motion.x;
            m_ctx.mouseY = event.motion.y;
            break;

        case SDL_KEYDOWN:
            if (m_ctx.textInputActive) {
                switch (event.key.keysym.sym) {
                    case SDLK_BACKSPACE:
                        if (m_ctx.textCursor > 0) {
                            m_ctx.textInput.erase(m_ctx.textCursor - 1, 1);
                            m_ctx.textCursor--;
                        }
                        break;
                    case SDLK_DELETE:
                        if (m_ctx.textCursor < (int)m_ctx.textInput.size()) {
                            m_ctx.textInput.erase(m_ctx.textCursor, 1);
                        }
                        break;
                    case SDLK_LEFT:
                        if (m_ctx.textCursor > 0) m_ctx.textCursor--;
                        break;
                    case SDLK_RIGHT:
                        if (m_ctx.textCursor < (int)m_ctx.textInput.size()) m_ctx.textCursor++;
                        break;
                    case SDLK_HOME:
                        m_ctx.textCursor = 0;
                        break;
                    case SDLK_END:
                        m_ctx.textCursor = (int)m_ctx.textInput.size();
                        break;
                    case SDLK_RETURN:
                    case SDLK_KP_ENTER:
                        m_ctx.textInputActive = false;
                        break;
                    case SDLK_ESCAPE:
                        m_ctx.textInputActive = false;
                        m_ctx.textInput = "";
                        break;
                }
            } else {
                // 全局快捷键
                switch (event.key.keysym.sym) {
                    case SDLK_ESCAPE:
                        if (m_mode != EditorMode::Normal) {
                            m_mode = EditorMode::Normal;
                        } else {
                            m_running = false;
                        }
                        break;
                }
            }
            break;

        case SDL_TEXTINPUT:
            if (m_ctx.textInputActive) {
                m_ctx.textInput.insert(m_ctx.textCursor, event.text.text);
                m_ctx.textCursor += strlen(event.text.text);
            }
            break;
    }
}

// ============================================================================
// 渲染 — 菜单栏
// ============================================================================

void EditorUI::renderMenuBar(EditorProject& project) {
    int y = 0;
    int w = m_ctx.width;
    UI::panel(m_ctx, 0, y, w, m_menuBarH);

    int x = 8;
    int btnW = 70, btnH = 22;

    if (UI::button(m_ctx, "新建", x, y + 4, btnW, btnH)) {
        m_mode = EditorMode::NewProject;
        m_dlgInput1 = "NewProject";
        m_dlgInput2 = "Unknown";
    }
    x += btnW + 4;

    if (UI::button(m_ctx, "打开", x, y + 4, btnW, btnH)) {
        menuOpen(project, "");
    }
    x += btnW + 4;

    if (UI::button(m_ctx, "保存", x, y + 4, btnW, btnH)) {
        menuSave(project);
    }
    x += btnW + 4;

    if (UI::button(m_ctx, "导出", x, y + 4, btnW, btnH)) {
        m_mode = EditorMode::ExportDialog;
        m_exportDir = "exported";
    }
    x += btnW + 4;

    // 右侧信息
    std::string info = project.name + " (v" + project.version + ")";
    if (m_modified) info += " *";
    UI::text(m_ctx, info, w - 250, y + 6, m_ctx.colTextDim);
}

// ============================================================================
// 渲染 — 场景列表（左面板）
// ============================================================================

void EditorUI::renderSceneList(EditorProject& project) {
    int x = 0;
    int y = m_menuBarH;
    int w = m_leftPanelW;
    int h = m_ctx.height - m_menuBarH - m_statusBarH;

    UI::panel(m_ctx, x, y, w, h, "场景列表");

    int itemY = y + 28;
    int itemH = 24;

    for (int i = 0; i < (int)project.scenes.size(); i++) {
        auto& sc = project.scenes[i];
        if (itemY + itemH > y + h) break;

        std::string label = sc.name + " [" + sc.id + "]";
        if (sc.id == project.startScene) label = ">> " + label;

        if (UI::listItem(m_ctx, label, x + 2, itemY, w - 4, itemH, m_selectedScene == i)) {
            m_selectedScene = i;
            m_selectedLine = -1;
        }
        itemY += itemH;
    }

    // 添加/删除场景按钮
    int btnY = y + h - 30;
    if (UI::button(m_ctx, "+ 添加", x + 8, btnY, 80, 22)) {
        m_mode = EditorMode::AddScene;
        m_dlgInput1 = "scene" + std::to_string(project.scenes.size() + 1);
        m_dlgInput2 = "Scene " + std::to_string(project.scenes.size() + 1);
    }
    if (UI::button(m_ctx, "- 删除", x + 92, btnY, 80, 22)) {
        if (m_selectedScene >= 0 && m_selectedScene < (int)project.scenes.size()) {
            m_mode = EditorMode::ConfirmDelete;
            m_dlgMessage = "确认删除场景: " + project.scenes[m_selectedScene].name + "?";
        }
    }
    // 场景重排序
    if (UI::button(m_ctx, "↑", x + 176, btnY, 30, 22)) {
        if (m_selectedScene > 0) {
            std::swap(project.scenes[m_selectedScene], project.scenes[m_selectedScene - 1]);
            m_selectedScene--;
            m_modified = true;
        }
    }
    if (UI::button(m_ctx, "↓", x + 210, btnY, 30, 22)) {
        if (m_selectedScene >= 0 && m_selectedScene < (int)project.scenes.size() - 1) {
            std::swap(project.scenes[m_selectedScene], project.scenes[m_selectedScene + 1]);
            m_selectedScene++;
            m_modified = true;
        }
    }
}

// ============================================================================
// 渲染 — 对话编辑器（中央面板）
// ============================================================================

void EditorUI::renderDialogueEditor(EditorProject& project) {
    int x = m_leftPanelW;
    int y = m_menuBarH;
    int w = m_ctx.width - m_leftPanelW - m_rightPanelW;
    int h = m_ctx.height - m_menuBarH - m_statusBarH;

    UI::panel(m_ctx, x, y, w, h, "对话编辑器");

    if (m_selectedScene < 0 || m_selectedScene >= (int)project.scenes.size()) {
        UI::text(m_ctx, "请选择或添加一个场景", x + 16, y + 40, m_ctx.colTextDim);
        return;
    }

    auto& scene = project.scenes[m_selectedScene];

    // 场景属性编辑
    int propY = y + 30;
    UI::textSmall(m_ctx, "场景ID:", x + 8, propY + 4, m_ctx.colTextDim);
    static std::string sceneIdEdit;
    sceneIdEdit = scene.id;
    if (UI::textInput(m_ctx, sceneIdEdit, x + 60, propY, 120, 22)) {
        scene.id = sceneIdEdit;
        m_modified = true;
    }

    UI::textSmall(m_ctx, "名称:", x + 190, propY + 4, m_ctx.colTextDim);
    static std::string sceneNameEdit;
    sceneNameEdit = scene.name;
    if (UI::textInput(m_ctx, sceneNameEdit, x + 230, propY, 150, 22)) {
        scene.name = sceneNameEdit;
        m_modified = true;
    }

    UI::textSmall(m_ctx, "背景:", x + 390, propY + 4, m_ctx.colTextDim);
    static std::string sceneBgEdit;
    sceneBgEdit = scene.background;
    if (UI::textInput(m_ctx, sceneBgEdit, x + 430, propY, 100, 22, "bg_id")) {
        scene.background = sceneBgEdit;
        m_modified = true;
    }

    UI::textSmall(m_ctx, "BGM:", x + 540, propY + 4, m_ctx.colTextDim);
    static std::string sceneBgmEdit;
    sceneBgmEdit = scene.bgm;
    if (UI::textInput(m_ctx, sceneBgmEdit, x + 575, propY, 100, 22, "bgm_id")) {
        scene.bgm = sceneBgmEdit;
        m_modified = true;
    }

    // 对话行列表
    int listY = propY + 30;
    int lineH = 28;
    int listH = h - (listY - y) - 35;

    UI::fillRect(m_ctx, x + 4, listY, w - 8, listH, m_ctx.colBg);
    UI::drawRect(m_ctx, x + 4, listY, w - 8, listH, m_ctx.colBorder);

    int itemY = listY + 4;
    for (int i = 0; i < (int)scene.lines.size(); i++) {
        if (itemY + lineH > listY + listH) break;

        auto& line = scene.lines[i];
        bool selected = (m_selectedLine == i);

        // 行类型标签
        const char* typeLabel = EditorProject::lineTypeName(line.type);
        SDL_Color typeColor = m_ctx.colAccent;
        if (line.type == LineType::Say) typeColor = {200, 200, 100, 255};
        else if (line.type == LineType::Narrate) typeColor = {150, 200, 150, 255};
        else if (line.type == LineType::Choice) typeColor = {200, 150, 200, 255};

        SDL_Color bg = selected ? m_ctx.colSelect : m_ctx.colPanel;
        if (UI::mouseInRect(m_ctx, x + 6, itemY, w - 12, lineH) && !selected)
            bg = m_ctx.colHover;
        UI::fillRect(m_ctx, x + 6, itemY, w - 12, lineH, bg);

        // 序号
        UI::textSmall(m_ctx, std::to_string(i + 1), x + 10, itemY + 6, m_ctx.colTextDim);
        // 类型
        UI::textSmall(m_ctx, typeLabel, x + 35, itemY + 6, typeColor);

        // 内容摘要
        std::string summary;
        switch (line.type) {
            case LineType::Say:
                summary = "[" + line.character + "] " + line.text;
                break;
            case LineType::Narrate:
                summary = line.text;
                break;
            case LineType::Choice:
                summary = "选项: " + std::to_string(line.choices.size()) + "个";
                break;
            case LineType::Goto:
                summary = "-> " + line.targetScene;
                break;
            case LineType::BGM:
                summary = "播放: " + line.resourceId;
                break;
            case LineType::SFX:
                summary = "音效: " + line.resourceId;
                break;
            case LineType::BG:
                summary = "背景: " + line.resourceId;
                break;
            case LineType::Ending:
                summary = "结局: " + line.endingTitle;
                break;
            default:
                summary = line.text;
                break;
        }
        if (summary.size() > 50) summary = summary.substr(0, 50) + "...";
        UI::textSmall(m_ctx, summary, x + 80, itemY + 6, m_ctx.colText);

        // 点击选择
        if (UI::mouseInRect(m_ctx, x + 6, itemY, w - 12, lineH) && m_ctx.mouseReleased) {
            m_selectedLine = i;
        }

        itemY += lineH;
    }

    // 添加/删除行按钮
    int btnY = y + h - 28;
    if (UI::button(m_ctx, "+ 添加行", x + 8, btnY, 90, 22)) {
        m_mode = EditorMode::AddLine;
        m_dlgTypeSel = 1;  // 默认 Say 类型
    }
    if (UI::button(m_ctx, "- 删除行", x + 102, btnY, 90, 22)) {
        if (m_selectedLine >= 0 && m_selectedLine < (int)scene.lines.size()) {
            scene.lines.erase(scene.lines.begin() + m_selectedLine);
            m_selectedLine = -1;
            m_modified = true;
            m_statusMsg = "已删除对话行";
        }
    }
    if (UI::button(m_ctx, "上移", x + 196, btnY, 60, 22)) {
        if (m_selectedLine > 0) {
            std::swap(scene.lines[m_selectedLine], scene.lines[m_selectedLine - 1]);
            m_selectedLine--;
            m_modified = true;
        }
    }
    if (UI::button(m_ctx, "下移", x + 260, btnY, 60, 22)) {
        if (m_selectedLine >= 0 && m_selectedLine < (int)scene.lines.size() - 1) {
            std::swap(scene.lines[m_selectedLine], scene.lines[m_selectedLine + 1]);
            m_selectedLine++;
            m_modified = true;
        }
    }
}

// ============================================================================
// 渲染 — 属性面板（右面板）
// ============================================================================

void EditorUI::renderPropertiesPanel(EditorProject& project) {
    int x = m_ctx.width - m_rightPanelW;
    int y = m_menuBarH;
    int w = m_rightPanelW;
    int h = m_ctx.height - m_menuBarH - m_statusBarH;

    UI::panel(m_ctx, x, y, w, h, "属性");

    int cy = y + 32;

    if (m_selectedScene < 0 || m_selectedScene >= (int)project.scenes.size()) {
        UI::text(m_ctx, "未选择场景", x + 12, cy, m_ctx.colTextDim);
        return;
    }

    auto& scene = project.scenes[m_selectedScene];

    // 项目信息
    UI::textSmall(m_ctx, "— 项目信息 —", x + 12, cy, m_ctx.colAccent);
    cy += 22;
    UI::textSmall(m_ctx, "名称: " + project.name, x + 12, cy, m_ctx.colText);
    cy += 18;
    UI::textSmall(m_ctx, "作者: " + project.author, x + 12, cy, m_ctx.colText);
    cy += 18;
    UI::textSmall(m_ctx, "版本: " + project.version, x + 12, cy, m_ctx.colText);
    cy += 18;
    UI::textSmall(m_ctx, "起始: " + project.startScene, x + 12, cy, m_ctx.colText);
    cy += 24;

    // 场景统计
    UI::textSmall(m_ctx, "— 场景统计 —", x + 12, cy, m_ctx.colAccent);
    cy += 22;
    UI::textSmall(m_ctx, "对话行数: " + std::to_string(scene.lines.size()), x + 12, cy, m_ctx.colText);
    cy += 18;

    int sayCount = 0, narrateCount = 0, choiceCount = 0;
    for (auto& l : scene.lines) {
        if (l.type == LineType::Say) sayCount++;
        else if (l.type == LineType::Narrate) narrateCount++;
        else if (l.type == LineType::Choice) choiceCount++;
    }
    UI::textSmall(m_ctx, "说话: " + std::to_string(sayCount) + " 旁白: " + std::to_string(narrateCount), x + 12, cy, m_ctx.colText);
    cy += 18;
    UI::textSmall(m_ctx, "选项: " + std::to_string(choiceCount), x + 12, cy, m_ctx.colText);
    cy += 24;

    // 对话行编辑
    if (m_selectedLine >= 0 && m_selectedLine < (int)scene.lines.size()) {
        auto& line = scene.lines[m_selectedLine];

        UI::textSmall(m_ctx, "— 行编辑 —", x + 12, cy, m_ctx.colAccent);
        cy += 22;

        // 类型选择
        UI::textSmall(m_ctx, "类型:", x + 12, cy + 4, m_ctx.colTextDim);
        static int typeSel = 1;
        std::vector<std::string> types = {"narrate", "say", "choice", "goto", "wait",
            "bgm", "bgm_stop", "sfx", "bg", "fadein", "fadeout", "ending", "label"};
        // 同步当前类型
        for (int i = 0; i < (int)types.size(); i++) {
            if (types[i] == EditorProject::lineTypeName(line.type)) { typeSel = i; break; }
        }
        if (UI::dropdown(m_ctx, types, typeSel, x + 60, cy, 120, 22)) {
            line.type = EditorProject::lineTypeFromName(types[typeSel]);
            m_modified = true;
        }
        cy += 28;

        // 根据类型显示不同编辑器
        if (line.type == LineType::Say || line.type == LineType::Narrate) {
            UI::textSmall(m_ctx, "文本:", x + 12, cy + 4, m_ctx.colTextDim);
            if (UI::textInput(m_ctx, line.text, x + 60, cy, w - 72, 22, "输入文本...")) {
                m_modified = true;
            }
            cy += 28;

            if (line.type == LineType::Say) {
                UI::textSmall(m_ctx, "角色:", x + 12, cy + 4, m_ctx.colTextDim);
                if (UI::textInput(m_ctx, line.character, x + 60, cy, 100, 22, "char_id")) {
                    m_modified = true;
                }
                UI::textSmall(m_ctx, "表情:", x + 170, cy + 4, m_ctx.colTextDim);
                if (UI::textInput(m_ctx, line.expression, x + 210, cy, 60, 22, "expr")) {
                    m_modified = true;
                }
                cy += 28;
            }
        } else if (line.type == LineType::Choice) {
            UI::textSmall(m_ctx, "选项列表:", x + 12, cy, m_ctx.colTextDim);
            cy += 20;
            for (int ci = 0; ci < (int)line.choices.size(); ci++) {
                UI::textSmall(m_ctx, std::to_string(ci + 1) + ":", x + 12, cy + 4, m_ctx.colTextDim);
                if (UI::textInput(m_ctx, line.choices[ci].text, x + 30, cy, 120, 22, "选项文本")) {
                    m_modified = true;
                }
                if (UI::textInput(m_ctx, line.choices[ci].nextScene, x + 155, cy, 100, 22, "目标场景")) {
                    m_modified = true;
                }
                cy += 26;
            }
            if (UI::button(m_ctx, "+ 添加选项", x + 12, cy, 100, 20)) {
                line.choices.push_back({"选项", ""});
                m_modified = true;
            }
            if (!line.choices.empty() && UI::button(m_ctx, "- 删除", x + 120, cy, 80, 20)) {
                line.choices.pop_back();
                m_modified = true;
            }
            cy += 28;
        } else if (line.type == LineType::Goto) {
            UI::textSmall(m_ctx, "目标场景:", x + 12, cy + 4, m_ctx.colTextDim);
            if (UI::textInput(m_ctx, line.targetScene, x + 80, cy, 120, 22, "scene_id")) {
                m_modified = true;
            }
            cy += 28;
        } else if (line.type == LineType::BGM || line.type == LineType::SFX || line.type == LineType::BG) {
            UI::textSmall(m_ctx, "资源ID:", x + 12, cy + 4, m_ctx.colTextDim);
            if (UI::textInput(m_ctx, line.resourceId, x + 70, cy, 120, 22, "resource_id")) {
                m_modified = true;
            }
            cy += 28;
        } else if (line.type == LineType::Wait) {
            UI::textSmall(m_ctx, "等待时间:", x + 12, cy + 4, m_ctx.colTextDim);
            static std::string waitStr;
            waitStr = std::to_string(line.waitTime);
            if (UI::textInput(m_ctx, waitStr, x + 80, cy, 80, 22, "秒")) {
                line.waitTime = atof(waitStr.c_str());
                m_modified = true;
            }
            cy += 28;
        } else if (line.type == LineType::FadeIn || line.type == LineType::FadeOut) {
            UI::textSmall(m_ctx, "淡入/淡出时间:", x + 12, cy + 4, m_ctx.colTextDim);
            static std::string fadeStr;
            fadeStr = std::to_string(line.fadeTime);
            if (UI::textInput(m_ctx, fadeStr, x + 100, cy, 80, 22, "秒")) {
                line.fadeTime = atof(fadeStr.c_str());
                m_modified = true;
            }
            cy += 28;
        } else if (line.type == LineType::Ending) {
            UI::textSmall(m_ctx, "结局ID:", x + 12, cy + 4, m_ctx.colTextDim);
            if (UI::textInput(m_ctx, line.endingId, x + 70, cy, 100, 22, "ending_id")) {
                m_modified = true;
            }
            cy += 26;
            UI::textSmall(m_ctx, "标题:", x + 12, cy + 4, m_ctx.colTextDim);
            if (UI::textInput(m_ctx, line.endingTitle, x + 70, cy, 150, 22, "结局标题")) {
                m_modified = true;
            }
            cy += 26;
            UI::textSmall(m_ctx, "描述:", x + 12, cy + 4, m_ctx.colTextDim);
            if (UI::textInput(m_ctx, line.endingDesc, x + 70, cy, 180, 22, "结局描述")) {
                m_modified = true;
            }
            cy += 28;
        } else if (line.type == LineType::Label) {
            UI::textSmall(m_ctx, "标签名:", x + 12, cy + 4, m_ctx.colTextDim);
            if (UI::textInput(m_ctx, line.label, x + 70, cy, 120, 22, "label_name")) {
                m_modified = true;
            }
            cy += 28;
        }
    }

    // 角色管理
    cy = y + h - 200;
    UI::textSmall(m_ctx, "— 角色管理 —", x + 12, cy, m_ctx.colAccent);
    cy += 20;

    // 角色列表（可点击选择）
    int charListH = 100;
    int charItemH = 18;
    for (int i = 0; i < (int)project.characters.size() && cy < y + h - 60; i++) {
        auto& ch = project.characters[i];
        std::string label = ch.id + ": " + ch.name;
        if (label.size() > 25) label = label.substr(0, 25);

        bool selected = (m_selectedCharacter == i);
        bool hover = UI::mouseInRect(m_ctx, x + 8, cy, w - 16, charItemH);
        SDL_Color bg = selected ? m_ctx.colSelect : (hover ? m_ctx.colHover : m_ctx.colPanel);
        UI::fillRect(m_ctx, x + 8, cy, w - 16, charItemH, bg);
        UI::textSmall(m_ctx, label, x + 12, cy + 2, m_ctx.colText);

        if (hover && m_ctx.mouseReleased) {
            m_selectedCharacter = i;
        }
        cy += charItemH;
    }

    // 角色操作按钮
    int charBtnY = y + h - 55;
    if (UI::button(m_ctx, "+ 角色", x + 8, charBtnY, 70, 20)) {
        m_mode = EditorMode::AddCharacter;
        m_dlgInput1 = "char" + std::to_string(project.characters.size() + 1);
        m_dlgInput2 = "角色" + std::to_string(project.characters.size() + 1);
    }
    if (UI::button(m_ctx, "- 删除", x + 82, charBtnY, 60, 20)) {
        if (m_selectedCharacter >= 0 && m_selectedCharacter < (int)project.characters.size()) {
            m_mode = EditorMode::ConfirmDelete;
            m_dlgMessage = "确认删除角色: " + project.characters[m_selectedCharacter].name + "?";
        }
    }

    // 选中角色的属性编辑
    if (m_selectedCharacter >= 0 && m_selectedCharacter < (int)project.characters.size()) {
        auto& ch = project.characters[m_selectedCharacter];
        int editY = charBtnY + 24;

        UI::textSmall(m_ctx, "ID:", x + 8, editY + 2, m_ctx.colTextDim);
        if (UI::textInput(m_ctx, ch.id, x + 30, editY, 80, 18, "id")) {
            m_modified = true;
        }
        UI::textSmall(m_ctx, "名:", x + 118, editY + 2, m_ctx.colTextDim);
        if (UI::textInput(m_ctx, ch.name, x + 140, editY, 100, 18, "显示名")) {
            m_modified = true;
        }
    }
}

// ============================================================================
// 渲染 — 状态栏
// ============================================================================

void EditorUI::renderStatusBar() {
    int y = m_ctx.height - m_statusBarH;
    UI::panel(m_ctx, 0, y, m_ctx.width, m_statusBarH);

    UI::textSmall(m_ctx, m_statusMsg, 8, y + 5, m_ctx.colTextDim);

    std::string pos = "X:" + std::to_string(m_ctx.mouseX) + " Y:" + std::to_string(m_ctx.mouseY);
    UI::textSmall(m_ctx, pos, m_ctx.width - 100, y + 5, m_ctx.colTextDim);
}

// ============================================================================
// 渲染 — 对话框
// ============================================================================

void EditorUI::renderDialog() {
    if (!m_dialogOpen) return;

    // 半透明遮罩
    SDL_SetRenderDrawColor(m_ctx.renderer, 0, 0, 0, 160);
    SDL_Rect overlay = {0, 0, m_ctx.width, m_ctx.height};
    SDL_RenderFillRect(m_ctx.renderer, &overlay);

    int dw = 400, dh = 160;
    int dx = (m_ctx.width - dw) / 2;
    int dy = (m_ctx.height - dh) / 2;

    UI::panel(m_ctx, dx, dy, dw, dh, m_dialogTitle);

    int cy = dy + 40;
    for (auto& msg : m_dialogMessages) {
        UI::text(m_ctx, msg, dx + 20, cy, m_ctx.colText);
        cy += 24;
    }

    int by = dy + dh - 36;
    if (UI::button(m_ctx, "确定", dx + dw - 180, by, 70, 24)) {
        m_dialogOpen = false;
    }
    if (UI::button(m_ctx, "取消", dx + dw - 90, by, 70, 24)) {
        m_dialogOpen = false;
    }
}

// ============================================================================
// 菜单动作
// ============================================================================

void EditorUI::menuNew(EditorProject& project) {
    project = EditorProject();
    project.name = "新项目";
    project.author = "未知";
    project.version = "1.0";
    project.startScene = "scene1";

    EditorScene s;
    s.id = "scene1";
    s.name = "场景1";
    s.background = "";
    s.bgm = "";
    project.scenes.push_back(s);

    m_selectedScene = 0;
    m_selectedLine = -1;
    m_modified = false;
    m_statusMsg = "已创建新项目";
}

void EditorUI::menuOpen(EditorProject& project, const std::string& path) {
    if (path.empty()) {
        m_dialogTitle = "打开项目";
        m_dialogMessages = {"请通过命令行指定项目文件路径:", "vn_editor --open <project.lua>"};
        m_dialogOpen = true;
        return;
    }
    if (ProjectFile::load(project, path)) {
        m_selectedScene = 0;
        m_selectedLine = -1;
        m_modified = false;
        m_projectPath = path;
        m_statusMsg = "已打开: " + path;
    } else {
        m_dialogTitle = "错误";
        m_dialogMessages = {"无法打开项目文件:", path};
        m_dialogOpen = true;
    }
}

void EditorUI::menuSave(EditorProject& project) {
    if (m_projectPath.empty()) {
        m_dialogTitle = "保存项目";
        m_dialogMessages = {"请通过命令行指定保存路径:", "vn_editor --open <path> --save"};
        m_dialogOpen = true;
        return;
    }
    if (ProjectFile::save(project, m_projectPath)) {
        m_modified = false;
        m_statusMsg = "已保存: " + m_projectPath;
    } else {
        m_dialogTitle = "错误";
        m_dialogMessages = {"保存失败:", m_projectPath};
        m_dialogOpen = true;
    }
}

void EditorUI::menuExport(EditorProject& project, const std::string& outPath) {
    std::string path = outPath;
    if (path.empty()) {
        if (m_projectPath.empty()) {
            m_dialogTitle = "导出脚本";
            m_dialogMessages = {"请先保存项目或指定导出路径:", "vn_editor --export <output.lua>"};
            m_dialogOpen = true;
            return;
        }
        // 默认导出到同目录
        size_t pos = m_projectPath.find_last_of('/');
        path = (pos != std::string::npos) ? m_projectPath.substr(0, pos + 1) : "";
        path += "exported_game.lua";
    }

    if (LuaExporter::exportSingleFile(project, path)) {
        m_statusMsg = "已导出: " + path;
        m_dialogTitle = "导出成功";
        m_dialogMessages = {"游戏脚本已导出到:", path, "可用 vn_engine --script " + path + " 运行"};
        m_dialogOpen = true;
    } else {
        m_dialogTitle = "错误";
        m_dialogMessages = {"导出失败:", path};
        m_dialogOpen = true;
    }
}

void EditorUI::menuAddScene(EditorProject& project) {
    EditorScene s;
    s.id = "scene" + std::to_string(project.scenes.size() + 1);
    s.name = "场景" + std::to_string(project.scenes.size() + 1);
    project.scenes.push_back(s);
    m_selectedScene = (int)project.scenes.size() - 1;
    m_selectedLine = -1;
    m_modified = true;
    m_statusMsg = "已添加场景: " + s.id;
}

void EditorUI::menuDeleteScene(EditorProject& project) {
    if (m_selectedScene < 0 || m_selectedScene >= (int)project.scenes.size()) return;
    if (project.scenes.size() <= 1) {
        m_statusMsg = "至少保留一个场景";
        return;
    }
    std::string id = project.scenes[m_selectedScene].id;
    project.scenes.erase(project.scenes.begin() + m_selectedScene);
    m_selectedScene = std::max(0, m_selectedScene - 1);
    m_selectedLine = -1;
    m_modified = true;
    m_statusMsg = "已删除场景: " + id;
}

void EditorUI::menuAddLine(EditorProject& project) {
    if (m_selectedScene < 0 || m_selectedScene >= (int)project.scenes.size()) return;
    auto& scene = project.scenes[m_selectedScene];
    EditorLine line;
    line.type = LineType::Narrate;
    line.text = "新行";
    if (m_selectedLine < 0) {
        scene.lines.push_back(line);
        m_selectedLine = (int)scene.lines.size() - 1;
    } else {
        scene.lines.insert(scene.lines.begin() + m_selectedLine + 1, line);
        m_selectedLine++;
    }
    m_modified = true;
    m_statusMsg = "已添加行";
}

void EditorUI::menuDeleteLine(EditorProject& project) {
    if (m_selectedScene < 0 || m_selectedScene >= (int)project.scenes.size()) return;
    if (m_selectedLine < 0 || m_selectedLine >= (int)project.scenes[m_selectedScene].lines.size()) return;
    auto& scene = project.scenes[m_selectedScene];
    scene.lines.erase(scene.lines.begin() + m_selectedLine);
    if (m_selectedLine >= (int)scene.lines.size()) m_selectedLine = (int)scene.lines.size() - 1;
    m_modified = true;
    m_statusMsg = "已删除行";
}

void EditorUI::menuMoveLineUp(EditorProject& project) {
    if (m_selectedScene < 0 || m_selectedScene >= (int)project.scenes.size()) return;
    if (m_selectedLine <= 0) return;
    auto& lines = project.scenes[m_selectedScene].lines;
    std::swap(lines[m_selectedLine], lines[m_selectedLine - 1]);
    m_selectedLine--;
    m_modified = true;
}

void EditorUI::menuMoveLineDown(EditorProject& project) {
    if (m_selectedScene < 0 || m_selectedScene >= (int)project.scenes.size()) return;
    auto& lines = project.scenes[m_selectedScene].lines;
    if (m_selectedLine < 0 || m_selectedLine >= (int)lines.size() - 1) return;
    std::swap(lines[m_selectedLine], lines[m_selectedLine + 1]);
    m_selectedLine++;
    m_modified = true;
}

void EditorUI::menuAddCharacter(EditorProject& project) {
    EditorCharacter ch;
    ch.id = "char" + std::to_string(project.characters.size() + 1);
    ch.name = "角色" + std::to_string(project.characters.size() + 1);
    ch.colorR = 255; ch.colorG = 255; ch.colorB = 255;
    project.characters.push_back(ch);
    m_modified = true;
    m_statusMsg = "已添加角色: " + ch.id;
}

void EditorUI::menuDeleteCharacter(EditorProject& project) {
    if (m_selectedCharacter < 0 || m_selectedCharacter >= (int)project.characters.size()) return;
    std::string id = project.characters[m_selectedCharacter].id;
    project.characters.erase(project.characters.begin() + m_selectedCharacter);
    if (m_selectedCharacter >= (int)project.characters.size())
        m_selectedCharacter = (int)project.characters.size() - 1;
    m_modified = true;
    m_statusMsg = "已删除角色: " + id;
}

// ============================================================================
// 渲染 — 模式对话框（添加场景/行/角色、新建项目、导出、确认删除）
// ============================================================================

void EditorUI::renderModeDialog(EditorProject& project) {
    if (m_mode == EditorMode::Normal) return;

    // 半透明遮罩
    SDL_SetRenderDrawColor(m_ctx.renderer, 0, 0, 0, 160);
    SDL_Rect overlay = {0, 0, m_ctx.width, m_ctx.height};
    SDL_RenderFillRect(m_ctx.renderer, &overlay);

    int dw = 420, dh = 200;
    int dx = (m_ctx.width - dw) / 2;
    int dy = (m_ctx.height - dh) / 2;

    switch (m_mode) {
    case EditorMode::AddScene: {
        UI::panel(m_ctx, dx, dy, dw, dh, "添加场景");
        int cy = dy + 40;

        UI::textSmall(m_ctx, "场景ID:", dx + 20, cy + 4, m_ctx.colTextDim);
        if (UI::textInput(m_ctx, m_dlgInput1, dx + 90, cy, 200, 22, "scene_id")) {}
        cy += 30;

        UI::textSmall(m_ctx, "场景名:", dx + 20, cy + 4, m_ctx.colTextDim);
        if (UI::textInput(m_ctx, m_dlgInput2, dx + 90, cy, 200, 22, "显示名")) {}
        cy += 30;

        UI::textSmall(m_ctx, "背景:", dx + 20, cy + 4, m_ctx.colTextDim);
        if (UI::textInput(m_ctx, m_dlgInput3, dx + 90, cy, 200, 22, "bg_id (可选)")) {}
        cy += 30;

        UI::textSmall(m_ctx, "BGM:", dx + 20, cy + 4, m_ctx.colTextDim);
        if (UI::textInput(m_ctx, m_dlgInput4, dx + 90, cy, 200, 22, "bgm_id (可选)")) {}
        cy += 30;

        int by = dy + dh - 36;
        if (UI::button(m_ctx, "确定", dx + dw - 180, by, 70, 24)) {
            if (!m_dlgInput1.empty()) {
                EditorScene s;
                s.id = m_dlgInput1;
                s.name = m_dlgInput2.empty() ? m_dlgInput1 : m_dlgInput2;
                s.background = m_dlgInput3;
                s.bgm = m_dlgInput4;
                project.scenes.push_back(s);
                m_selectedScene = (int)project.scenes.size() - 1;
                m_selectedLine = -1;
                m_modified = true;
                m_statusMsg = "已添加场景: " + s.id;
            }
            m_mode = EditorMode::Normal;
        }
        if (UI::button(m_ctx, "取消", dx + dw - 90, by, 70, 24)) {
            m_mode = EditorMode::Normal;
        }
        break;
    }

    case EditorMode::AddLine: {
        UI::panel(m_ctx, dx, dy, dw, 160, "添加对话行");
        int cy = dy + 40;

        UI::textSmall(m_ctx, "行类型:", dx + 20, cy + 4, m_ctx.colTextDim);
        std::vector<std::string> types = {"narrate", "say", "choice", "goto", "wait",
            "bgm", "bgm_stop", "sfx", "bg", "fadein", "fadeout", "ending", "label"};
        if (UI::dropdown(m_ctx, types, m_dlgTypeSel, dx + 90, cy, 150, 22)) {}
        cy += 35;

        UI::text(m_ctx, "将在当前场景中添加一行: " + types[m_dlgTypeSel], dx + 20, cy, m_ctx.colTextDim);
        cy += 30;

        int by = dy + 160 - 36;
        if (UI::button(m_ctx, "确定", dx + dw - 180, by, 70, 24)) {
            if (m_selectedScene >= 0 && m_selectedScene < (int)project.scenes.size()) {
                auto& scene = project.scenes[m_selectedScene];
                EditorLine line;
                line.type = EditorProject::lineTypeFromName(types[m_dlgTypeSel]);
                switch (line.type) {
                    case LineType::Say:
                        line.text = "新对话";
                        line.character = "narrator";
                        break;
                    case LineType::Narrate:
                        line.text = "新旁白";
                        break;
                    case LineType::Choice:
                        line.choices.push_back({"选项1", ""});
                        line.choices.push_back({"选项2", ""});
                        break;
                    case LineType::Goto:
                        line.targetScene = "scene1";
                        break;
                    case LineType::Wait:
                        line.waitTime = 1.0f;
                        break;
                    case LineType::BGM:
                    case LineType::SFX:
                    case LineType::BG:
                        line.resourceId = "resource_id";
                        break;
                    case LineType::FadeIn:
                    case LineType::FadeOut:
                        line.fadeTime = 0.5f;
                        break;
                    case LineType::Ending:
                        line.endingId = "ending1";
                        line.endingTitle = "结局";
                        line.endingDesc = "描述";
                        break;
                    case LineType::Label:
                        line.label = "label1";
                        break;
                    default:
                        line.text = "新行";
                        break;
                }
                if (m_selectedLine < 0) {
                    scene.lines.push_back(line);
                    m_selectedLine = (int)scene.lines.size() - 1;
                } else {
                    scene.lines.insert(scene.lines.begin() + m_selectedLine + 1, line);
                    m_selectedLine++;
                }
                m_modified = true;
                m_statusMsg = "已添加行: " + types[m_dlgTypeSel];
            }
            m_mode = EditorMode::Normal;
        }
        if (UI::button(m_ctx, "取消", dx + dw - 90, by, 70, 24)) {
            m_mode = EditorMode::Normal;
        }
        break;
    }

    case EditorMode::NewProject: {
        UI::panel(m_ctx, dx, dy, dw, 160, "新建项目");
        int cy = dy + 40;

        UI::textSmall(m_ctx, "项目名:", dx + 20, cy + 4, m_ctx.colTextDim);
        if (UI::textInput(m_ctx, m_dlgInput1, dx + 90, cy, 200, 22, "项目名")) {}
        cy += 30;

        UI::textSmall(m_ctx, "作者:", dx + 20, cy + 4, m_ctx.colTextDim);
        if (UI::textInput(m_ctx, m_dlgInput2, dx + 90, cy, 200, 22, "作者名")) {}
        cy += 35;

        int by = dy + 160 - 36;
        if (UI::button(m_ctx, "确定", dx + dw - 180, by, 70, 24)) {
            project = EditorProject();
            project.name = m_dlgInput1.empty() ? "新项目" : m_dlgInput1;
            project.author = m_dlgInput2.empty() ? "未知" : m_dlgInput2;
            project.version = "1.0";
            project.startScene = "scene1";

            EditorScene s;
            s.id = "scene1";
            s.name = "场景1";
            project.scenes.push_back(s);

            m_selectedScene = 0;
            m_selectedLine = -1;
            m_modified = true;
            m_statusMsg = "已创建新项目: " + project.name;
            m_mode = EditorMode::Normal;
        }
        if (UI::button(m_ctx, "取消", dx + dw - 90, by, 70, 24)) {
            m_mode = EditorMode::Normal;
        }
        break;
    }

    case EditorMode::ExportDialog: {
        UI::panel(m_ctx, dx, dy, dw, 160, "导出 Lua 脚本");
        int cy = dy + 40;

        UI::textSmall(m_ctx, "导出路径:", dx + 20, cy + 4, m_ctx.colTextDim);
        if (UI::textInput(m_ctx, m_exportDir, dx + 90, cy, 250, 22, "output.lua")) {}
        cy += 35;

        UI::text(m_ctx, "将项目导出为可运行的 Lua 游戏脚本", dx + 20, cy, m_ctx.colTextDim);
        cy += 25;

        int by = dy + 160 - 36;
        if (UI::button(m_ctx, "导出", dx + dw - 180, by, 70, 24)) {
            if (!m_exportDir.empty()) {
                menuExport(project, m_exportDir);
            }
            m_mode = EditorMode::Normal;
        }
        if (UI::button(m_ctx, "取消", dx + dw - 90, by, 70, 24)) {
            m_mode = EditorMode::Normal;
        }
        break;
    }

    case EditorMode::AddCharacter: {
        UI::panel(m_ctx, dx, dy, dw, 160, "添加角色");
        int cy = dy + 40;

        UI::textSmall(m_ctx, "角色ID:", dx + 20, cy + 4, m_ctx.colTextDim);
        if (UI::textInput(m_ctx, m_dlgInput1, dx + 90, cy, 200, 22, "char_id")) {}
        cy += 30;

        UI::textSmall(m_ctx, "显示名:", dx + 20, cy + 4, m_ctx.colTextDim);
        if (UI::textInput(m_ctx, m_dlgInput2, dx + 90, cy, 200, 22, "显示名")) {}
        cy += 35;

        int by = dy + 160 - 36;
        if (UI::button(m_ctx, "确定", dx + dw - 180, by, 70, 24)) {
            if (!m_dlgInput1.empty()) {
                EditorCharacter ch;
                ch.id = m_dlgInput1;
                ch.name = m_dlgInput2.empty() ? m_dlgInput1 : m_dlgInput2;
                ch.colorR = 255; ch.colorG = 255; ch.colorB = 255;
                project.characters.push_back(ch);
                m_selectedCharacter = (int)project.characters.size() - 1;
                m_modified = true;
                m_statusMsg = "已添加角色: " + ch.id;
            }
            m_mode = EditorMode::Normal;
        }
        if (UI::button(m_ctx, "取消", dx + dw - 90, by, 70, 24)) {
            m_mode = EditorMode::Normal;
        }
        break;
    }

    case EditorMode::ConfirmDelete: {
        UI::panel(m_ctx, dx, dy, dw, 140, "确认删除");
        int cy = dy + 44;

        UI::text(m_ctx, m_dlgMessage, dx + 20, cy, m_ctx.colText);
        cy += 30;
        UI::text(m_ctx, "此操作不可撤销", dx + 20, cy, m_ctx.colTextDim);

        int by = dy + 140 - 36;
        if (UI::button(m_ctx, "删除", dx + dw - 180, by, 70, 24)) {
            // 判断是删除场景还是角色
            if (m_dlgMessage.find("场景") != std::string::npos) {
                menuDeleteScene(project);
            } else if (m_dlgMessage.find("角色") != std::string::npos) {
                menuDeleteCharacter(project);
            }
            m_mode = EditorMode::Normal;
        }
        if (UI::button(m_ctx, "取消", dx + dw - 90, by, 70, 24)) {
            m_mode = EditorMode::Normal;
        }
        break;
    }

    default:
        break;
    }
}

// ============================================================================
// 主渲染
// ============================================================================

void EditorUI::render(EditorProject& project) {
    // 清屏
    SDL_SetRenderDrawColor(m_ctx.renderer,
        m_ctx.colBg.r,
        m_ctx.colBg.g,
        m_ctx.colBg.b,
        255);
    SDL_RenderClear(m_ctx.renderer);

    // 渲染各面板
    renderMenuBar(project);
    renderSceneList(project);
    renderDialogueEditor(project);
    renderPropertiesPanel(project);
    renderStatusBar();

    // 对话框（最上层）
    renderDialog();
    renderModeDialog(project);

    // 重置鼠标点击状态
    m_ctx.mouseClicked = false;
}

// ============================================================================
// 事件处理
// ============================================================================

bool EditorUI::handleEvents(EditorProject& project) {
    m_ctx.mouseClicked = false;
    m_ctx.mousePressed = false;
    m_ctx.mouseReleased = false;

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_QUIT:
            return false;

        case SDL_KEYDOWN:
            if (m_ctx.textInputActive) {
                // 文本输入模式下的键盘处理
                switch (event.key.keysym.sym) {
                case SDLK_BACKSPACE:
                    if (m_ctx.textCursor > 0) {
                        m_ctx.textInput.erase(m_ctx.textCursor - 1, 1);
                        m_ctx.textCursor--;
                    }
                    break;
                case SDLK_DELETE:
                    if (m_ctx.textCursor < (int)m_ctx.textInput.size()) {
                        m_ctx.textInput.erase(m_ctx.textCursor, 1);
                    }
                    break;
                case SDLK_LEFT:
                    if (m_ctx.textCursor > 0) m_ctx.textCursor--;
                    break;
                case SDLK_RIGHT:
                    if (m_ctx.textCursor < (int)m_ctx.textInput.size()) m_ctx.textCursor++;
                    break;
                case SDLK_HOME:
                    m_ctx.textCursor = 0;
                    break;
                case SDLK_END:
                    m_ctx.textCursor = (int)m_ctx.textInput.size();
                    break;
                case SDLK_RETURN:
                case SDLK_KP_ENTER:
                    m_ctx.textInputActive = false;
                    break;
                case SDLK_ESCAPE:
                    m_ctx.textInputActive = false;
                    break;
                }
            } else {
                // 全局快捷键
                switch (event.key.keysym.sym) {
                case SDLK_ESCAPE:
                    if (m_dialogOpen) { m_dialogOpen = false; break; }
                    if (m_mode != EditorMode::Normal) { m_mode = EditorMode::Normal; break; }
                    return false;
                case SDLK_n:
                    if (SDL_GetModState() & KMOD_CTRL) menuNew(project);
                    break;
                case SDLK_s:
                    if (SDL_GetModState() & KMOD_CTRL) menuSave(project);
                    break;
                case SDLK_UP:
                    if (m_selectedLine > 0) m_selectedLine--;
                    break;
                case SDLK_DOWN:
                    if (m_selectedScene >= 0 && m_selectedScene < (int)project.scenes.size()) {
                        auto& lines = project.scenes[m_selectedScene].lines;
                        if (m_selectedLine < (int)lines.size() - 1) m_selectedLine++;
                    }
                    break;
                case SDLK_DELETE:
                    if (m_mode == EditorMode::Normal)
                        menuDeleteLine(project);
                    break;
                }
            }
            break;

        case SDL_MOUSEMOTION:
            m_ctx.mouseX = event.motion.x;
            m_ctx.mouseY = event.motion.y;
            break;

        case SDL_MOUSEBUTTONDOWN:
            if (event.button.button == SDL_BUTTON_LEFT) {
                m_ctx.mousePressed = true;
                m_ctx.mouseClicked = true;
                m_ctx.mouseX = event.button.x;
                m_ctx.mouseY = event.button.y;
            }
            break;

        case SDL_MOUSEBUTTONUP:
            if (event.button.button == SDL_BUTTON_LEFT) {
                m_ctx.mouseReleased = true;
            }
            break;

        case SDL_TEXTINPUT:
            if (m_ctx.textInputActive) {
                m_ctx.textInput.insert(m_ctx.textCursor, event.text.text);
                m_ctx.textCursor += strlen(event.text.text);
            }
            break;

        case SDL_MOUSEWHEEL:
            m_ctx.mouseWheel = event.wheel.y;
            break;
        }
    }
    return true;
}

// ============================================================================
// 主循环
// ============================================================================

void EditorUI::run(EditorProject& project, const std::string& exportPath) {
    m_running = true;

    while (m_running) {
        if (!handleEvents(project)) {
            m_running = false;
            break;
        }

        // 处理菜单动作
        int action = m_menuAction;
        m_menuAction = -1;

        switch (action) {
        case 0: menuNew(project); break;
        case 1: menuOpen(project, ""); break;
        case 2: menuSave(project); break;
        case 3: menuExport(project, exportPath); break;
        case 4: menuAddScene(project); break;
        case 5: menuDeleteScene(project); break;
        case 6: menuAddLine(project); break;
        case 7: menuDeleteLine(project); break;
        case 8: menuMoveLineUp(project); break;
        case 9: menuMoveLineDown(project); break;
        case 10: menuAddCharacter(project); break;
        case 11: // 退出
            m_running = false;
            break;
        }

        render(project);
        SDL_RenderPresent(m_ctx.renderer);
        SDL_Delay(16); // ~60fps
    }
}
