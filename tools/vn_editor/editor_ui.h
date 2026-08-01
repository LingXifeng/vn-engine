#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <string>
#include <vector>

#include "editor_project.h"

// ============================================================================
// 简易即时模式 GUI（用于 VN 编辑器）
// ============================================================================

struct UICtx {
    SDL_Renderer* renderer = nullptr;
    TTF_Font* font = nullptr;
    TTF_Font* fontSmall = nullptr;
    int mouseX = 0, mouseY = 0;
    bool mousePressed = false;
    bool mouseReleased = false;
    bool mouseClicked = false;
    int clickX = 0, clickY = 0;
    int width = 0, height = 0;

    // 文本输入
    std::string textInput;
    bool textInputActive = false;
    int textCursor = 0;
    int mouseWheel = 0;
    int scrollY = 0;

    // 颜色
    SDL_Color colBg     = {30, 30, 40, 255};
    SDL_Color colPanel  = {45, 45, 55, 255};
    SDL_Color colBorder = {70, 70, 85, 255};
    SDL_Color colText   = {220, 220, 230, 255};
    SDL_Color colTextDim= {140, 140, 155, 255};
    SDL_Color colAccent = {100, 160, 255, 255};
    SDL_Color colHover  = {60, 60, 80, 255};
    SDL_Color colSelect = {80, 120, 200, 255};
    SDL_Color colBtn    = {55, 55, 70, 255};
    SDL_Color colBtnHover = {75, 75, 95, 255};
};

namespace UI {
    void fillRect(UICtx& ctx, int x, int y, int w, int h, SDL_Color color);
    void drawRect(UICtx& ctx, int x, int y, int w, int h, SDL_Color color);
    void text(UICtx& ctx, const std::string& s, int x, int y, SDL_Color color);
    void textSmall(UICtx& ctx, const std::string& s, int x, int y, SDL_Color color);
    int textWidth(UICtx& ctx, const std::string& s);
    int textHeight(UICtx& ctx);
    bool button(UICtx& ctx, const std::string& label, int x, int y, int w, int h);
    bool listItem(UICtx& ctx, const std::string& label, int x, int y, int w, int h, bool selected);
    bool mouseInRect(UICtx& ctx, int x, int y, int w, int h);
    void panel(UICtx& ctx, int x, int y, int w, int h, const std::string& title = "");
    bool textInput(UICtx& ctx, std::string& value, int x, int y, int w, int h, const std::string& hint = "");
    bool dropdown(UICtx& ctx, const std::vector<std::string>& options, int& selected, int x, int y, int w, int h);
}

// ============================================================================
// 编辑器模式
// ============================================================================

enum class EditorMode {
    Normal,
    NewProject,
    ExportDialog,
    AddScene,
    AddCharacter,
    AddLine,
    ConfirmDelete,
    EditCharacter
};

// ============================================================================
// 编辑器主 UI
// ============================================================================

class EditorUI {
public:
    EditorUI();
    ~EditorUI();

    bool init(int width = 1280, int height = 800);
    void shutdown();

    // 主循环
    void run(EditorProject& project, const std::string& exportPath = "");

private:
    SDL_Window* m_window = nullptr;
    SDL_Renderer* m_renderer = nullptr;
    TTF_Font* m_font = nullptr;
    TTF_Font* m_fontSmall = nullptr;
    UICtx m_ctx;

    bool m_running = false;
    EditorMode m_mode = EditorMode::Normal;

    // 选择状态
    int m_selectedScene = 0;
    int m_selectedLine = -1;
    int m_selectedCharacter = -1;  // 选中的角色索引

    // 项目文件路径
    std::string m_projectPath;
    bool m_modified = false;

    // 菜单动作
    int m_menuAction = -1;

    // 对话框
    bool m_dialogOpen = false;
    std::string m_dialogTitle;
    std::vector<std::string> m_dialogMessages;

    // 对话框临时数据
    std::string m_dlgInput1;
    std::string m_dlgInput2;
    std::string m_dlgInput3;
    std::string m_dlgInput4;
    std::string m_dlgMessage;
    std::string m_exportDir;
    int m_dlgTypeSel = 1;  // 添加行时的类型选择

    // 状态栏
    std::string m_statusMsg;

    // 布局
    int m_menuBarH = 30;
    int m_leftPanelW = 220;
    int m_rightPanelW = 280;
    int m_statusBarH = 25;

    // 事件处理
    void handleEvent(SDL_Event& event, EditorProject& project);
    bool handleEvents(EditorProject& project);

    // 渲染
    void renderMenuBar(EditorProject& project);
    void renderSceneList(EditorProject& project);
    void renderDialogueEditor(EditorProject& project);
    void renderPropertiesPanel(EditorProject& project);
    void renderStatusBar();
    void renderDialog();
    void renderModeDialog(EditorProject& project);
    void render(EditorProject& project);

    // 菜单动作
    void menuNew(EditorProject& project);
    void menuOpen(EditorProject& project, const std::string& path);
    void menuSave(EditorProject& project);
    void menuExport(EditorProject& project, const std::string& outPath);
    void menuAddScene(EditorProject& project);
    void menuDeleteScene(EditorProject& project);
    void menuAddLine(EditorProject& project);
    void menuDeleteLine(EditorProject& project);
    void menuMoveLineUp(EditorProject& project);
    void menuMoveLineDown(EditorProject& project);
    void menuAddCharacter(EditorProject& project);
    void menuDeleteCharacter(EditorProject& project);
};
