#include "flowchart.h"
#include <algorithm>
#include <cmath>
#include <sstream>

Flowchart::Flowchart(Renderer* renderer)
    : m_renderer(renderer) {
}

Flowchart::~Flowchart() = default;

// === 节点管理 ===

void Flowchart::addNode(const std::string& id, const std::string& label,
                        FlowchartNodeType type, const std::string& scriptLabel,
                        const std::string& description) {
    if (m_nodeIndex.count(id)) return;  // 已存在

    FlowchartNode node;
    node.id = id;
    node.label = label;
    node.type = type;
    node.scriptLabel = scriptLabel;
    node.description = description;

    // 根据类型设置节点大小
    switch (type) {
        case FlowchartNodeType::START:
            node.width = 120;
            node.height = 44;
            break;
        case FlowchartNodeType::ENDING_GOOD:
        case FlowchartNodeType::ENDING_NORMAL:
        case FlowchartNodeType::ENDING_BAD:
        case FlowchartNodeType::ENDING_TRUE:
            node.width = 180;
            node.height = 50;
            break;
        default:
            node.width = 160;
            node.height = 46;
            break;
    }

    m_nodeIndex[id] = static_cast<int>(m_nodes.size());
    m_nodes.push_back(node);
}

void Flowchart::setNodePosition(const std::string& id, float x, float y) {
    auto it = m_nodeIndex.find(id);
    if (it == m_nodeIndex.end()) return;
    m_nodes[it->second].x = x;
    m_nodes[it->second].y = y;
    m_nodes[it->second].manualPos = true;
}

FlowchartNode* Flowchart::getNode(const std::string& id) {
    auto it = m_nodeIndex.find(id);
    if (it == m_nodeIndex.end()) return nullptr;
    return &m_nodes[it->second];
}

bool Flowchart::hasNode(const std::string& id) const {
    return m_nodeIndex.count(id) > 0;
}

// === 连线管理 ===

void Flowchart::addEdge(const std::string& fromId, const std::string& toId,
                        const std::string& label) {
    if (!hasNode(fromId) || !hasNode(toId)) return;

    // 避免重复连线
    for (const auto& e : m_edges) {
        if (e.fromId == fromId && e.toId == toId) return;
    }

    FlowchartEdge edge;
    edge.fromId = fromId;
    edge.toId = toId;
    edge.label = label;
    m_edges.push_back(edge);
}

// === 进度追踪 ===

void Flowchart::markVisited(const std::string& id) {
    auto it = m_nodeIndex.find(id);
    if (it == m_nodeIndex.end()) return;
    m_nodes[it->second].visited = true;
}

void Flowchart::markCurrent(const std::string& id) {
    clearCurrent();
    auto it = m_nodeIndex.find(id);
    if (it == m_nodeIndex.end()) return;
    m_nodes[it->second].current = true;
    m_nodes[it->second].visited = true;  // 当前节点也算已访问
}

void Flowchart::clearCurrent() {
    for (auto& node : m_nodes) {
        node.current = false;
    }
}

bool Flowchart::isVisited(const std::string& id) const {
    auto it = m_nodeIndex.find(id);
    if (it == m_nodeIndex.end()) return false;
    return m_nodes[it->second].visited;
}

std::vector<std::string> Flowchart::getVisitedNodes() const {
    std::vector<std::string> result;
    for (const auto& node : m_nodes) {
        if (node.visited) result.push_back(node.id);
    }
    return result;
}

std::vector<std::string> Flowchart::getReachedEndings() const {
    std::vector<std::string> result;
    for (const auto& node : m_nodes) {
        if (!node.visited) continue;
        if (node.type == FlowchartNodeType::ENDING_GOOD ||
            node.type == FlowchartNodeType::ENDING_NORMAL ||
            node.type == FlowchartNodeType::ENDING_BAD ||
            node.type == FlowchartNodeType::ENDING_TRUE) {
            result.push_back(node.id);
        }
    }
    return result;
}

void Flowchart::setVisitedNodes(const std::vector<std::string>& ids) {
    for (const auto& id : ids) {
        markVisited(id);
    }
}

// === 控制 ===

void Flowchart::show() {
    m_visible = true;
}

void Flowchart::hide() {
    m_visible = false;
}

void Flowchart::resetView() {
    m_zoom = 1.0f;
    m_panX = 0.0f;
    m_panY = 0.0f;
}

// === 坐标变换 ===

void Flowchart::worldToScreen(float wx, float wy, float& sx, float& sy) const {
    int screenW = m_renderer->getWidth();
    int screenH = m_renderer->getHeight();
    sx = (wx - m_panX) * m_zoom + screenW / 2.0f;
    sy = (wy - m_panY) * m_zoom + screenH / 2.0f;
}

void Flowchart::screenToWorld(float sx, float sy, float& wx, float& wy) const {
    int screenW = m_renderer->getWidth();
    int screenH = m_renderer->getHeight();
    wx = (sx - screenW / 2.0f) / m_zoom + m_panX;
    wy = (sy - screenH / 2.0f) / m_zoom + m_panY;
}

// === 颜色 ===

SDL_Color Flowchart::getNodeColor(FlowchartNodeType type, bool visited, bool current) const {
    if (current) {
        return {255, 200, 60, 255};  // 金色高亮
    }

    if (!visited) {
        return {50, 50, 60, 255};    // 未访问：暗灰
    }

    switch (type) {
        case FlowchartNodeType::START:
            return {60, 180, 100, 255};   // 绿色
        case FlowchartNodeType::ENDING_GOOD:
            return {80, 200, 120, 255};   // 亮绿
        case FlowchartNodeType::ENDING_TRUE:
            return {200, 180, 60, 255};   // 金黄
        case FlowchartNodeType::ENDING_NORMAL:
            return {100, 150, 220, 255};  // 蓝色
        case FlowchartNodeType::ENDING_BAD:
            return {200, 80, 80, 255};    // 红色
        case FlowchartNodeType::BRANCH:
            return {180, 140, 220, 255};  // 紫色
        default:
            return {70, 110, 160, 255};   // 蓝灰
    }
}

SDL_Color Flowchart::getNodeBorderColor(FlowchartNodeType type, bool current) const {
    if (current) return {255, 230, 100, 255};

    switch (type) {
        case FlowchartNodeType::ENDING_GOOD:
            return {120, 255, 160, 255};
        case FlowchartNodeType::ENDING_BAD:
            return {255, 120, 120, 255};
        case FlowchartNodeType::ENDING_TRUE:
            return {255, 230, 120, 255};
        default:
            return {180, 200, 230, 255};
    }
}

std::string Flowchart::nodeTypeLabel(FlowchartNodeType type) const {
    switch (type) {
        case FlowchartNodeType::START:         return "START";
        case FlowchartNodeType::CHAPTER:       return "CH";
        case FlowchartNodeType::BRANCH:        return "BRANCH";
        case FlowchartNodeType::SCENE:         return "";
        case FlowchartNodeType::ENDING_GOOD:   return "Good End";
        case FlowchartNodeType::ENDING_NORMAL: return "End";
        case FlowchartNodeType::ENDING_BAD:    return "Bad End";
        case FlowchartNodeType::ENDING_TRUE:   return "True End";
        default: return "";
    }
}

// === 自动布局 ===

std::vector<std::string> Flowchart::getChildren(const std::string& nodeId) const {
    std::vector<std::string> children;
    for (const auto& edge : m_edges) {
        if (edge.fromId == nodeId) {
            children.push_back(edge.toId);
        }
    }
    return children;
}

int Flowchart::getSubtreeWidth(const std::string& nodeId,
                               std::unordered_set<std::string>& visited) const {
    if (visited.count(nodeId)) return 1;
    visited.insert(nodeId);

    auto children = getChildren(nodeId);
    if (children.empty()) return 1;

    int total = 0;
    for (const auto& child : children) {
        total += getSubtreeWidth(child, visited);
    }
    return std::max(1, total);
}

void Flowchart::layoutSubtree(const std::string& nodeId, float x, float y,
                              std::unordered_set<std::string>& placed, int depth) {
    if (placed.count(nodeId)) return;
    placed.insert(nodeId);

    auto* node = getNode(nodeId);
    if (!node) return;

    node->x = x;
    node->y = y;

    auto children = getChildren(nodeId);
    if (children.empty()) return;

    // 计算每个子树的宽度
    int totalWidth = 0;
    std::vector<std::pair<std::string, int>> childWidths;
    for (const auto& child : children) {
        std::unordered_set<std::string> vis;
        int w = getSubtreeWidth(child, vis);
        childWidths.push_back({child, w});
        totalWidth += w;
    }

    // 从左到右排列子节点
    float childY = y + m_nodeSpacingY;
    float startX = x - (totalWidth - 1) * m_nodeSpacingX / 2.0f;

    float curX = startX;
    for (const auto& [child, w] : childWidths) {
        float childCenter = curX + (w - 1) * m_nodeSpacingX / 2.0f;
        layoutSubtree(child, childCenter, childY, placed, depth + 1);
        curX += w * m_nodeSpacingX;
    }
}

void Flowchart::layoutFromNode(const std::string& rootId, float startX, float startY) {
    std::unordered_set<std::string> placed;
    layoutSubtree(rootId, startX, startY, placed, 0);
}

void Flowchart::autoLayout() {
    // 找到 START 节点作为根
    std::string rootId;
    for (const auto& node : m_nodes) {
        if (node.type == FlowchartNodeType::START) {
            rootId = node.id;
            break;
        }
    }

    if (rootId.empty() && !m_nodes.empty()) {
        rootId = m_nodes[0].id;
    }

    if (!rootId.empty()) {
        layoutFromNode(rootId, 0.0f, 0.0f);
    }

    // 对手动指定位置的节点保持不变（layoutSubtree 会覆盖，所以这里仅对未布局的节点处理）
    // 简单处理：未在树中的节点放在右侧
    std::unordered_set<std::string> inTree;
    if (!rootId.empty()) {
        std::unordered_set<std::string> placed;
        // 重新收集已放置的节点
        for (const auto& node : m_nodes) {
            if (node.manualPos) {
                inTree.insert(node.id);
            }
        }
    }

    float offsetX = 0;
    for (auto& node : m_nodes) {
        if (node.manualPos) continue;
        // 已经被 layoutSubtree 设置了位置，跳过
    }
}

// === 更新 ===

void Flowchart::update(float dt, const Input& input) {
    if (!m_visible) {
        m_fadeAlpha = std::max(0.0f, m_fadeAlpha - dt * 8.0f);
        return;
    }
    m_fadeAlpha = std::min(1.0f, m_fadeAlpha + dt * 8.0f);

    int mx, my;
    input.getMousePosition(mx, my);
    int wheelY = input.getMouseWheelY();

    // 鼠标滚轮缩放
    if (wheelY != 0) {
        float zoomDelta = wheelY > 0 ? 0.1f : -0.1f;
        // 以鼠标位置为中心缩放
        float worldX, worldY;
        screenToWorld(static_cast<float>(mx), static_cast<float>(my), worldX, worldY);
        m_zoom = std::clamp(m_zoom + zoomDelta, m_minZoom, m_maxZoom);
        // 调整 pan 使鼠标位置对应的世界坐标不变
        float newSx, newSy;
        worldToScreen(worldX, worldY, newSx, newSy);
        m_panX += (newSx - mx) / m_zoom;
        m_panY += (newSy - my) / m_zoom;
    }

    // 拖拽平移（右键或左键拖空白处）
    bool rightDown = input.isMouseButtonPressed(SDL_BUTTON_RIGHT);
    bool leftDown = input.isMouseButtonPressed(SDL_BUTTON_LEFT);

    if (rightDown && !m_dragging) {
        m_dragging = true;
        m_dragStartX = mx;
        m_dragStartY = my;
        m_panStartX = m_panX;
        m_panStartY = m_panY;
    }

    if (m_dragging) {
        if (!rightDown) {
            m_dragging = false;
        } else {
            m_panX = m_panStartX - (mx - m_dragStartX) / m_zoom;
            m_panY = m_panStartY - (my - m_dragStartY) / m_zoom;
        }
    }

    // 检测 hover
    m_hoverNodeId.clear();
    m_tooltipText.clear();
    for (const auto& node : m_nodes) {
        SDL_Rect rect = getNodeScreenRect(node);
        if (mx >= rect.x && mx < rect.x + rect.w &&
            my >= rect.y && my < rect.y + rect.h) {
            m_hoverNodeId = node.id;
            if (!node.description.empty()) {
                m_tooltipText = node.description;
            } else {
                m_tooltipText = node.label;
            }
            break;
        }
    }

    // tooltip 计时
    if (!m_hoverNodeId.empty()) {
        m_tooltipTimer += dt;
    } else {
        m_tooltipTimer = 0.0f;
    }

    // 左键点击节点 → 跳转
    if (leftDown && !m_dragging && !m_hoverNodeId.empty()) {
        auto* node = getNode(m_hoverNodeId);
        if (node && node->visited && !node->scriptLabel.empty() && m_jumpCallback) {
            m_jumpCallback(node->scriptLabel);
        }
    }

    // ESC 关闭
    if (input.isKeyPressed(SDL_SCANCODE_ESCAPE)) {
        hide();
    }
}

// === 渲染 ===

SDL_Rect Flowchart::getNodeScreenRect(const FlowchartNode& node) const {
    float sx, sy;
    worldToScreen(node.x, node.y, sx, sy);
    int w = static_cast<int>(node.width * m_zoom);
    int h = static_cast<int>(node.height * m_zoom);
    return {static_cast<int>(sx) - w / 2, static_cast<int>(sy) - h / 2, w, h};
}

void Flowchart::renderNode(const FlowchartNode& node, Uint8 alpha) {
    SDL_Rect rect = getNodeScreenRect(node);

    // 跳过屏幕外的节点
    int screenW = m_renderer->getWidth();
    int screenH = m_renderer->getHeight();
    if (rect.x + rect.w < 0 || rect.x > screenW ||
        rect.y + rect.h < 0 || rect.y > screenH) {
        return;
    }

    SDL_Color fillColor = getNodeColor(node.type, node.visited, node.current);
    SDL_Color borderColor = getNodeBorderColor(node.type, node.current);
    fillColor.a = alpha;
    borderColor.a = alpha;

    // 节点背景
    m_renderer->drawRect(static_cast<float>(rect.x), static_cast<float>(rect.y),
                         rect.w, rect.h, fillColor, true);

    // 边框
    m_renderer->drawRect(static_cast<float>(rect.x), static_cast<float>(rect.y),
                         rect.w, rect.h, borderColor, false);

    // hover 高亮边框
    if (node.id == m_hoverNodeId) {
        SDL_Color hoverColor = {255, 255, 255, alpha};
        m_renderer->drawRect(static_cast<float>(rect.x - 2), static_cast<float>(rect.y - 2),
                             rect.w + 4, rect.h + 4, hoverColor, false);
    }

    // 节点文字
    if (m_font && m_zoom > 0.5f) {
        SDL_Color textColor = node.visited ? SDL_Color{255, 255, 255, alpha}
                                           : SDL_Color{120, 120, 130, alpha};
        auto textTex = m_renderer->renderText(node.label, m_font, textColor);
        if (textTex) {
            int tw = textTex->width();
            int th = textTex->height();
            float tx = rect.x + (rect.w - tw) / 2.0f;
            float ty = rect.y + (rect.h - th) / 2.0f;
            m_renderer->drawTexture(textTex.get(), tx, ty, 1.0f, 1.0f, alpha);
        }
    }

    // 未访问标记（锁图标用文字代替）
    if (!node.visited && m_smallFont && m_zoom > 0.5f) {
        SDL_Color lockColor = {180, 180, 190, alpha};
        auto lockTex = m_renderer->renderText("?", m_smallFont, lockColor);
        if (lockTex) {
            m_renderer->drawTexture(lockTex.get(),
                                    rect.x + rect.w - 16, rect.y + 4, 1.0f, 1.0f, alpha);
        }
    }

    // 结局类型标签
    std::string typeLabel = nodeTypeLabel(node.type);
    if (!typeLabel.empty() && node.visited && m_smallFont && m_zoom > 0.6f) {
        SDL_Color typeColor = borderColor;
        auto typeTex = m_renderer->renderText(typeLabel, m_smallFont, typeColor);
        if (typeTex) {
            m_renderer->drawTexture(typeTex.get(),
                                    rect.x + 4, rect.y + rect.h - 14, 1.0f, 1.0f, alpha);
        }
    }
}

void Flowchart::renderEdge(const FlowchartEdge& edge, Uint8 alpha) {
    auto* from = getNode(edge.fromId);
    auto* to = getNode(edge.toId);
    if (!from || !to) return;

    float fromSx, fromSy, toSx, toSy;
    worldToScreen(from->x, from->y, fromSx, fromSy);
    worldToScreen(to->x, to->y, toSx, toSy);

    // 调整起止点到节点边缘
    float dx = toSx - fromSx;
    float dy = toSy - fromSy;
    float dist = std::sqrt(dx * dx + dy * dy);
    if (dist < 1.0f) return;

    float nx = dx / dist;
    float ny = dy / dist;

    float fromHalfW = from->width * m_zoom / 2.0f;
    float fromHalfH = from->height * m_zoom / 2.0f;
    float toHalfW = to->width * m_zoom / 2.0f;
    float toHalfH = to->height * m_zoom / 2.0f;

    // 从源节点边缘出发
    float startX = fromSx + nx * fromHalfW;
    float startY = fromSy + ny * fromHalfH;
    // 到目标节点边缘
    float endX = toSx - nx * toHalfW;
    float endY = toSy - ny * toHalfH;

    // 连线颜色
    bool bothVisited = from->visited && to->visited;
    SDL_Color lineColor;
    if (bothVisited) {
        lineColor = {160, 200, 240, alpha};
    } else {
        lineColor = {70, 70, 80, alpha};
    }

    m_renderer->drawLine(startX, startY, endX, endY, lineColor);

    // 箭头
    float arrowSize = 8.0f * m_zoom;
    float ax1 = endX - nx * arrowSize + ny * arrowSize * 0.5f;
    float ay1 = endY - ny * arrowSize - nx * arrowSize * 0.5f;
    float ax2 = endX - nx * arrowSize - ny * arrowSize * 0.5f;
    float ay2 = endY - ny * arrowSize + nx * arrowSize * 0.5f;

    m_renderer->drawLine(endX, endY, ax1, ay1, lineColor);
    m_renderer->drawLine(endX, endY, ax2, ay2, lineColor);

    // 连线标签
    if (!edge.label.empty() && m_smallFont && m_zoom > 0.5f) {
        float midX = (startX + endX) / 2.0f;
        float midY = (startY + endY) / 2.0f;
        SDL_Color labelColor = bothVisited ? SDL_Color{220, 220, 240, alpha}
                                           : SDL_Color{100, 100, 110, alpha};
        auto labelTex = m_renderer->renderText(edge.label, m_smallFont, labelColor);
        if (labelTex) {
            float lx = midX - labelTex->width() / 2.0f;
            float ly = midY - labelTex->height() / 2.0f;
            m_renderer->drawTexture(labelTex.get(), lx, ly, 1.0f, 1.0f, alpha);
        }
    }
}

void Flowchart::renderTooltip(Uint8 alpha) {
    if (m_hoverNodeId.empty() || m_tooltipTimer < 0.5f || m_tooltipText.empty()) return;
    if (!m_smallFont) return;

    auto tex = m_renderer->renderText(m_tooltipText, m_smallFont, {240, 240, 250, alpha});
    if (!tex) return;

    int mx, my;
    // 获取鼠标位置（通过渲染器尺寸估算）
    int tw = tex->width();
    int th = tex->height();
    int padding = 6;

    // 使用 hover 节点的位置作为参考
    auto* node = getNode(m_hoverNodeId);
    if (!node) return;

    SDL_Rect nodeRect = getNodeScreenRect(*node);
    float tx = static_cast<float>(nodeRect.x);
    float ty = static_cast<float>(nodeRect.y - th - padding * 2 - 4);

    // 背景
    SDL_Color bg = {20, 20, 30, static_cast<Uint8>(alpha * 0.9f)};
    m_renderer->drawRect(tx - padding, ty - padding,
                         tw + padding * 2, th + padding * 2, bg, true);
    SDL_Color border = {100, 120, 160, alpha};
    m_renderer->drawRect(tx - padding, ty - padding,
                         tw + padding * 2, th + padding * 2, border, false);

    m_renderer->drawTexture(tex.get(), tx, ty, 1.0f, 1.0f, alpha);
}

void Flowchart::renderLegend(Uint8 alpha) {
    if (!m_smallFont) return;

    int screenW = m_renderer->getWidth();
    float legendX = 16.0f;
    float legendY = 16.0f;

    // 背景
    SDL_Color bg = {15, 15, 25, static_cast<Uint8>(alpha * 0.85f)};
    m_renderer->drawRect(legendX - 8, legendY - 8, 180, 150, bg, true);
    SDL_Color border = {80, 90, 120, alpha};
    m_renderer->drawRect(legendX - 8, legendY - 8, 180, 150, border, false);

    // 标题
    auto titleTex = m_renderer->renderText("Flowchart", m_smallFont, {200, 200, 220, alpha});
    if (titleTex) {
        m_renderer->drawTexture(titleTex.get(), legendX, legendY, 1.0f, 1.0f, alpha);
    }
    legendY += 24;

    struct LegendItem {
        SDL_Color color;
        std::string label;
    };
    std::vector<LegendItem> items = {
        {{60, 180, 100, alpha},   "Start"},
        {{70, 110, 160, alpha},   "Scene (visited)"},
        {{50, 50, 60, alpha},     "Locked"},
        {{80, 200, 120, alpha},   "Good End"},
        {{200, 80, 80, alpha},    "Bad End"},
    };

    for (const auto& item : items) {
        m_renderer->drawRect(legendX, legendY, 14, 14, item.color, true);
        auto tex = m_renderer->renderText(item.label, m_smallFont, {200, 200, 210, alpha});
        if (tex) {
            m_renderer->drawTexture(tex.get(), legendX + 20, legendY - 2, 1.0f, 1.0f, alpha);
        }
        legendY += 22;
    }
}

void Flowchart::render() {
    if (m_fadeAlpha < 0.01f) return;

    Uint8 alpha = static_cast<Uint8>(m_fadeAlpha * 255);

    int screenW = m_renderer->getWidth();
    int screenH = m_renderer->getHeight();

    // 半透明背景遮罩
    SDL_Color bg = {10, 10, 18, static_cast<Uint8>(alpha * 0.85f)};
    m_renderer->drawRect(0, 0, screenW, screenH, bg, true);

    // 渲染连线（先画线，再画节点，节点覆盖在线上面）
    for (const auto& edge : m_edges) {
        renderEdge(edge, alpha);
    }

    // 渲染节点
    for (const auto& node : m_nodes) {
        renderNode(node, alpha);
    }

    // tooltip
    renderTooltip(alpha);

    // 图例
    renderLegend(alpha);

    // 操作提示
    if (m_smallFont) {
        SDL_Color hintColor = {140, 150, 170, alpha};
        auto hintTex = m_renderer->renderText(
            "Scroll: Zoom | R-Drag: Pan | Click: Jump | ESC: Close",
            m_smallFont, hintColor);
        if (hintTex) {
            m_renderer->drawTexture(hintTex.get(),
                                    screenW - hintTex->width() - 16,
                                    screenH - 24, 1.0f, 1.0f, alpha);
        }
    }
}
