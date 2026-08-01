#include "cg_gallery.h"
#include <algorithm>
#include <cmath>

CGGallery::CGGallery(Renderer* renderer, ResourceManager* resMgr)
    : m_renderer(renderer), m_resMgr(resMgr) {}

CGGallery::~CGGallery() {}

void CGGallery::addCG(const std::string& id, const std::string& thumbPath,
                      const std::string& fullPath, const std::string& caption) {
    CGEntry entry;
    entry.id = id;
    entry.thumbPath = thumbPath;
    entry.fullPath = fullPath;
    entry.caption = caption;
    entry.unlocked = false;
    m_cgs.push_back(entry);
}

void CGGallery::unlockCG(const std::string& id) {
    for (auto& cg : m_cgs) {
        if (cg.id == id) { cg.unlocked = true; return; }
    }
}

bool CGGallery::isCGUnlocked(const std::string& id) const {
    for (const auto& cg : m_cgs) {
        if (cg.id == id) return cg.unlocked;
    }
    return false;
}

int CGGallery::getUnlockedCount() const {
    int count = 0;
    for (const auto& cg : m_cgs) if (cg.unlocked) count++;
    return count;
}

void CGGallery::addStand(const std::string& name, const std::string& path,
                          const std::vector<std::string>& expressions) {
    StandEntry entry;
    entry.name = name;
    entry.path = path;
    entry.expressions = expressions;
    m_stands.push_back(entry);
}

std::vector<std::string> CGGallery::getUnlockedIDs() const {
    std::vector<std::string> ids;
    for (const auto& cg : m_cgs) if (cg.unlocked) ids.push_back(cg.id);
    return ids;
}

void CGGallery::setUnlockedIDs(const std::vector<std::string>& ids) {
    for (const auto& id : ids) unlockCG(id);
}

void CGGallery::show() {
    m_visible = true;
    m_mode = GalleryMode::GRID;
    m_fadeAlpha = 0.0f;
    m_gridPage = 0;
    m_cgHoverIndex = -1;
    m_cgViewIndex = -1;
    updateGridLayout();
}

void CGGallery::hide() {
    m_visible = false;
    m_mode = GalleryMode::GRID;
}

void CGGallery::updateGridLayout() {
    int sw = m_renderer->getWidth();
    int sh = m_renderer->getHeight();
    int totalW = m_gridCols * m_thumbW + (m_gridCols - 1) * 10;
    int totalH = m_gridRows * m_thumbH + (m_gridRows - 1) * 10;
    m_gridStartX = (sw - totalW) / 2;
    m_gridStartY = (sh - totalH) / 2 + 20;

    int pageStart = m_gridPage * m_gridCols * m_gridRows;
    for (size_t i = 0; i < m_cgs.size(); i++) {
        int localIdx = (int)i - pageStart;
        if (localIdx < 0 || localIdx >= m_gridCols * m_gridRows) continue;
        int col = localIdx % m_gridCols;
        int row = localIdx / m_gridCols;
        m_cgs[i].gridRect.x = m_gridStartX + col * (m_thumbW + 10);
        m_cgs[i].gridRect.y = m_gridStartY + row * (m_thumbH + 10);
        m_cgs[i].gridRect.w = m_thumbW;
        m_cgs[i].gridRect.h = m_thumbH;
    }
}

int CGGallery::getGridIndexAt(int x, int y) const {
    int pageStart = m_gridPage * m_gridCols * m_gridRows;
    int pageEnd = pageStart + m_gridCols * m_gridRows;
    for (int i = pageStart; i < (int)m_cgs.size() && i < pageEnd; i++) {
        const auto& r = m_cgs[i].gridRect;
        if (x >= r.x && x < r.x + r.w && y >= r.y && y < r.y + r.h) return i;
    }
    return -1;
}

int CGGallery::getTotalPages() const {
    int perPage = m_gridCols * m_gridRows;
    return std::max(1, (int)((m_cgs.size() + perPage - 1) / perPage));
}

void CGGallery::update(float dt, const Input& input) {
    if (!m_visible) return;
    m_fadeAlpha = std::min(m_fadeAlpha + 3.0f * dt, 1.0f);

    int mx, my;
    input.getMousePosition(mx, my);

    if (m_mode == GalleryMode::GRID) {
        m_cgHoverIndex = getGridIndexAt(mx, my);

        // 点击查看
        if (input.isMouseButtonPressed(1) && m_cgHoverIndex >= 0) {
            if (m_cgs[m_cgHoverIndex].unlocked) {
                m_mode = GalleryMode::VIEWER;
                m_cgViewIndex = m_cgHoverIndex;
                m_viewScale = 1.0f;
                m_viewX = 0;
                m_viewY = 0;
            }
        }

        // 翻页
        if (input.isKeyPressed(SDL_SCANCODE_RIGHT) && m_gridPage < getTotalPages() - 1) {
            m_gridPage++;
            updateGridLayout();
        }
        if (input.isKeyPressed(SDL_SCANCODE_LEFT) && m_gridPage > 0) {
            m_gridPage--;
            updateGridLayout();
        }
        // 滚轮翻页
        if (input.getMouseWheelY() < 0 && m_gridPage < getTotalPages() - 1) {
            m_gridPage++;
            updateGridLayout();
        }
        if (input.getMouseWheelY() > 0 && m_gridPage > 0) {
            m_gridPage--;
            updateGridLayout();
        }

        // ESC 返回
        if (input.isKeyPressed(SDL_SCANCODE_ESCAPE)) hide();

    } else if (m_mode == GalleryMode::VIEWER) {
        // 拖拽
        if (input.isMouseButtonPressed(1)) {
            m_dragging = true;
            m_dragStartX = mx - (int)m_viewX;
            m_dragStartY = my - (int)m_viewY;
        }
        if (input.isMouseButtonReleased(1)) m_dragging = false;
        if (m_dragging) {
            m_viewX = mx - m_dragStartX;
            m_viewY = my - m_dragStartY;
        }

        // 缩放
        int wheel = input.getMouseWheelY();
        if (wheel != 0) {
            m_viewScale = std::clamp(m_viewScale + wheel * 0.1f, 0.5f, 3.0f);
        }

        // ESC/右键返回网格
        if (input.isKeyPressed(SDL_SCANCODE_ESCAPE) || input.isMouseButtonPressed(3)) {
            m_mode = GalleryMode::GRID;
            m_cgViewIndex = -1;
        }
    }
}

void CGGallery::render() {
    if (!m_visible) return;
    Uint8 alpha = (Uint8)(m_fadeAlpha * 255);

    // 背景
    m_renderer->drawRect(0, 0, m_renderer->getWidth(), m_renderer->getHeight(),
                         {15, 15, 25, alpha}, true);

    if (m_mode == GalleryMode::GRID) renderGrid();
    else if (m_mode == GalleryMode::VIEWER) renderViewer();
}

void CGGallery::renderGrid() {
    Uint8 alpha = (Uint8)(m_fadeAlpha * 255);
    if (!m_font) return;

    // 标题
    auto titleTex = m_renderer->renderText("CG Gallery", m_font, {255, 220, 100, alpha});
    if (titleTex) {
        m_renderer->drawTexture(titleTex.get(),
                                (m_renderer->getWidth() - titleTex->width()) / 2, 20, 1.0f, 1.0f, alpha);
    }

    int pageStart = m_gridPage * m_gridCols * m_gridRows;
    int pageEnd = std::min(pageStart + m_gridCols * m_gridRows, (int)m_cgs.size());

    for (int i = pageStart; i < pageEnd; i++) {
        const auto& cg = m_cgs[i];
        const auto& r = cg.gridRect;

        // 边框
        SDL_Color borderColor;
        if (i == m_cgHoverIndex && cg.unlocked) borderColor = {255, 220, 100, alpha};
        else if (cg.unlocked) borderColor = {100, 150, 200, alpha};
        else borderColor = {50, 50, 60, alpha};

        m_renderer->drawRect(r.x - 2, r.y - 2, r.w + 4, r.h + 4, borderColor, false);

        if (cg.unlocked) {
            // 显示缩略图
            auto tex = m_resMgr->getTexture(cg.thumbPath);
            if (tex) {
                float sx = (float)r.w / tex->width();
                float sy = (float)r.h / tex->height();
                float s = std::min(sx, sy);
                int dw = (int)(tex->width() * s);
                int dh = (int)(tex->height() * s);
                m_renderer->drawTexture(tex.get(), r.x + (r.w - dw) / 2, r.y + (r.h - dh) / 2, s, s, alpha);
            }
        } else {
            // 未解锁：显示 "???"
            auto lockTex = m_renderer->renderText("???", m_font, {80, 80, 80, alpha});
            if (lockTex) {
                m_renderer->drawTexture(lockTex.get(),
                                        r.x + (r.w - lockTex->width()) / 2,
                                        r.y + (r.h - lockTex->height()) / 2, 1.0f, 1.0f, alpha);
            }
        }
    }

    // 页码
    if (getTotalPages() > 1) {
        std::string pageStr = std::to_string(m_gridPage + 1) + " / " + std::to_string(getTotalPages());
        auto pageTex = m_renderer->renderText(pageStr, m_font, {180, 180, 180, alpha});
        if (pageTex) {
            m_renderer->drawTexture(pageTex.get(),
                                    (m_renderer->getWidth() - pageTex->width()) / 2,
                                    m_renderer->getHeight() - 40, 1.0f, 1.0f, alpha);
        }
    }
}

void CGGallery::renderViewer() {
    if (m_cgViewIndex < 0 || m_cgViewIndex >= (int)m_cgs.size()) return;
    Uint8 alpha = (Uint8)(m_fadeAlpha * 255);

    const auto& cg = m_cgs[m_cgViewIndex];
    auto tex = m_resMgr->getTexture(cg.fullPath);
    if (!tex) return;

    int sw = m_renderer->getWidth();
    int sh = m_renderer->getHeight();

    // 居中显示
    float baseScale = std::min((float)sw / tex->width(), (float)sh / tex->height());
    float scale = baseScale * m_viewScale;
    int dw = (int)(tex->width() * scale);
    int dh = (int)(tex->height() * scale);
    int dx = (sw - dw) / 2 + (int)m_viewX;
    int dy = (sh - dh) / 2 + (int)m_viewY;

    m_renderer->drawTexture(tex.get(), dx, dy, scale, scale, alpha);

    // 说明文字
    if (m_font && !cg.caption.empty()) {
        auto capTex = m_renderer->renderText(cg.caption, m_font, {200, 200, 200, alpha});
        if (capTex) {
            m_renderer->drawTexture(capTex.get(),
                                    (sw - capTex->width()) / 2, sh - 40, 1.0f, 1.0f, alpha);
        }
    }
}

void CGGallery::renderStand() {
    // 立绘浏览模式（简化版，复用网格布局）
    renderGrid();
}
