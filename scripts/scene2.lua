-- scene2.lua - 第二章：日常

scene2 = {}

function scene2.start()
    -- 第二天早晨
    scene.setBackground("bg/school_morning.png", 2.0)
    audio.playBGM("bgm/morning.ogg", 2.0)
    flow.wait(2.0)

    narrate("第二天早晨。")
    flow.waitClick()

    narrate("校门口，熟悉的身影正在等待。")
    flow.waitClick()

    -- 显示立绘
    character.show("yuki", "happy", "left", 0.5)
    flow.wait(0.5)

    -- 根据好感度分支
    if game.flags.yuki_affinity >= 1 then
        -- 高好感路线
        say("雪", "早上好！昨天很开心呢。")
        flow.waitClick()

        character.show("akira", "normal", "right", 0.5)
        flow.wait(0.5)

        say("晶", "……嗯。")
        flow.waitClick()

        character.setExpression("yuki", "happy")
        say("雪", "今天也一起走吧？")
        flow.waitClick()

        -- 选择
        local choice = scene.addChoice("好啊")
        scene.addChoice("……随便")
        flow.waitChoice()

        if choice == 1 then
            say("雪", "嘿嘿，那走吧！")
            game.flags.yuki_affinity = game.flags.yuki_affinity + 1
        else
            say("雪", "……嗯，走吧。")
        end

    elseif game.flags.yuki_affinity == 0 then
        -- 中立路线
        say("雪", "早上好。")
        flow.waitClick()
        say("雪", "昨天……抱歉，打扰你了。")
        flow.waitClick()
        narrate("气氛有些微妙。")
        flow.waitClick()

    else
        -- 低好感路线
        say("雪", "……早。")
        flow.waitClick()
        narrate("简短的问候，然后各自走向教室。")
        flow.waitClick()
    end

    flow.waitClick()

    -- 隐藏角色
    character.hide("yuki", 0.5)
    character.hide("akira", 0.5)
    flow.wait(0.5)

    -- 教室场景
    scene.fadeOut(1.0)
    scene.setBackground("bg/classroom_morning.png")
    scene.fadeIn(1.0)
    flow.wait(1.0)

    narrate("教室里，新的一天开始了。")
    flow.waitClick()

    -- 淡出结束
    audio.stopBGM(2.0)
    scene.fadeOut(2.0)
    flow.wait(2.0)
end
