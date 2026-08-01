-- ============================================================
-- 流程图（Flowchart）功能演示脚本
-- 展示如何用 Lua API 构建故事分支图、追踪进度、显示流程图
-- ============================================================

print("=== Flowchart Demo ===")

-- 1. 构建流程图节点
-- 节点类型: start, chapter, branch, scene, good, normal, bad, true

flowchart.addNode("start",    "故事开始",     "start",   "scene_intro",   "游戏的开端")
flowchart.addNode("ch1",      "第一章",       "chapter", "scene_ch1",     "共通线第一章")
flowchart.addNode("branch1",  "选择A还是B?",  "branch",  "",              "第一个分支点")

-- A 路线
flowchart.addNode("routeA",   "A路线",        "scene",   "scene_routeA",  "选择了A的路线")
flowchart.addNode("routeA_2", "A-深入调查",   "scene",   "scene_a2",      "A路线第二章")
flowchart.addNode("endA_good","A-Good End",   "good",    "scene_endA_g",  "A路线好结局")
flowchart.addNode("endA_bad", "A-Bad End",    "bad",     "scene_endA_b",  "A路线坏结局")

-- B 路线
flowchart.addNode("routeB",   "B路线",        "scene",   "scene_routeB",  "选择了B的路线")
flowchart.addNode("routeB_2", "B-隐藏真相",   "scene",   "scene_b2",      "B路线第二章")
flowchart.addNode("endB_norm","B-Normal End", "normal",  "scene_endB_n",  "B路线普通结局")
flowchart.addNode("endB_true","B-True End",   "true",    "scene_endB_t",  "B路线真结局")

-- 2. 构建连线（分支结构）
flowchart.addEdge("start",    "ch1")
flowchart.addEdge("ch1",      "branch1")

flowchart.addEdge("branch1",  "routeA",   "选择A")
flowchart.addEdge("branch1",  "routeB",   "选择B")

flowchart.addEdge("routeA",   "routeA_2")
flowchart.addEdge("routeA_2", "endA_good", "信任同伴")
flowchart.addEdge("routeA_2", "endA_bad",  "独自行动")

flowchart.addEdge("routeB",   "routeB_2")
flowchart.addEdge("routeB_2", "endB_norm", "接受现实")
flowchart.addEdge("routeB_2", "endB_true", "追寻真相")

-- 3. 标记已访问的节点（模拟游戏进度）
flowchart.markVisited("start")
flowchart.markVisited("ch1")
flowchart.markVisited("branch1")
flowchart.markVisited("routeA")
flowchart.markVisited("routeA_2")
flowchart.markVisited("endA_good")

-- 标记当前位置
flowchart.markCurrent("endA_good")

-- 4. 自动布局
flowchart.autoLayout()

-- 5. 查询已到达的结局
local endings = flowchart.getEndings()
print("已到达的结局数量: " .. #endings)
for i, id in ipairs(endings) do
    print("  结局 " .. i .. ": " .. id)
end

local visited = flowchart.getVisited()
print("已访问节点数量: " .. #visited)

-- 6. 显示流程图
print("显示流程图... (ESC 关闭)")
flowchart.show()

print("=== Flowchart Demo Ready ===")
