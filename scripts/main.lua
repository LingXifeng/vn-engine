-- main.lua - VN Engine 运行时测试主脚本
-- 使用正确的 Lua API 名称（首字母大写的全局表）

-- 游戏全局状态
game = {
    chapter = 1,
    flags = {},
    day = 1,
}

-- 主函数（由引擎作为协程启动）
function main()
    -- 设置窗口标题
    System.setTitle("VN Engine - Runtime Test")
    System.log("=== 运行时测试开始 ===")

    -- 第一章：开场
    System.log("Chapter 1: Opening")
    Text.narrate("—— 樱花的季节 ——")
    
    Text.say("美咲", "今天天气真好呢。")
    Text.say("主人公", "是啊，我们去散步吧。")
    Text.say("美咲", "嗯！那就去公园吧。")
    
    Text.narrate("两人走在樱花飘落的街道上。")
    
    -- 选项分支
    Text.narrate("来到公园，该怎么做？")
    local choice = Text.choice({"在长椅上坐下", "去散步", "买冰淇淋"})
    
    if choice == 1 then
        Text.say("美咲", "啊，这里的长椅很舒服呢。")
        Text.say("主人公", "是啊，风吹过来很凉快。")
    elseif choice == 2 then
        Text.say("美咲", "散步真好，能和你一起走。")
        Text.say("主人公", "嗯，我也很享受。")
    else
        Text.say("美咲", "冰淇淋！我要草莓味的！")
        Text.say("主人公", "好，那我要巧克力味的。")
    end
    
    -- 第二章
    game.chapter = 2
    System.log("Chapter 2: Afternoon")
    Text.narrate("—— 下午的时光 ——")
    
    Text.say("美咲", "对了，你还记得我们第一次见面的时候吗？")
    Text.say("主人公", "当然记得。那天下着大雨。")
    Text.say("美咲", "你把伞借给了我，自己却淋湿了。")
    Text.say("主人公", "那没什么，谁都会那样做的。")
    Text.say("美咲", "不，对我来说那是特别的。")
    
    -- 第三章
    game.chapter = 3
    System.log("Chapter 3: Confession")
    Text.narrate("—— 黄昏时分 ——")
    
    Text.say("美咲", "其实……我有话想对你说。")
    Text.say("主人公", "什么？")
    Text.say("美咲", "谢谢你一直陪在我身边。")
    Text.say("主人公", "美咲……")
    
    local choice2 = Text.choice({"我也一直很感谢你", "……（沉默）"})
    
    if choice2 == 1 then
        Text.say("主人公", "我也一直很感谢你。能和你在一起，我很开心。")
        Text.say("美咲", "嗯……我也是。")
        Text.narrate("—— Good End ——")
    else
        Text.say("主人公", "……")
        Text.say("美咲", "没关系，不用说什么。")
        Text.narrate("—— Normal End ——")
    end
    
    -- 结尾
    Text.narrate("—— 感谢游玩 ——")
    System.log("=== 运行时测试结束 ===")
    
    -- 退出
    System.quit()
end
