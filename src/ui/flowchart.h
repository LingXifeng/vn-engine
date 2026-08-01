#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <functional>
#include "renderer.h"
#include "input.h"

// 流程图节点类型
enum class FlowchartNodeType {
    START,          // 开始
    CHAPTER,        // 章节
    BRANCH,         // 分支点（选择）
    SCENE,          // 普通场景
    ENDING_GOOD,    // Good End
    ENDING_NORMAL,  // Normal End
    ENDING_BAD,     // Bad End
    ENDING_TRUE,    // True End
};

// 流程图节点
struct FlowchartNode {
    std::string id;             // 唯一标识
    std::string label;          // 显示名称
    FlowchartNodeType type = FlowchartNodeType::SCENE;
    std::string scriptLabel;    // 跳转用的 Lua 函数名
    std::string description;    // 简短描述（tooltip）

    // 布局位置（自动计算或手动指定）
    float x = 0.0f;
    float y = 0.0f;
    bool manualPos = false;     // 是否手动指定位置

    // 状态
    bool visited = false;       // 是否已到达
    bool current = false;       // 是否为当前位置

    // 渲染缓存
    int width = 160;
    int height = 50;
};

// 流程图连线
struct FlowchartEdge {
    std::string fromId;         // 起始节点 ID
    std::string toId;           // 目标节点 ID
    std::string label;          // 连线标签（如选项文字）
    bool highlighted = false;   // 是否高亮（当前路径）
};

// 流程图视图
class Flowchart {
public:
    Flowchart(Renderer* renderer);
    ~Flowchart();

    void setFont(TTF_Font* font) { m_font = font; }
    void setSmallFont(TTF_Font* font) { m_smallFont = font; }

    // === 节点管理 ===
    void addNode(const std::string& id, const std::string& label,
                 FlowchartNodeType type, const std::string& scriptLabel = "",
                 const std::string& description = "");
    void setNodePosition(const std::string& id, float x, float y);
    FlowchartNode* getNode(const std::string& id);
    bool hasNode(const std::string& id) const;

    // === 连线管理 ===
    void addEdge(const std::string& fromId, const std::string& toId,
                 const std::string& label = "");

    // === 进度追踪 ===
    void markVisited(const std::string& id);
    void markCurrent(const std::string& id);
    void clearCurrent();
    bool isVisited(const std::string& id) const;
    std::vector<std::string> getVisitedNodes() const;
    std::vector<std::string> getReachedEndings() const;
    void setVisitedNodes(const std::vector<std::string>& ids);

    // === 跳转回调 ===
    // 当用户点击已访问的节点时调用
    void setJumpCallback(std::function<void(const std::string& scriptLabel)> callback) {
        m_jumpCallback = callback;
    }

    // === 布局 ===
    void autoLayout();          // 自动布局（树形）
    void layoutFromNode(const std::string& rootId, float startX, float startY);

    // === 控制 ===
    void show();
    void hide();
    bool isVisible() const { return m_visible; }

    // === 缩放/平移 ===
    void setZoom(float zoom) { m_zoom = zoom; }
    float getZoom() const { return m_zoom; }
    void resetView();

    // === 更新与渲染 ===
    void update(float dt, const Input& input);
    void render();

private:
    Renderer* m_renderer;
    TTF_Font* m_font = nullptr;
    TTF_Font* m_smallFont = nullptr;

    bool m_visible = false;
    float m_fadeAlpha = 0.0f;

    // 节点与连线
    std::vector<FlowchartNode> m_nodes;
    std::vector<FlowchartEdge> m_edges;
    std::unordered_map<std::string, int> m_nodeIndex;  // id → index

    // 视图变换
    float m_zoom = 1.0f;
    float m_panX = 0.0f;
    float m_panY = 0.0f;
    float m_minZoom = 0.3f;
    float m_maxZoom = 3.0f;

    // 交互状态
    std::string m_hoverNodeId;
    bool m_dragging = false;
    int m_dragStartX = 0, m_dragStartY = 0;
    float m_panStartX = 0, m_panStartY = 0;
    float m_tooltipTimer = 0.0f;
    std::string m_tooltipText;

    // 跳转回调
    std::function<void(const std::string&)> m_jumpCallback;

    // 布局参数
    float m_nodeSpacingX = 220.0f;
    float m_nodeSpacingY = 90.0f;

    // 辅助方法
    SDL_Color getNodeColor(FlowchartNodeType type, bool visited, bool current) const;
    SDL_Color getNodeBorderColor(FlowchartNodeType type, bool current) const;
    std::string nodeTypeLabel(FlowchartNodeType type) const;

    // 坐标变换：世界坐标 → 屏幕坐标
    void worldToScreen(float wx, float wy, float& sx, float& sy) const;
    void screenToWorld(float sx, float sy, float& wx, float& wy) const;

    // 渲染辅助
    void renderNode(const FlowchartNode& node, Uint8 alpha);
    void renderEdge(const FlowchartEdge& edge, Uint8 alpha);
    void renderTooltip(Uint8 alpha);
    void renderLegend(Uint8 alpha);

    // 获取节点在屏幕上的矩形
    SDL_Rect getNodeScreenRect(const FlowchartNode& node) const;

    // 自动布局辅助
    void layoutSubtree(const std::string& nodeId, float x, float y,
                       std::unordered_set<std::string>& placed,
                       int depth);
    std::vector<std::string> getChildren(const std::string& nodeId) const;
    int getSubtreeWidth(const std::string& nodeId,
                        std::unordered_set<std::string>& visited) const;
};
