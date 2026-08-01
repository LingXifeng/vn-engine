--[[
    example_game.lua - 完整示例剧本
    
    本脚本演示 VN Engine 的全部 15 项功能，串联成一段完整剧情。
    游戏创作者可以此为模板，编写自己的视觉小说。
    
    功能清单：
      1. 标题/配置画面    9. Backlog（历史记录）
      2. CG 画廊          10. Auto/Skip（自动/跳过）
      3. 粒子特效         11. 转场效果
      4. 富文本（Ruby/粗体/阴影） 12. 背景动画
      5. 音乐室/语音画廊   13. 结局列表
      6. 制作人员名单      14. 场景回放
      7. 流程图           15. 术语词典
      8. 存档/读档 UI
]]

-- ============================================================
-- 游戏全局状态
-- ============================================================
game = {
    chapter = 0,
    flags = {},
    day = 1,
    endings = {},
}

-- ============================================================
-- 初始化：注册所有功能数据
-- ============================================================
function initGame()
    -- 设置窗口标题
    System.setTitle("樱花物语 - VN Engine Demo")
    System.log("=== 樱花物语 启动 ===")
    
    -- 【功能13：结局列表】预注册结局
    -- Ending.add(id, title, description?, condition?)
    Ending.add("good",  "Good End  - 樱花之下", "在樱花树下许下约定")
    Ending.add("normal", "Normal End - 各自的道路", "各自走向不同的未来")
    Ending.add("bad",   "Bad End   - 消失的背影", "错过的心意无法挽回")
    
    -- 【功能14：场景回放】预注册可回放场景
    -- SceneReplay.add(id, title, description?, scriptName?, startLine?, endLine?, chapter?)
    SceneReplay.add("ch1_opening", "第一章：樱花飘落", "故事的开端", "example_game", 0, 0, "第一章")
    SceneReplay.add("ch2_park",    "第二章：公园午后", "公园里的日常", "example_game", 0, 0, "第二章")
    SceneReplay.add("ch3_sunset",  "第三章：黄昏告白", "黄昏的告白", "example_game", 0, 0, "第三章")
    
    -- 【功能15：术语词典】注册游戏术语
    -- Glossary.add(term, description, category?, reading?)
    Glossary.add("樱花", "本游戏的主题花。春天开放，花瓣随风飘落，象征短暂而美丽的生命。", "一般", "さくら")
    Glossary.add("美咲", "女主角。性格温柔，喜欢樱花。和主人公是青梅竹马。", "角色", "みさき")
    Glossary.add("公园", "两人经常一起散步的地方。长椅上留下了许多回忆。", "地点")
    
    -- 【功能2：CG画廊】预注册CG（游戏中通过事件解锁）
    -- Gallery.unlock("cg01")  -- 在剧情中解锁
    
    -- 【功能5：音乐室】注册BGM曲目
    -- MusicRoom.add(id, title, artist, path)
    MusicRoom.add("bgm_opening", "樱花之风", "VN Engine", "bgm/opening.ogg")
    MusicRoom.add("bgm_daily",   "日常时光", "VN Engine", "bgm/daily.ogg")
    MusicRoom.add("bgm_emotion", "心动时刻", "VN Engine", "bgm/emotion.ogg")
    MusicRoom.add("bgm_sad",     "离别之音", "VN Engine", "bgm/sad.ogg")
    
    -- 【功能7：流程图】构建剧情流程节点
    -- flowchart.addNode(id, label, type, scriptLabel?, description?)
    flowchart.addNode("start",      "故事开始",   "start")
    flowchart.addNode("ch1",        "第一章",     "chapter")
    flowchart.addNode("ch2",        "第二章",     "chapter")
    flowchart.addNode("ch3",        "第三章",     "chapter")
    flowchart.addNode("good_end",   "Good End",   "ending")
    flowchart.addNode("normal_end", "Normal End", "ending")
    flowchart.addNode("bad_end",    "Bad End",    "ending")
    flowchart.addEdge("start", "ch1")
    flowchart.addEdge("ch1", "ch2")
    flowchart.addEdge("ch2", "ch3")
    flowchart.addEdge("ch3", "good_end")
    flowchart.addEdge("ch3", "normal_end")
    flowchart.addEdge("ch3", "bad_end")
    
    System.log("游戏数据初始化完成")
end

-- ============================================================
-- 第一章：樱花飘落
-- ============================================================
function chapter1()
    game.chapter = 1
    System.log(">> 第一章开始")
    
    -- 【功能7：流程图】标记当前节点
    flowchart.markCurrent("ch1")
    flowchart.markVisited("start")
    
    -- 【功能11：转场效果】淡入
    -- Transition.fade(duration?)
    Transition.fade(1.0)
    
    -- 【功能12：背景动画】设置天气效果
    -- BgEffect.setWeather(type?, intensity?)  type: "rain"|"snow"|"fog"|"none"
    BgEffect.setWeather("none", 1.0)
    
    -- 【功能1：标题/配置】播放开场BGM
    -- Audio.playBGM(path, loop?, fade?)
    Audio.playBGM("bgm/opening.ogg", -1)
    
    -- 【功能14：场景回放】标记场景开始
    SceneReplay.unlock("ch1_opening")
    
    -- 旁白
    -- Text.narrate(text) - 显示旁白（无角色名），等待点击
    Text.narrate("—— 樱花飘落的季节 ——")
    Text.narrate("春风拂过，粉色的花瓣在空中飞舞。")
    Text.narrate("在这条熟悉的街道上，两个人的故事开始了。")
    
    -- 【功能3：粒子特效】创建樱花粒子
    -- Particle.create(type, count?, speed?)  type: "rain"|"snow"|"confetti"|"fire"|"sakura"|"spark"
    Particle.create("sakura", 50, 100.0)
    
    -- 角色登场
    -- Character.show(name, expression?, position?, fade?)
    -- position: "left"|"center"|"right"
    Character.show("misaki", "normal", "center", 0.5)
    
    -- 对话
    -- Text.say(name, text) - 显示角色对话，等待点击
    Text.say("美咲", "今天的风好温柔呢。")
    Text.say("美咲", "看，樱花飘下来了。")
    
    -- 【功能4：富文本】使用Ruby注音和粗体标记
    -- 在文本中使用 {ruby:注音}文字{/ruby} 和 {b}...{/b} 标记
    Text.say("美咲", "この{ruby:さくら}桜{/ruby}、綺麗だね。")
    
    -- 【功能8：存档/读档】提示玩家可以存档
    Text.narrate("（提示：按 F3 可以随时存档）")
    
    -- 【功能2：CG画廊】解锁CG
    Gallery.unlock("cg01")
    Text.narrate("（CG「樱花街道」已解锁）")
    
    Text.say("主人公", "嗯，每年这个时候都会这样呢。")
    Text.say("美咲", "走吧，去公园看看？")
    
    -- 【功能11：转场效果】淡出 → 切换场景
    Transition.fade(0.5)
    BgEffect.stopShake()
    Character.hide("misaki")
    
    System.log(">> 第一章结束")
end

-- ============================================================
-- 第二章：公园午后
-- ============================================================
function chapter2()
    game.chapter = 2
    System.log(">> 第二章开始")
    
    flowchart.markCurrent("ch2")
    flowchart.markVisited("ch1")
    
    -- 【功能11：转场效果】滑入转场
    -- Transition.slide(dir?, duration?)  dir: "left"|"right"|"up"|"down"
    Transition.slide("left", 0.8)
    
    -- 切换BGM
    Audio.stopBGM()
    Audio.playBGM("bgm/daily.ogg", -1)
    
    -- 【功能14：场景回放】
    SceneReplay.unlock("ch2_park")
    
    -- 【功能12：背景动画】微风效果
    BgEffect.setWeather("none", 0.5)
    
    Text.narrate("—— 公园的午后 ——")
    Text.narrate("阳光透过树叶的缝隙，在地上投下斑驳的光影。")
    
    -- 角色登场
    Character.show("misaki", "smile", "left", 0.3)
    Character.show("hero", "normal", "right", 0.3)
    
    Text.say("美咲", "啊，长椅还在这里呢。")
    Text.say("主人公", "我们以前经常坐在这里聊天。")
    Text.say("美咲", "嗯，那时候什么都不懂呢。")
    
    -- 【功能15：术语词典】在对话中引用术语
    Glossary.unlock("公园")
    Text.narrate("（词典条目「公园」已解锁）")
    
    -- 【功能9：Backlog】历史记录自动累积
    -- （系统自动将对话加入Backlog，无需手动调用）
    
    -- 【功能10：Auto/Skip】演示自动模式
    Text.narrate("（提示：按 A 键开启自动模式，按 S 键跳过）")
    
    -- 分支选择
    Text.narrate("美咲看着远方的樱花树，似乎在想什么。")
    -- Text.choice({选项列表}) 返回选中的索引（1-based）
    local choice = Text.choice({
        "问她在想什么",
        "安静地陪在她身边",
        "提议去买冰淇淋"
    })
    
    if choice == 1 then
        -- 【功能5：语音画廊】播放语音
        -- Audio.playVoice(path)
        Audio.playVoice("voice/misaki_01.ogg")
        Text.say("主人公", "在想什么呢？")
        Text.say("美咲", "我在想……如果我们一直这样就好了。")
        Text.say("主人公", "……")
        
        -- 【功能3：粒子特效】火花粒子
        Particle.create("spark", 20, 80.0)
        
    elseif choice == 2 then
        Text.say("主人公", "……")
        Text.say("美咲", "谢谢你。有时候不需要说话也很好。")
        Text.narrate("两个人安静地坐着，听着风声。")
        
    else
        Text.say("主人公", "去买冰淇淋吧！")
        Text.say("美咲", "好呀！我要草莓味的。")
        Text.say("主人公", "那我要巧克力味的。")
        
        -- 【功能2：CG画廊】解锁新CG
        Gallery.unlock("cg02")
        Text.narrate("（CG「冰淇淋约会」已解锁）")
    end
    
    -- 【功能12：背景动画】屏幕震动效果
    -- BgEffect.shake(intensity?, duration?)
    Text.narrate("突然，一阵大风吹过——")
    BgEffect.shake(8.0, 0.5)
    Text.narrate("樱花瓣被吹得漫天飞舞。")
    
    -- 【功能11：转场效果】溶解转场
    -- Transition.dissolve(duration?)
    Transition.dissolve(0.8)
    Character.hide("misaki")
    Character.hide("hero")
    
    System.log(">> 第二章结束")
end

-- ============================================================
-- 第三章：黄昏告白
-- ============================================================
function chapter3()
    game.chapter = 3
    System.log(">> 第三章开始")
    
    flowchart.markCurrent("ch3")
    flowchart.markVisited("ch2")
    
    -- 【功能11：转场效果】百叶窗转场
    -- Transition.blind(duration?)
    Transition.blind(1.0)
    
    -- 切换BGM
    Audio.stopBGM()
    Audio.playBGM("bgm/emotion.ogg", -1)
    
    -- 【功能14：场景回放】
    SceneReplay.unlock("ch3_sunset")
    
    -- 【功能12：背景动画】黄昏效果（用fog模拟）
    BgEffect.setWeather("fog", 0.3)
    
    Text.narrate("—— 黄昏时分 ——")
    Text.narrate("夕阳将天空染成橘红色。")
    Text.narrate("两个人的影子在草地上拉得很长。")
    
    Character.show("misaki", "serious", "center", 0.5)
    
    Text.say("美咲", "有件事……我一直想告诉你。")
    Text.say("主人公", "什么？")
    
    -- 【功能4：富文本】粗体强调标记
    Text.say("美咲", "{b}其实，我一直……{/b}")
    Text.say("美咲", "一直很喜欢和你在一起。")
    
    -- 【功能15：术语词典】解锁角色条目
    Glossary.unlock("美咲")
    Glossary.unlock("樱花")
    
    -- 关键选择
    Text.narrate("美咲的眼中映着夕阳的光。")
    local choice2 = Text.choice({
        "我也一直很珍惜和你在一起的时光",
        "……（沉默）",
        "对不起，我一直把你当朋友"
    })
    
    if choice2 == 1 then
        -- Good End
        Text.say("主人公", "我也一直很珍惜和你在一起的时光。")
        Text.say("主人公", "和你在一起的时候，是我最开心的时刻。")
        
        -- 【功能3：粒子特效】樱花雨
        Particle.create("sakura", 100, 150.0)
        
        Text.say("美咲", "真的吗……？")
        Text.say("主人公", "真的。")
        
        Text.narrate("美咲笑了，眼中泛着泪光。")
        Text.narrate("樱花瓣在两人之间飘落。")
        
        -- 【功能2：CG画廊】解锁结局CG
        Gallery.unlock("cg_good")
        
        -- 【功能13：结局列表】解锁Good End
        Ending.unlock("good")
        
        -- 【功能11：转场效果】白色淡入
        -- Transition.fadeWhite(duration?)
        Transition.fadeWhite(2.0)
        
        Text.narrate("—— Good End：樱花之下 ——")
        Text.narrate("从那以后，两个人经常一起来看樱花。")
        Text.narrate("每一年的春天，樱花都会如期绽放。")
        Text.narrate("就像他们的约定一样。")
        
        game.endings[#game.endings + 1] = "good"
        
    elseif choice2 == 2 then
        -- Normal End
        Text.say("主人公", "……")
        Text.say("美咲", "没关系。不用说什么。")
        Text.say("美咲", "只要能像现在这样在一起，我就很满足了。")
        
        Text.narrate("夕阳渐渐沉入地平线。")
        Text.narrate("两个人的影子渐渐模糊，最终消失在暮色中。")
        
        -- 【功能2：CG画廊】
        Gallery.unlock("cg_normal")
        
        -- 【功能13：结局列表】
        Ending.unlock("normal")
        
        Transition.fade(2.0)
        
        Text.narrate("—— Normal End：各自的道路 ——")
        Text.narrate("那天之后，两个人还是像以前一样。")
        Text.narrate("只是有些话，始终没有说出口。")
        
        game.endings[#game.endings + 1] = "normal"
        
    else
        -- Bad End
        Text.say("主人公", "对不起，美咲。")
        Text.say("主人公", "我一直把你当好朋友。")
        
        Text.say("美咲", "……是这样啊。")
        Text.narrate("美咲的笑容僵在脸上。")
        
        -- 【功能12：背景动画】雨效果
        BgEffect.setWeather("rain", 1.0)
        Audio.stopBGM()
        Audio.playBGM("bgm/sad.ogg", -1)
        
        Text.say("美咲", "没关系。我知道了。")
        Text.narrate("美咲转身离去，背影渐渐消失在雨中。")
        
        -- 【功能2：CG画廊】
        Gallery.unlock("cg_bad")
        
        -- 【功能13：结局列表】
        Ending.unlock("bad")
        
        Transition.fade(2.0)
        
        Text.narrate("—— Bad End：消失的背影 ——")
        Text.narrate("那天的雨，下了很久很久。")
        Text.narrate("有些错过，是一辈子都无法挽回的。")
        
        game.endings[#game.endings + 1] = "bad"
    end
    
    System.log(">> 第三章结束")
end

-- ============================================================
-- 结尾：展示统计和功能
-- ============================================================
function ending()
    System.log(">> 结尾画面")
    
    -- 【功能13：结局列表】显示已解锁结局
    Text.narrate("=== 游戏结束 ===")
    Text.narrate("已达成结局：" .. #game.endings .. " / 3")
    
    -- 【功能7：流程图】显示流程图
    flowchart.markVisited("ch3")
    if game.endings[1] == "good" then
        flowchart.markVisited("good_end")
    elseif game.endings[1] == "normal" then
        flowchart.markVisited("normal_end")
    else
        flowchart.markVisited("bad_end")
    end
    Text.narrate("（流程图已更新，可在主菜单查看）")
    
    -- 【功能14：场景回放】提示
    Text.narrate("（已解锁的场景可以在主菜单回放）")
    
    -- 【功能15：术语词典】显示
    Text.narrate("（词典条目可在主菜单查看）")
    
    -- 【功能6：制作人员名单】滚动显示
    -- Credits.addTitle(text) / addHeading(text) / addName(text) / addSmall(text)
    Text.narrate("=== 制作人员 ===")
    Credits.addTitle("樱花物语")
    Credits.addHeading("制作")
    Credits.addName("VN Engine Demo")
    Credits.addHeading("音乐")
    Credits.addName("VN Engine Sound Team")
    Credits.addHeading("脚本")
    Credits.addName("VN Engine Script Team")
    Credits.addSmall("Powered by C++/SDL2/Lua")
    Credits.show()
    
    -- 等待Credits结束
    -- Tween.wait(duration) - 协程等待指定秒数
    Tween.wait(3.0)
    
    Text.narrate("感谢游玩樱花物语！")
    Text.narrate("—— VN Engine Demo ——")
    
    System.log("=== 游戏结束 ===")
end

-- ============================================================
-- 主函数（由引擎作为协程启动）
-- ============================================================
function main()
    -- 初始化游戏数据
    initGame()
    
    -- 第一章
    chapter1()
    
    -- 第二章
    chapter2()
    
    -- 第三章
    chapter3()
    
    -- 结尾
    ending()
    
    -- 退出
    System.quit()
end
