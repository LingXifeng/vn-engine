-- demo_features.lua - 演示 VN Engine 六大新功能
-- 运行方式: 在游戏中调用 startScript("demo_features")

function demo_features()
    -- ========== 1. 标题画面 ==========
    -- (标题画面由引擎状态机管理，Lua 可通过 System 切换状态)
    System.log("=== VN Engine 六大功能演示 ===")

    -- ========== 2. 粒子特效 ==========
    -- Particle.create(type, count, speed)
    -- 支持: rain, snow, confetti, fire, sakura, spark

    System.log(">> 樱花飘落特效")
    Particle.create("sakura", 80, 50)
    Tween.wait(3)

    System.log(">> 雨天特效")
    Particle.create("rain", 200, 300)
    Tween.wait(3)
    Particle.stop()

    System.log(">> 火焰特效")
    Particle.create("fire", 150, 80)
    Tween.wait(3)
    Particle.stop()

    -- ========== 3. 富文本渲染 ==========
    -- 支持 {b}粗体{/b}, {i}斜体{/i}, {ruby:注音}文字{/ruby} 等标记
    -- RichText.render(text, x, y, maxWidth)

    RichText.setSpacing(6, 1)  -- 行间距6, 字间距1
    -- 渲染带注音和粗体的文本
    RichText.render("{ruby:とうきょう}東京{/ruby}は{b}日本{/b}の首都です", 100, 200, 800)
    Tween.wait(3)

    -- ========== 4. CG 画廊 ==========
    -- Gallery.unlock(id) - 解锁 CG
    -- Gallery.show() - 显示画廊界面

    Gallery.unlock("cg01")
    Gallery.unlock("cg02")
    Gallery.unlock("cg03")
    System.log("已解锁 3 张 CG")
    -- Gallery.show()  -- 取消注释以显示画廊界面
    -- Tween.wait(5)

    -- ========== 5. 音乐室 ==========
    -- MusicRoom.add(id, title, artist, path) - 添加 BGM

    MusicRoom.add("bgm01", "メインテーマ", "作曲者A", "bgm/main_theme.ogg")
    MusicRoom.add("bgm02", "日常BGM", "作曲者B", "bgm/daily.ogg")
    MusicRoom.add("bgm03", "戦闘BGM", "作曲者C", "bgm/battle.ogg")
    System.log("音乐室已添加 3 首 BGM")
    -- MusicRoom.show()  -- 取消注释以显示音乐室界面
    -- Tween.wait(5)

    -- ========== 6. Credits 滚动字幕 ==========
    -- Credits.addTitle(text) - 添加大标题
    -- Credits.addHeading(text) - 添加中标题
    -- Credits.addName(text) - 添加人名
    -- Credits.addSmall(text) - 添加小字
    -- Credits.loadFile(path) - 从文件加载
    -- Credits.show() - 开始滚动
    -- Credits.setSpeed(speed) - 设置滚动速度

    Credits.addTitle("VN Engine")
    Credits.addSmall("Powered by C++ + SDL2 + Lua")
    Credits.addBlank(40)

    Credits.addHeading("制作人员")
    Credits.addBlank(10)
    Credits.addName("企画・原案")
    Credits.addName("VN Engine Team")
    Credits.addBlank(20)

    Credits.addHeading("プログラム")
    Credits.addName("メインプログラマー")
    Credits.addName("Engine Core")
    Credits.addBlank(20)

    Credits.addHeading("グラフィック")
    Credits.addName("UI Designer")
    Credits.addName("Effect Artist")
    Credits.addBlank(20)

    Credits.addHeading("サウンド")
    Credits.addName("BGM Composer")
    Credits.addName("Sound Effects")
    Credits.addBlank(30)

    Credits.addSmall("Copyright (C) 2024 VN Engine Project")
    Credits.addSmall("All Rights Reserved.")

    Credits.setSpeed(60)  -- 滚动速度 60px/s
    Credits.show()

    System.log("Credits 滚动中... 按 ESC 跳过")
    -- 等待 Credits 结束
    Tween.wait(30)

    System.log("=== 演示结束 ===")
end
