-- demo_package.lua — 资源打包系统测试
-- 测试 Package API: load, isLoaded, has, list, getFileCount, unload

local function runPackageDemo()
    print("[Lua] === 资源打包系统测试 ===")

    -- 1. 检查初始状态
    print("[Lua] 1. 初始状态: isLoaded = " .. tostring(Package.isLoaded()))

    -- 2. 加载 .pak 包
    print("[Lua] 2. 加载 test_resources.pak ...")
    local ok = Package.load("test_resources.pak")
    print("[Lua]    结果: " .. tostring(ok))
    print("[Lua]    isLoaded = " .. tostring(Package.isLoaded()))

    if not ok then
        print("[Lua]    ⚠️ 包加载失败（可能文件不存在），跳过后续测试")
        return
    end

    -- 3. 获取文件数量
    local count = Package.getFileCount()
    print("[Lua] 3. 包内文件数量: " .. count)

    -- 4. 列出所有资源
    print("[Lua] 4. 包内资源列表:")
    local list = Package.list()
    for i, name in ipairs(list) do
        print(string.format("      [%d] %s", i, name))
    end

    -- 5. 检查特定资源是否存在
    print("[Lua] 5. 检查资源存在性:")
    print("      has('main.lua') = " .. tostring(Package.has("main.lua")))
    print("      has('scene1.lua') = " .. tostring(Package.has("scene1.lua")))
    print("      has('nonexistent.txt') = " .. tostring(Package.has("nonexistent.txt")))

    -- 6. 卸载包
    print("[Lua] 6. 卸载资源包")
    Package.unload()
    print("      isLoaded = " .. tostring(Package.isLoaded()))

    print("[Lua] === 资源打包系统测试完成 ===")
end

return runPackageDemo
