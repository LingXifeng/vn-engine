cmake_minimum_required(VERSION 3.16)
project(VNEngine VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# ============================================================================
# 平台检测
# ============================================================================
if(WIN32)
    message(STATUS "平台: Windows")
    set(VN_PLATFORM_WINDOWS ON)
else()
    message(STATUS "平台: Unix/Linux")
    set(VN_PLATFORM_WINDOWS OFF)
endif()

# ============================================================================
# 依赖查找 — Linux 用 pkg-config，Windows 用 find_package (vcpkg)
# ============================================================================
if(VN_PLATFORM_WINDOWS)
    # Windows: 使用 vcpkg 安装的库，通過 find_package 查找
    find_package(SDL2 CONFIG REQUIRED)
    find_package(SDL2_image CONFIG REQUIRED)
    find_package(SDL2_ttf CONFIG REQUIRED)
    find_package(SDL2_mixer CONFIG REQUIRED)

    # Lua: 嘗試 find_package，失敗則手動查找
    find_package(Lua 5.4 QUIET)
    if(NOT LUA_FOUND)
        find_package(lua CONFIG QUIET)
        if(NOT lua_FOUND)
            # 手動查找
            find_path(LUA_INCLUDE_DIR lua.h PATH_SUFFIXES lua)
            find_library(LUA_LIBRARY NAMES lua lua54)
            if(LUA_INCLUDE_DIR AND LUA_LIBRARY)
                set(LUA_FOUND TRUE)
            endif()
        endif()
    endif()
    if(NOT LUA_FOUND AND NOT lua_FOUND)
        message(FATAL_ERROR "Lua 5.4 not found. Install via vcpkg: vcpkg install lua")
    endif()

    # 統一變量名（映射到與 pkg-config 相同的變量）
    set(SDL2_INCLUDE_DIRS "")
    set(SDL2_LIBRARIES SDL2::SDL2)
    set(SDL2_IMAGE_INCLUDE_DIRS "")
    set(SDL2_IMAGE_LIBRARIES SDL2_image::SDL2_image)
    set(SDL2_TTF_INCLUDE_DIRS "")
    set(SDL2_TTF_LIBRARIES SDL2_ttf::SDL2_ttf)
    set(SDL2_MIXER_INCLUDE_DIRS "")
    set(SDL2_MIXER_LIBRARIES SDL2_mixer::SDL2_mixer)
    if(TARGET Lua::Lua)
        set(LUA_LIBRARIES Lua::Lua)
    elseif(TARGET lua::lua)
        set(LUA_LIBRARIES lua::lua)
    elseif(LUA_LIBRARY)
        set(LUA_LIBRARIES ${LUA_LIBRARY})
    endif()
    if(LUA_INCLUDE_DIR)
        set(LUA_INCLUDE_DIRS ${LUA_INCLUDE_DIR})
    endif()
else()
    # Linux/macOS: 使用 pkg-config
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(SDL2 REQUIRED sdl2)
    pkg_check_modules(SDL2_IMAGE REQUIRED SDL2_image)
    pkg_check_modules(SDL2_TTF REQUIRED SDL2_ttf)
    pkg_check_modules(SDL2_MIXER REQUIRED SDL2_mixer)
    # Lua: 不同发行版的 pkg-config 模块名不同（lua, lua5.4, lua5.3）
    pkg_check_modules(LUA QUIET lua)
    if(NOT LUA_FOUND)
        pkg_check_modules(LUA QUIET lua5.4)
    endif()
    if(NOT LUA_FOUND)
        pkg_check_modules(LUA QUIET lua5.3)
    endif()
    if(NOT LUA_FOUND)
        pkg_check_modules(LUA QUIET lua5.2)
    endif()
    if(NOT LUA_FOUND)
        message(FATAL_ERROR "Lua not found via pkg-config. Tried: lua, lua5.4, lua5.3, lua5.2. Install e.g. liblua5.3-dev or liblua5.4-dev")
    endif()
endif()

# ============================================================================
# 全局包含目錄
# ============================================================================
include_directories(
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/engine
    ${CMAKE_SOURCE_DIR}/src/script
    ${CMAKE_SOURCE_DIR}/src/adv
    ${CMAKE_SOURCE_DIR}/src/ui
    ${SDL2_INCLUDE_DIRS}
    ${SDL2_IMAGE_INCLUDE_DIRS}
    ${SDL2_TTF_INCLUDE_DIRS}
    ${SDL2_MIXER_INCLUDE_DIRS}
    ${LUA_INCLUDE_DIRS}
)

# ============================================================================
# 引擎源文件
# ============================================================================
set(ENGINE_SOURCES
    src/engine/engine.cpp
    src/engine/renderer.cpp
    src/engine/audio.cpp
    src/engine/input.cpp
    src/engine/resource_manager.cpp
    src/engine/tween.cpp
    src/engine/save_load.cpp
    src/engine/particle.cpp
    src/engine/video_player.cpp
    src/engine/config_persistence.cpp
    src/engine/window_resize.cpp
    src/engine/localization.cpp
    src/engine/character_expression.cpp
    src/engine/resource_package.cpp
    src/script/script_engine.cpp
    src/script/lua_bindings.cpp
    src/adv/text_box.cpp
    src/adv/character.cpp
    src/adv/scene.cpp
    src/ui/title_screen.cpp
    src/ui/config_screen.cpp
    src/ui/cg_gallery.cpp
    src/ui/rich_text.cpp
    src/ui/music_room.cpp
    src/ui/voice_gallery.cpp
    src/ui/credits.cpp
    src/ui/flowchart.cpp
    src/ui/save_load_ui.cpp
    src/ui/backlog.cpp
    src/ui/auto_skip.cpp
    src/ui/transition.cpp
    src/ui/background_animation.cpp
    src/ui/ending_list.cpp
    src/ui/scene_replay.cpp
    src/ui/glossary.cpp
)

# ============================================================================
# 主引擎目標
# ============================================================================
add_executable(vn_engine src/main.cpp ${ENGINE_SOURCES})

target_link_libraries(vn_engine
    ${SDL2_LIBRARIES}
    ${SDL2_IMAGE_LIBRARIES}
    ${SDL2_TTF_LIBRARIES}
    ${SDL2_MIXER_LIBRARIES}
    ${LUA_LIBRARIES}
)

if(NOT VN_PLATFORM_WINDOWS)
    target_link_directories(vn_engine PRIVATE
        ${SDL2_LIBRARY_DIRS}
        ${SDL2_IMAGE_LIBRARY_DIRS}
        ${SDL2_TTF_LIBRARY_DIRS}
        ${SDL2_MIXER_LIBRARY_DIRS}
        ${LUA_LIBRARY_DIRS}
    )
endif()

# Windows: 複製 DLL 到輸出目錄
if(VN_PLATFORM_WINDOWS)
    add_custom_command(TARGET vn_engine POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:SDL2::SDL2> $<TARGET_FILE_DIR:vn_engine>
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:SDL2_image::SDL2_image> $<TARGET_FILE_DIR:vn_engine>
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:SDL2_ttf::SDL2_ttf> $<TARGET_FILE_DIR:vn_engine>
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:SDL2_mixer::SDL2_mixer> $<TARGET_FILE_DIR:vn_engine>
    )
endif()

# Copy scripts to build directory
add_custom_command(TARGET vn_engine POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/scripts ${CMAKE_BINARY_DIR}/bin/scripts
)

# ============================================================================
# 資源打包工具
# ============================================================================
add_executable(pak_tool tools/pak_tool.cpp ${CMAKE_SOURCE_DIR}/src/engine/resource_package.cpp)
target_include_directories(pak_tool PRIVATE ${CMAKE_SOURCE_DIR}/src/engine)
set_target_properties(pak_tool PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# ============================================================================
# VN 可視化編輯器
# ============================================================================
set(EDITOR_SOURCES
    tools/vn_editor/vn_editor.cpp
    tools/vn_editor/editor_project.cpp
    tools/vn_editor/lua_exporter.cpp
)

# 檢查是否有顯示環境（無頭模式只編譯 CLI）
option(ENABLE_EDITOR_GUI "啟用編輯器 GUI 模式（需要 SDL2 顯示）" ON)

if(ENABLE_EDITOR_GUI)
    list(APPEND EDITOR_SOURCES tools/vn_editor/editor_ui.cpp)
endif()

add_executable(vn_editor ${EDITOR_SOURCES})

target_include_directories(vn_editor PRIVATE
    ${CMAKE_SOURCE_DIR}/tools/vn_editor
    ${CMAKE_SOURCE_DIR}/src/engine
    ${SDL2_INCLUDE_DIRS}
    ${SDL2_TTF_INCLUDE_DIRS}
    ${LUA_INCLUDE_DIRS}
)

if(ENABLE_EDITOR_GUI)
    target_include_directories(vn_editor PRIVATE
        ${SDL2_IMAGE_INCLUDE_DIRS}
    )
    target_compile_definitions(vn_editor PRIVATE USE_GUI)
    target_link_libraries(vn_editor
        ${SDL2_LIBRARIES}
        ${SDL2_IMAGE_LIBRARIES}
        ${SDL2_TTF_LIBRARIES}
        ${LUA_LIBRARIES}
    )
    if(NOT VN_PLATFORM_WINDOWS)
        target_link_directories(vn_editor PRIVATE
            ${SDL2_LIBRARY_DIRS}
            ${SDL2_IMAGE_LIBRARY_DIRS}
            ${SDL2_TTF_LIBRARY_DIRS}
            ${LUA_LIBRARY_DIRS}
        )
    endif()
else()
    target_link_libraries(vn_editor
        ${LUA_LIBRARIES}
    )
    if(NOT VN_PLATFORM_WINDOWS)
        target_link_directories(vn_editor PRIVATE
            ${LUA_LIBRARY_DIRS}
        )
    endif()
endif()

set_target_properties(vn_editor PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Windows: 編編譯時隱藏控制台窗口（GUI 模式）
if(VN_PLATFORM_WINDOWS AND ENABLE_EDITOR_GUI)
    set_target_properties(vn_editor PROPERTIES WIN32_EXECUTABLE TRUE)
endif()
