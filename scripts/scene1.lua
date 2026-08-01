-- scene1.lua - 第一章：邂逅

scene1 = {}

function scene1.start()
    -- 切换背景（黄昏的教室）
    scene.setBackground("bg/classroom_evening.png", 1.5)
    flow.wait(1.5)

    -- BGM
    audio.playBGM("bgm/scene1.ogg", 2.0)

    -- 旁白
    narrate("放学后的教室，夕阳从窗户洒入。")
    flow.waitClick()

    narrate("教室里只剩下一个人——")
    flow.waitClick()

    -- 显示立绘
    character.show("yuki", "normal", "left", 0.5)
    flow.wait(0.5)

    -- 对话
    say("雪", "……你还在这里啊。")
    flow.waitClick()

    say("雪", "今天值日生是你吧？辛苦了。")
    flow.waitClick()

    -- 切换表情
    character.setExpression("yuki", "happy")
    say("雪", "啊，那个……如果不急的话，能聊聊天吗？")
    flow.waitClick()

    -- 选择支
    local choice = scene.addChoice("当然可以")
    scene.addChoice("抱歉，我还有事")
    scene.addChoice("……")

    flow.waitChoice()

    if choice == 1 then
        -- 好感路线
        character.setExpression("yuki", "happy")
        say("雪", "谢谢你！那……我们走吧。")
        game.flags.met_heroine = true
        game.flags.yuki_affinity = 1
    elseif choice == 2 then
        -- 中立路线
        character.setExpression("yuki", "sad")
        say("雪", "是吗……那，明天见。")
        game.flags.met_heroine = true
        game.flags.yuki_affinity = 0
    else
        -- 沉默路线
        character.setExpression("yuki", "normal")
        say("雪", "……")
        flow.wait(1.0)
        narrate("沉默在教室里蔓延。")
        flow.waitClick()
        say("雪", "好吧。那先这样。")
        game.flags.met_heroine = true
        game.flags.yuki_affinity = -1
    end

    flow.waitClick()

    -- 隐藏立绘
    character.hide("yuki", 0.5)
    flow.wait(0.5)

    -- 切换背景
    scene.fadeOut(1.0)
    scene.setBackground("bg/sunset_road.png")
    scene.fadeIn(1.0)
    flow.wait(1.0)

    narrate("夕阳下，两个人的影子被拉得很长。")
    flow.waitClick()

    -- 淡出
    audio.stopBGM(2.0)
    scene.fadeOut(2.0)
    flow.wait(2.0)
end
