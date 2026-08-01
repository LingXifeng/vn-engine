#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <string>
#include <vector>
#include <unordered_set>
#include <memory>
#include "renderer.h"
#include "input.h"
#include "resource_manager.h"

// CG 画廊条目
struct CGEntry {
    std::string id;            // 唯一标识
    std::string thumbPath;     // 缩略图路径
    std::string fullPath;      // 完整图路径
    std::string caption;       // 说明文字
    bool unlocked = false;     // 是否解锁
    SDL_Rect gridRect;         // 网格中的位置
};

// 画廊模式
enum class GalleryMode {
    GRID,       // 网格浏览
    VIEWER,     // 大图查看
    STAND       // 立绘浏览
};

// 立绘条目
struct StandEntry {
    std::string name;
    std::string path;
    std::vector<std::string> expressions;  // 表情列表
    SDL_Rect gridRect;
};

// CG 画廊
class CGGallery {
public:
    CGGallery(Renderer* renderer, ResourceManager* resMgr);
    ~CGGallery();

    void setFont(TTF_Font* font) { m_font = font; }

    // CG 管理
    void addCG(const std::string& id, const std::string& thumbPath,
               const std::string& fullPath, const std::string& caption);
    void unlockCG(const std::string& id);
    bool isCGUnlocked(const std::string& id) const;
    int getUnlockedCount() const;

    // 立绘管理
    void addStand(const std::string& name, const std::string& path,
                  const std::vector<std::string>& expressions);

    // 存档支持
    std::vector<std::string> getUnlockedIDs() const;
    void setUnlockedIDs(const std::vector<std::string>& ids);

    // 控制
    void show();
    void hide();
    bool isVisible() const { return m_visible; }

    // 更新与渲染
    void update(float dt, const Input& input);
    void render();

private:
    Renderer* m_renderer;
    ResourceManager* m_resMgr;
    TTF_Font* m_font = nullptr;

    bool m_visible = false;
    GalleryMode m_mode = GalleryMode::GRID;

    // CG 列表
    std::vector<CGEntry> m_cgs;
    int m_cgHoverIndex = -1;
    int m_cgViewIndex = -1;   // 大图查看的索引

    // 立绘列表
    std::vector<StandEntry> m_stands;
    int m_standHoverIndex = -1;
    int m_standViewIndex = -1;
    int m_standExprIndex = 0;

    // 网格布局
    int m_gridCols = 4;
    int m_gridRows = 3;
    int m_thumbW = 200;
    int m_thumbH = 150;
    int m_gridStartX = 0;
    int m_gridStartY = 0;
    int m_gridPage = 0;

    // 大图查看
    float m_viewScale = 1.0f;
    float m_viewX = 0, m_viewY = 0;
    bool m_dragging = false;
    int m_dragStartX = 0, m_dragStartY = 0;

    float m_fadeAlpha = 0.0f;

    void updateGridLayout();
    void renderGrid();
    void renderViewer();
    void renderStand();
    int getGridIndexAt(int x, int y) const;
    int getTotalPages() const;
};
