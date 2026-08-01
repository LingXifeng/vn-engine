-- demo_new_features.lua
-- 演示8个新功能的Lua API用法

print("=== VN Engine 新功能演示 ===")

-- 1. 存档/读档 UI
print("\n[1] 存档/读档 UI")
SaveLoad.setSlotCount(20)       -- 设置20个存档槽位
SaveLoad.showSave()             -- 显示存档界面
-- SaveLoad.showLoad()          -- 显示读档界面
-- SaveLoad.hide()              -- 隐藏界面

-- 2. 履历回看
print("[2] 履历回看")
Backlog.add("美咲", "今天天气真好呢。", "voice/misaki/001.wav")
Backlog.add("主人公", "是啊，我们去散步吧。", "voice/hero/002.wav")
Backlog.add("美咲", "嗯！那就去公园吧。", "voice/misaki/003.wav")
Backlog.add("旁白", "两人走在樱花飘落的街道上。", "")
-- Backlog.show()               -- 显示履历界面
-- Backlog.clear()              -- 清空履历

-- 3. 自动/跳过模式
print("[3] 自动/跳过模式")
AutoSkip.setAuto(true)          -- 开启自动模式
AutoSkip.setAutoSpeed(3.0)      -- 自动模式每句等待3秒
-- AutoSkip.toggleAuto()        -- 切换自动模式
-- AutoSkip.setSkip(true)       -- 开启跳过模式（快进已读文本）
-- AutoSkip.toggleSkip()        -- 切换跳过模式
-- AutoSkip.stopAll()           -- 停止所有模式

-- 4. 画面转场特效
print("[4] 画面转场特效")
Transition.fade(0.5)            -- 黑色淡入淡出，0.5秒
-- Transition.fadeWhite(0.5)    -- 白色淡入淡出
-- Transition.slide("left", 0.4)   -- 向左滑动
-- Transition.slide("right", 0.4)  -- 向右滑动
-- Transition.slide("up", 0.4)     -- 向上滑动
-- Transition.slide("down", 0.4)   -- 向下滑动
-- Transition.dissolve(0.6)     -- 溶解转场
-- Transition.blind(0.5)        -- 百叶窗
-- Transition.mosaic(0.6)       -- 马赛克
-- Transition.curtain(0.6)      -- 帘幕
-- Transition.zoom(0.4)         -- 缩放
-- Transition.stop()            -- 停止转场
print("  转场活跃: " .. tostring(Transition.isActive()))

-- 5. 背景动画
print("[5] 背景动画")
BgEffect.setWeather("rain", 1.0)  -- 下雨效果，强度1.0
-- BgEffect.setWeather("snow", 0.8) -- 下雪效果
-- BgEffect.setWeather("fog", 0.5)  -- 雾效果
-- BgEffect.setWeather("none")      -- 关闭天气
BgEffect.shake(10.0, 0.3)         -- 屏幕震动，强度10，持续0.3秒
-- BgEffect.stopShake()             -- 停止震动

-- 6. 结局列表
print("[6] 结局列表")
Ending.add("good", "Good End", "与美咲幸福地生活在一起。", "完成所有好感度事件")
Ending.add("normal", "Normal End", "平淡的日常继续着。", "通关游戏")
Ending.add("bad", "Bad End", "孤独的结局。", "好感度不足")
Ending.add("true", "True End", "揭示一切真相的结局。", "收集所有线索后通关")
Ending.unlock("good")             -- 解锁Good End
Ending.unlock("normal")           -- 解锁Normal End
print("  Good End解锁: " .. tostring(Ending.isUnlocked("good")))
print("  True End解锁: " .. tostring(Ending.isUnlocked("true")))
print("  完成率: " .. tostring(Ending.getCompletion()))
-- Ending.show()                  -- 显示结局列表

-- 7. 场景回想
print("[7] 场景回想")
SceneReplay.add("scene01", "初遇", "主人公与美咲初次相遇的场景。", 
    "chapter1", 10, 50, "第一章")
SceneReplay.add("scene02", "告白", "主人公的告白场景。", 
    "chapter3", 20, 80, "第三章")
SceneReplay.add("scene03", "约定", "两人的重要约定。", 
    "chapter5", 5, 40, "第五章")
SceneReplay.unlock("scene01")     -- 解锁场景1
SceneReplay.unlock("scene02")     -- 解锁场景2
-- SceneReplay.show()             -- 显示场景回想界面

-- 8. 词典/用語集
print("[8] 词典/用語集")
Glossary.add("精灵", "居住在森林中的神秘种族，寿命长达千年。", "种族", "せいれい")
Glossary.add("圣剑", "传说中能斩断一切的光之剑。", "物品", "せいけん")
Glossary.add("魔力", "驱动魔法的基本能量。", "术语", "まりょく")
Glossary.add("结界", "保护特定区域的魔法屏障。", "术语", "けっかい")
Glossary.unlock("精灵")           -- 解锁"精灵"词条
Glossary.unlock("圣剑")           -- 解锁"圣剑"词条
-- Glossary.show()                -- 显示词典界面
-- Glossary.showTerm("精灵")      -- 直接显示"精灵"的详情

print("\n=== 演示完成 ===")
print("所有8个新功能已成功注册并可用。")
