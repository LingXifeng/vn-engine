-- characters.lua - 角色定义
-- 定义游戏中所有角色的属性

characters = {}

-- 角色定义
characters.akira = {
    name = "晶",
    fullName = "氷室 晶",
    color = {r = 200, g = 220, b = 255},
    defaultExpression = "normal",
}

characters.yuki = {
    name = "雪",
    fullName = "白藤 雪音",
    color = {r = 255, g = 200, b = 220},
    defaultExpression = "normal",
}

characters.narrator = {
    name = "",
    fullName = "",
    color = {r = 200, g = 200, b = 200},
}

-- 获取角色名
function characters.getName(id)
    if characters[id] then
        return characters[id].name
    end
    return ""
end

-- 获取角色颜色
function characters.getColor(id)
    if characters[id] then
        return characters[id].color
    end
    return {r = 255, g = 255, b = 255}
end

-- 注册角色到引擎
function characters.register()
    -- 注册立绘
    character.define("akira", {
        normal = "char/akira_normal.png",
        happy  = "char/akira_happy.png",
        sad    = "char/akira_sad.png",
        angry  = "char/akira_angry.png",
    })

    character.define("yuki", {
        normal = "char/yuki_normal.png",
        happy  = "char/yuki_happy.png",
        sad    = "char/yuki_sad.png",
        angry  = "char/yuki_angry.png",
    })
end

-- 初始化时注册
characters.register()
