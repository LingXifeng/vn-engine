-- main_package_test.lua — 资源包运行时测试
-- 加载 .pak 包，验证 Package API，然后运行基本引擎功能

function main()
    System.log("=== 资源包运行时测试 ===")

    -- 加载资源包
    local ok = Package.load("test_resources.pak")
    System.log("Package.load: " .. tostring(ok))

    if ok then
        System.log("文件数: " .. Package.getFileCount())

        -- 列出前5个资源
        local list = Package.list()
        for i = 1, math.min(5, #list) do
            System.log("  " .. list[i])
        end

        -- 检查资源
        System.log("has('main.lua') = " .. tostring(Package.has("main.lua")))
        System.log("has('nonexistent') = " .. tostring(Package.has("nonexistent")))
    end

    -- 基本渲染测试
    Scene.fadeIn(0.5)
    Text.say("系统", "资源包测试完成")
    Flow.wait(1.0)

    -- 卸载包
    Package.unload()
    System.log("卸载后 isLoaded = " .. tostring(Package.isLoaded()))

    System.log("=== 测试结束 ===")
end
