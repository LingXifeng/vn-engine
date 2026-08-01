# VN Engine Lua API 参考手册

> VN Engine（C++/SDL2/Lua）Lua 脚本 API 参考文档  
> 面向游戏创作者 — 只需编写 Lua 脚本即可制作视觉小说

---

## 目录

1. [入门指南](#1-入门指南)
2. [基本结构](#2-基本结构)
3. [Text — 文本与对话](#3-text--文本与对话)
4. [Character — 角色](#4-character--角色)
5. [Scene — 背景与淡入淡出](#5-scene--背景与淡入淡出)
6. [Audio — 音频](#6-audio--音频)
7. [Transition — 画面转场](#7-transition--画面转场)
8. [Particle — 粒子特效](#8-particle--粒子特效)
9. [BgEffect — 背景动画](#9-bgeffect--背景动画)
10. [RichText — 富文本](#10-richtext--富文本)
11. [Gallery — CG 画廊](#11-gallery--cg-画廊)
12. [MusicRoom — 音乐室](#12-musicroom--音乐室)
13. [Credits — 制作人员名单](#13-credits--制作人员名单)
14. [SaveLoad — 存档/读档界面](#14-saveload--存档读档界面)
15. [Backlog — 历史记录](#15-backlog--历史记录)
16. [AutoSkip — 自动/跳过](#16-autoskip--自动跳过)
17. [flowchart — 流程图](#17-flowchart--流程图)
18. [Ending — 结局列表](#18-ending--结局列表)
19. [SceneReplay — 场景回放](#19-scenereplay--场景回放)
20. [Glossary — 术语词典](#20-glossary--术语词典)
21. [Tween — 补间动画/等待](#21-tween--补间动画等待)
22. [System — 系统功能](#22-system--系统功能)
23. [附录: 全 API 速查表](#23-附录-全-api-速查表)

---

## 1. 入门指南

VN Engine 使用 **Lua 5.4** 脚本编写游戏逻辑。
引擎侧（C++）负责渲染、音频、输入处理，Lua 侧负责剧情与数据定义。

### 脚本运行方式

```bash
./vn_engine --script scripts/my_game.lua
```

### 关于协程

`Text.say()`、`Text.narrate()` 等需要等待玩家输入的函数会
**在协程中 yield**。引擎会自动将 `main()` 函数作为协程启动，
因此通常无需手动处理协程问题。

---

## 2. 基本结构

```lua
-- 游戏状态（用全局变量管理）
game = {
    chapter = 0,
    flags = {},
}

-- 初始化函数（数据注册）
function initGame()
    System.setTitle("我的游戏")
    Ending.add("good", "完美结局")
    -- ...
end

-- 各章节函数
function chapter1()
    Transition.fade(1.0)
    Text.narrate("故事开始了…")
    Text.say("美咲", "你好呀！")
end

-- 主函数（引擎会将其作为协程启动）
function main()
    initGame()
    chapter1()
    System.quit()
end
```

---

## 3. Text — 文本与对话

对话显示、旁白、选项等文本相关 API。

### Text.say(name, text)

显示角色台词，等待玩家点击。

| 参数 | 类型 | 必填 | 说明 |
|------|------|------|------|
| name | string | ✓ | 角色名 |
| text | string | ✓ | 台词文本 |

```lua
Text.say("美咲", "今天天气真好。")
Text.say("主角", "{b}有件重要的事{/b}想告诉你。")
```

> **yield** — 等待玩家点击

---

### Text.narrate(text)

显示旁白文字，等待点击。

| 参数 | 类型 | 必填 | 说明 |
|------|------|------|------|
| text | string | ✓ | 旁白文本 |

```lua
Text.narrate("—— 樱花飘落的季节 ——")
Text.narrate("两人的故事就此开始。")
```

> **yield** — 等待玩家点击

---

### Text.clear()

清除当前文本显示。

```lua
Text.clear()
```

---

### Text.setColor(r, g, b)

设置文本颜色。

| 参数 | 类型 | 必填 | 默认值 | 说明 |
|------|------|------|--------|------|
| r | int | ✓ | — | 红色分量 (0-255) |
| g | int | ✓ | — | 绿色分量 (0-255) |
| b | int | ✓ | — | 蓝色分量 (0-255) |

```lua
Text.setColor(255, 200, 200)  -- 淡粉色
Text.say("美咲", "呵呵，谢谢你。")
Text.setColor(255, 255, 255)  -- 恢复白色
```

---

### Text.setSpeed(speed)

设置文本显示速度。

| 参数 | 类型 | 必填 | 默认值 | 说明 |
|------|------|------|--------|------|
| speed | float | ✓ | — | 显示速度（字/秒） |

```lua
Text.setSpeed(30.0)  -- 每秒30字
```

---

### Text.choice(options) → int

显示选项，等待玩家选择。

| 参数 | 类型 | 必填 | 说明 |
|------|------|------|------|
| options | table | ✓ | 选项字符串数组 |

**返回值:** 选中项的索引（从1开始）

```lua
local result = Text.choice({
    "试着问问",
    "默默陪在身边",
    "去买冰淇淋"
})

if result == 1 then
    Text.say("主角", "你在想什么呢？")
elseif result == 2 then
    Text.say("主角", "……")
else
    Text.say("主角", "去买冰淇淋吧！")
end
```

> **yield** — 等待玩家选择

---

## 4. Character — 角色

角色的显示、隐藏、表情切换。

### Character.show(name, expression?, position?, fade?)

在画面上显示角色。

| 参数 | 类型 | 必填 | 默认值 | 说明 |
|------|------|------|--------|------|
| name | string | ✓ | — | 角色标识名 |
| expression | string | | `"default"` | 表情名 |
| position | string | | `"left"` | 位置: `"left"` / `"center"` / `"right"` |
| fade | float | | `0.3` | 淡入时间（秒） |

```lua
Character.show("misaki", "smile", "center", 0.5)
Character.show("hero", "normal", "right")
```

---

### Character.hide(name, fade?)

隐藏角色。

| 参数 | 类型 | 必填 | 默认值 | 说明 |
|------|------|------|--------|------|
| name | string | ✓ | — | 角色标识名 |
| fade | float | | `0.3` | 淡出时间（秒） |

```lua
Character.hide("misaki", 0.5)
```

---

### Character.setExpression(name, expression)

切换已显示角色的表情。

| 参数 | 类型 | 必填 | 说明 |
|------|------|------|------|
| name | string | ✓ | 角色标识名 |
| expression | string | ✓ | 表情名 |

```lua
Character.setExpression("misaki", "surprised")
Text.say("美咲", "诶！？")
Character.setExpression("misaki", "smile")
Text.say("美咲", "没事，没什么。")
```

---

### Character.move()

> 目前为预留接口，未来版本将实现角色移动动画。

---

## 5. Scene — 背景与淡入淡出

背景图片设置与画面淡入淡出。

### Scene.setBackground(path, fade?)

设置背景图片。

| 参数 | 类型 | 必填 | 默认值 | 说明 |
|------|------|------|--------|------|
| path | string | ✓ | — | 图片文件路径 |
| fade | float | | `0.0` | 淡入时间（秒） |

```lua
Scene.setBackground("bg/street_day.png", 0.5)
```

---

### Scene.clearBackground()

清除背景。

```lua
Scene.clearBackground()
```

---

### Scene.fadeIn(duration?) / Scene.fadeOut(duration?)

画面整体淡入/淡出。

| 参数 | 类型 | 必填 | 默认值 | 说明 |
|------|------|------|--------|------|
| duration | float | | `0.5` | 淡入淡出时间（秒） |

```lua
Scene.fadeOut(1.0)
Scene.setBackground("bg/park.png")
Scene.fadeIn(1.0)
```

---

## 6. Audio — 音频

BGM、音效、语音的播放控制。

### Audio.playBGM(path, loop?, fade?)

播放 BGM。

| 参数 | 类型 | 必填 | 默认值 | 说明 |
|------|------|------|--------|------|
| path | string | ✓ | — | 音频文件路径 |
| loop | int | | — | 循环次数（-1 为无限循环） |
| fade | int | | `0` | 淡入时间（毫秒） |

```lua
Audio.playBGM("bgm/opening.ogg", -1)  -- 无限循环
```

---

### Audio.stopBGM()

停止 BGM 播放。

```lua
Audio.stopBGM()
```

---

### Audio.playSE(path)

播放音效（SE）。

| 参数 | 类型 | 必填 | 说明 |
|------|------|------|------|
| path | string | ✓ | 音频文件路径 |

```lua
Audio.playSE("se/click.ogg")
```

---

### Audio.playVoice(path)

播放语音。

| 参数 | 类型 | 必填 | 说明 |
|------|------|------|------|
| path | string | ✓ | 音频文件路径 |

```lua
Audio.playVoice("voice/misaki_01.ogg")
```

---

### Audio.setVolume(type, volume)

设置音量。

| 参数 | 类型 | 必填 | 说明 |
|------|------|------|------|
| type | string | ✓ | `"bgm"` / `"se"` / `"voice"` / `"master"` |
| volume | int | ✓ | 音量 (0-100) |

```lua
Audio.setVolume("bgm", 80)
Audio.setVolume("voice", 100)
Audio.setVolume("master", 90)
```

---

## 7. Transition — 画面转场

画面转场特效（淡入淡出、滑动、溶解等）。

### Transition.fade(duration?)

黑色淡入淡出转场。

| 参数 | 类型 | 必填 | 默认值 | 说明 |
|------|------|------|--------|------|
| duration | float | | `0.5` | 转场时间（秒） |

```lua
Transition.fade(1.0)
```

> **yield** — 等待转场完成

---

### Transition.fadeWhite(duration?)

白色淡入淡出转场。

| 参数 | 类型 | 必填 | 默认值 | 说明 |
|------|------|------|--------|------|
| duration | float | | `0.5` | 转场时间（秒） |

```lua
Transition.fadeWhite(0.8)
```

> **yield**

---

### Transition.slide(direction?, duration?)

滑动转场。

| 参数 | 类型 | 必填 | 默认值 | 说明 |
|------|------|------|--------|------|
| direction | string | | `"left"` | 方向: `"left"` / `"right"` / `"up"` / `"down"` |
| duration | float | | `0.5` | 转场时间（秒） |

```lua
Transition.slide("right", 0.6)
```

> **yield**

---

### Transition.dissolve(duration?)

溶解（交叉淡入淡出）转场。

| 参数 | 类型 | 必填 | 默认值 | 说明 |
|------|------|------|--------|------|
| duration | float | | `0.5` | 转场时间（秒） |

```lua
Transition.dissolve(1.0)
```

> **yield**

---

### Transition.blind(duration?)

百叶窗转场。

| 参数 | 类型 | 必填 | 默认值 | 说明 |
|------|------|------|--------|------|
| duration | float | | `0.5` | 转场时间（秒） |

```lua
Transition.blind(0.7)
```

> **yield**

---

### Transition.mosaic(duration?)

马赛克转场。

| 参数 | 类型 | 必填 | 默认值 | 说明 |
|------|------|------|--------|------|
| duration | float | | `0.5` | 转场时间（秒） |

```lua
Transition.mosaic(0.8)
```

> **yield**

---

## 8. Particle — 粒子特效

雪、雨、樱花、光点等画面特效。

### Particle.create(type, count?, speed?)

创建粒子特效。

| 参数 | 类型 | 必填 | 默认值 | 说明 |
|------|------|------|--------|------|
| type | string | ✓ | — | 特效类型（见下方） |
| count | int | | `100` | 粒子数量 |
| speed | float | | `1.0` | 速度倍率 |

**特效类型:** `"snow"`, `"rain"`, `"sakura"`, `"light"`, `"sparkle"`

```lua
Particle.create("sakura", 200, 1.5)
```

---

### Particle.setType(type)

切换正在运行的粒子特效类型。

| 参数 | 类型 | 必填 | 说明 |
|------|------|------|------|
| type | string | ✓ | 特效类型 |

```lua
Particle.setType("snow")
```

---

### Particle.setParam(key, value)

设置粒子参数。

| 参数 | 类型 | 必填 | 说明 |
|------|------|------|------|
| key | string | ✓ | 参数名 |
| value | float | ✓ | 参数值 |

```lua
Particle.setParam("gravity", 0.5)
Particle.setParam("wind", -0.3)
```

---

### Particle.start() / Particle.stop() / Particle.clear()

启动、停止、清除粒子特效。

```lua
Particle.start()
Particle.stop()
Particle.clear()
```

---

## 9. BgEffect — 背景动画

天气效果与画面震动。

### BgEffect.setWeather(type?, intensity?)

设置天气效果。

| 参数 | 类型 | 必填 | 默认值 | 说明 |
|------|------|------|--------|------|
| type | string | | `"none"` | 天气类型 |
| intensity | float | | `1.0` | 强度 (0.0-1.0) |

**天气类型:** `"none"`, `"rain"`, `"snow"`, `"storm"`, `"fog"`

```lua
BgEffect.setWeather("rain", 0.8)
-- ...
BgEffect.setWeather("none")  -- 清除天气
```

---

### BgEffect.shake(intensity?, duration?)

画面震动效果。

| 参数 | 类型 | 必填 | 默认值 | 说明 |
|------|------|------|--------|------|
| intensity | float | | `5.0` | 震动强度（像素） |
| duration | float | | `0.3` | 持续时间（秒） |

```lua
BgEffect.shake(10.0, 0.5)  -- 强烈震动
```

---

### BgEffect.stopShake()

停止画面震动。

```lua
BgEffect.stopShake()
```

---

## 10. RichText — 富文本

注音、粗体、阴影、字间距等富文本功能。

### RichText.setRuby(base, ruby)

设置注音（假名标注）。

| 参数 | 类型 | 必填 | 说明 |
|------|------|------|------|
| base | string | ✓ | 基础文字 |
| ruby | string | ✓ | 注音文字 |

```lua
RichText.setRuby("魔法", "まほう")
```

---

### RichText.setBold(enable)

开启/关闭粗体显示。

| 参数 | 类型 | 必填 | 说明 |
|------|------|------|------|
| enable | bool | ✓ | true 为粗体 |

```lua
RichText.setBold(true)
```

---

### RichText.setShadow(r, g, b, offset)

为文本添加阴影。

| 参数 | 类型 | 必填 | 说明 |
|------|------|------|------|
| r | int | ✓ | 阴影红色分量 (0-255) |
| g | int | ✓ | 阴影绿色分量 (0-255) |
| b | int | ✓ | 阴影蓝色分量 (0-255) |
| offset | int | ✓ | 阴影偏移（像素） |

```lua
RichText.setShadow(0, 0, 0, 2)  -- 黑色阴影
```

---

### RichText.setSpacing(lineSpacing, charSpacing)

设置行间距和字间距。

| 参数 | 类型 | 必填 | 说明 |
|------|------|------|------|
| lineSpacing | float | ✓ | 行间距（像素） |
| charSpacing | float | ✓ | 字间距（像素） |

```lua
RichText.setSpacing(8.0, 2.0)
```

---

### RichText.render(text, x, y, maxWidth)

在指定位置渲染富文本。

| 参数 | 类型 | 必填 | 说明 |
|------|------|------|------|
| text | string | ✓ | 文本内容 |
| x | int | ✓ | X 坐标 |
| y | int | ✓ | Y 坐标 |
| maxWidth | int | ✓ | 最大宽度（像素） |

```lua
RichText.render("{b}重要{/b}通知", 100, 200, 600)
```

---

## 11. Gallery — CG 画廊

CG 解锁状态管理与画廊界面。

### Gallery.unlock(id)

解锁 CG。

| 参数 | 类型 | 必填 | 说明 |
|------|------|------|------|
| id | string | ✓ | CG 标识 ID |

```lua
Gallery.unlock("cg_01")
Gallery.unlock("cg_event_kiss")
```

---

### Gallery.isUnlocked(id) → bool

检查 CG 是否已解锁。

| 参数 | 类型 | 必填 | 说明 |
|------|------|------|------|
| id | string | ✓ | CG 标识 ID |

**返回值:** 已解锁返回 `true`

```lua
if Gallery.isUnlocked("cg_01") then
    Text.narrate("这张 CG 之前已经看过了。")
end
```

---

### Gallery.show()

显示 CG 画廊界面。

```lua
Gallery.show()
```

---

### Gallery.loadData(ids) / Gallery.saveData() → table

保存/加载解锁状态。

```lua
-- 加载
Gallery.loadData({"cg_01", "cg_02", "cg_03"})

-- 保存
local unlocked = Gallery.saveData()
-- → {"cg_01", "cg_02", ...}
```

---

## 12. MusicRoom — 音乐室

BGM 试听功能。

### MusicRoom.add(id, title, artist, path)

向试听列表添加乐曲。

| 参数 | 类型 | 必填 | 说明 |
|------|------|------|------|
| id | string | ✓ | 乐曲标识 ID |
| title | string | ✓ | 乐曲标题 |
| artist | string | ✓ | 艺术家名 |
| path | string | ✓ | 音频文件路径 |

```lua
MusicRoom.add("bgm_01", "樱花季节", "山田太郎", "bgm/opening.ogg")
MusicRoom.add("bgm_02", "雨后初晴", "佐藤花子", "bgm/rain.ogg")
```

---

### MusicRoom.show()

显示音乐室界面。

```lua
MusicRoom.show()
```

---

### MusicRoom.play(path) / MusicRoom.stop()

播放/停止乐曲。

```lua
MusicRoom.play("bgm/opening.ogg")
MusicRoom.stop()
```

---

## 13. Credits — 制作人员名单

结局后的制作人员滚动字幕。

### Credits.addTitle(text) / Credits.addHeading(text) / Credits.addName(text) / Credits.addSmall(text)

向字幕添加各类文本。

| 参数 | 类型 | 必填 | 说明 |
|------|------|------|------|
| text | string | ✓ | 显示文本 |

```lua
Credits.addTitle("我的游戏")
Credits.addHeading("— 制作团队 —")
Credits.addName("剧本: 山田太郎")
Credits.addName("插画: 佐藤花子")
Credits.addSmall("特别感谢: 你")
```

---

### Credits.loadFile(path) → bool

从文件加载制作人员名单。

| 参数 | 类型 | 必填 | 说明 |
|------|------|------|------|
| path | string | ✓ | 文件路径 |

**返回值:** 加载成功返回 `true`

```lua
if Credits.loadFile("credits/staff.txt") then
    Credits.show()
end
```

---

### Credits.show() / Credits.hide()

显示/隐藏制作人员名单。

```lua
Credits.show()
-- ...
Credits.hide()
```

---

### Credits.setSpeed(speed)

设置字幕滚动速度。

| 参数 | 类型 | 必填 | 说明 |
|------|------|------|------|
| speed | float | ✓ | 滚动速度 |

```lua
Credits.setSpeed(50.0)
```

---

## 14. SaveLoad — 存档/读档界面

存档/读档界面控制。

### SaveLoad.showSave() / SaveLoad.showLoad()

显示存档/读档界面。

```lua
SaveLoad.showSave()
SaveLoad.showLoad()
```

---

### SaveLoad.hide()

隐藏存档/读档界面。

```lua
SaveLoad.hide()
```

---

### SaveLoad.setSlotCount(count)

设置存档槽位数量。

| 参数 | 类型 | 必填 | 说明 |
|------|------|------|------|
| count | int | ✓ | 槽位数量 |

```lua
SaveLoad.setSlotCount(20)
```

---

## 15. Backlog — 历史记录

已读文本的历史记录管理与显示。

### Backlog.add(speaker?, text, voice?)

向历史记录添加条目。

| 参数 | 类型 | 必填 | 默认值 | 说明 |
|------|------|------|--------|------|
| speaker | string | | `""` | 说话者名 |
| text | string | ✓ | — | 文本内容 |
| voice | string | | `""` | 语音文件路径 |

```lua
Backlog.add("美咲", "今天天气真好。", "voice/misaki_01.ogg")
Backlog.add("", "—— 樱花飘落的季节 ——")  -- 旁白
```

> 通常 `Text.say()` / `Text.narrate()` 会自动添加到历史记录。
> 此函数用于需要手动添加的情况。

---

### Backlog.show() / Backlog.hide()

显示/隐藏历史记录界面。

```lua
Backlog.show()
Backlog.hide()
```

---

### Backlog.clear()

清空历史记录。

```lua
Backlog.clear()
```

---

## 16. AutoSkip — 自动/跳过

自动模式与跳过模式控制。

### AutoSkip.setAuto(enable) / AutoSkip.setSkip(enable)

启用/禁用自动/跳过模式。

| 参数 | 类型 | 必填 | 说明 |
|------|------|------|------|
| enable | bool | ✓ | true 为启用 |

```lua
AutoSkip.setAuto(true)   -- 开启自动模式
AutoSkip.setSkip(true)   -- 开启跳过模式
```

---

### AutoSkip.toggleAuto() / AutoSkip.toggleSkip()

切换自动/跳过模式的开/关状态。

```lua
AutoSkip.toggleAuto()
AutoSkip.toggleSkip()
```

---

### AutoSkip.setAutoSpeed(speed)

设置自动模式的推进速度。

| 参数 | 类型 | 必填 | 说明 |
|------|------|------|------|
| speed | float | ✓ | 速度（字/秒） |

```lua
AutoSkip.setAutoSpeed(40.0)
```

---

### AutoSkip.stopAll()

同时停止自动和跳过模式。

```lua
AutoSkip.stopAll()
```

---

## 17. flowchart — 流程图

故事结构可视化与进度管理。

### flowchart.addNode(id, label, type, scriptLabel?, description?)

向流程图添加节点。

| 参数 | 类型 | 必填 | 说明 |
|------|------|------|------|
| id | string | ✓ | 节点标识 ID |
| label | string | ✓ | 显示标签 |
| type | string | ✓ | 节点类型 |
| scriptLabel | string | | 对应的脚本标签 |
| description | string | | 节点描述 |

**节点类型:** `"start"`, `"chapter"`, `"scene"`, `"choice"`, `"ending"`

```lua
flowchart.addNode("ch1", "第一章: 开始", "chapter", "chapter1", "故事的开端")
flowchart.addNode("ch1_s1", "相遇", "scene", "scene_meet", "与她的初次相遇")
flowchart.addNode("end_good", "完美结局", "ending", "ending_good")
```

---

### flowchart.addEdge(fromId, toId, label?)

添加节点间的边（转移关系）。

| 参数 | 类型 | 必填 | 默认值 | 说明 |
|------|------|------|--------|------|
| fromId | string | ✓ | — | 起始节点 ID |
| toId | string | ✓ | — | 目标节点 ID |
| label | string | | `""` | 边标签 |

```lua
flowchart.addEdge("ch1", "ch1_s1")
flowchart.addEdge("ch1_s1", "end_good", "好的选择")
```

---

### flowchart.setPosition(id, x, y)

设置节点的显示位置。

| 参数 | 类型 | 必填 | 说明 |
|------|------|------|------|
| id | string | ✓ | 节点 ID |
| x | float | ✓ | X 坐标 |
| y | float | ✓ | Y 坐标 |

```lua
flowchart.setPosition("ch1", 100, 200)
```

---

### flowchart.markVisited(id) / flowchart.markCurrent(id)

将节点标记为"已访问"/"当前位置"。

| 参数 | 类型 | 必填 | 说明 |
|------|------|------|------|
| id | string | ✓ | 节点 ID |

```lua
flowchart.markVisited("ch1")
flowchart.markCurrent("ch1_s1")
```

---

### flowchart.autoLayout()

自动布局节点。

```lua
flowchart.autoLayout()
```

---

### flowchart.show() / flowchart.hide() / flowchart.resetView()

显示/隐藏流程图界面、重置视图。

```lua
flowchart.show()
flowchart.resetView()
flowchart.hide()
```

---

### flowchart.getVisited() → table / flowchart.getEndings() → table

获取已访问节点/结局节点的 ID 列表。

```lua
local visited = flowchart.getVisited()  -- {"ch1", "ch1_s1", ...}
local endings = flowchart.getEndings()  -- {"end_good", ...}
```

---

## 18. Ending — 结局列表

结局的注册与解锁状态管理。

### Ending.add(id, title, description?, condition?)

注册结局。

| 参数 | 类型 | 必填 | 默认值 | 说明 |
|------|------|------|--------|------|
| id | string | ✓ | — | 结局标识 ID |
| title | string | ✓ | — | 结局标题 |
| description | string | | `""` | 描述文本 |
| condition | string | | `""` | 解锁条件说明 |

```lua
Ending.add("good", "完美结局", "所有幸福的结局", "美咲好感度MAX")
Ending.add("normal", "普通结局", "平凡的结局")
Ending.add("bad", "坏结局", "悲伤的结局", "选择失误")
```

---

### Ending.unlock(id)

解锁结局。

| 参数 | 类型 | 必填 | 说明 |
|------|------|------|------|
| id | string | ✓ | 结局 ID |

```lua
Ending.unlock("good")
```

---

### Ending.isUnlocked(id) → bool

检查结局是否已解锁。

```lua
if Ending.isUnlocked("good") then
    Text.narrate("已经看过完美结局了。")
end
```

---

### Ending.getCompletion() → float

获取结局完成率。

**返回值:** 完成率 (0.0-1.0)

```lua
local rate = Ending.getCompletion()
Text.narrate(string.format("完成率: %d%%", rate * 100))
```

---

## 19. SceneReplay — 场景回放

特定场景的重新播放功能。

### SceneReplay.add(id, title, description?, scriptName?, startLine?, endLine?, chapter?)

注册可回放的场景。

| 参数 | 类型 | 必填 | 默认值 | 说明 |
|------|------|------|--------|------|
| id | string | ✓ | — | 场景标识 ID |
| title | string | ✓ | — | 场景标题 |
| description | string | | `""` | 描述文本 |
| scriptName | string | | `""` | 脚本文件名 |
| startLine | int | | `0` | 起始行号 |
| endLine | int | | `0` | 结束行号 |
| chapter | string | | `""` | 章节名 |

```lua
SceneReplay.add("scene_meet", "初次相遇", "与她的第一次相遇",
                 "chapter1.lua", 10, 50, "第一章")
SceneReplay.add("scene_confess", "告白", "重要的场景",
                 "chapter3.lua", 80, 120, "第三章")
```

---

### SceneReplay.unlock(id)

解锁场景回放。

```lua
SceneReplay.unlock("scene_meet")
```

---

### SceneReplay.show() / SceneReplay.hide()

显示/隐藏场景回放界面。

```lua
SceneReplay.show()
SceneReplay.hide()
```

---

## 20. Glossary — 术语词典

游戏内术语的词典功能。

### Glossary.add(term, description, category?, reading?)

向词典添加术语。

| 参数 | 类型 | 必填 | 默认值 | 说明 |
|------|------|------|--------|------|
| term | string | ✓ | — | 术语 |
| description | string | ✓ | — | 描述文本 |
| category | string | | `""` | 分类 |
| reading | string | | `""` | 读音 |

```lua
Glossary.add("魔法", "这个世界中可使用的不思议之力。", "术语", "まほう")
Glossary.add("精灵", "寄宿于自然中的意志存在。", "种族", "せいれい")
Glossary.add("聖都", "故事的舞台所在的都市。", "地名", "えと")
```

---

### Glossary.unlock(term)

解锁术语使其在词典中可查看。

```lua
Glossary.unlock("魔法")
```

---

### Glossary.show() / Glossary.hide()

显示/隐藏术语词典界面。

```lua
Glossary.show()
Glossary.hide()
```

---

### Glossary.showTermDetail(term)

显示特定术语的详情。

```lua
Glossary.showTermDetail("魔法")
```

---

## 21. Tween — 补间动画/等待

动画控制与等待功能。

### Tween.start(duration)

启动指定时长的补间动画。

| 参数 | 类型 | 必填 | 说明 |
|------|------|------|------|
| duration | float | ✓ | 时间（秒） |

```lua
Tween.start(1.0)
```

> **yield** — 等待补间完成

---

### Tween.wait(duration)

等待指定时长。

| 参数 | 类型 | 必填 | 说明 |
|------|------|------|------|
| duration | float | ✓ | 等待时间（秒） |

```lua
Tween.wait(2.0)  -- 等待2秒
```

> **yield** — 等待指定时间经过

---

## 22. System — 系统功能

日志输出、屏幕信息、标题设置等系统功能。

### System.log(message)

向控制台输出日志。

| 参数 | 类型 | 必填 | 说明 |
|------|------|------|------|
| message | string | ✓ | 日志消息 |

```lua
System.log("游戏开始")
```

---

### System.getScreenSize() → int, int

获取屏幕尺寸。

**返回值:** 宽度, 高度

```lua
local w, h = System.getScreenSize()
System.log(string.format("屏幕尺寸: %dx%d", w, h))
```

---

### System.setTitle(title)

设置窗口标题。

| 参数 | 类型 | 必填 | 说明 |
|------|------|------|------|
| title | string | ✓ | 标题文本 |

```lua
System.setTitle("我的游戏 — 第一章")
```

---

### System.quit()

退出引擎。

```lua
System.quit()
```

---

### System.save(slot) / System.load(slot)

执行存档/读档。

| 参数 | 类型 | 必填 | 说明 |
|------|------|------|------|
| slot | int | ✓ | 存档槽位号 |

```lua
System.save(1)
System.load(1)
```

---

## 23. 附录: 全 API 速查表

### yield 函数一览（等待玩家输入的函数）

以下函数在协程中 yield，等待玩家输入或时间经过。
请在 `main()` 函数内调用。

| 函数 | 等待条件 |
|------|---------|
| `Text.say(name, text)` | 等待点击 |
| `Text.narrate(text)` | 等待点击 |
| `Text.choice(options)` | 等待选择 |
| `Transition.fade(duration?)` | 转场完成 |
| `Transition.fadeWhite(duration?)` | 转场完成 |
| `Transition.slide(dir?, duration?)` | 转场完成 |
| `Transition.dissolve(duration?)` | 转场完成 |
| `Transition.blind(duration?)` | 转场完成 |
| `Transition.mosaic(duration?)` | 转场完成 |
| `Tween.start(duration)` | 补间完成 |
| `Tween.wait(duration)` | 指定时间经过 |

### 全 API 快速参考

```
-- Text
Text.say(name, text)
Text.narrate(text)
Text.clear()
Text.setColor(r, g, b)
Text.setSpeed(speed)
Text.choice(options) → int

-- Character
Character.show(name, expression?, position?, fade?)
Character.hide(name, fade?)
Character.setExpression(name, expression)
Character.move()

-- Scene
Scene.setBackground(path, fade?)
Scene.clearBackground()
Scene.fadeIn(duration?)
Scene.fadeOut(duration?)

-- Audio
Audio.playBGM(path, loop?, fade?)
Audio.stopBGM()
Audio.playSE(path)
Audio.playVoice(path)
Audio.setVolume(type, volume)

-- Transition
Transition.fade(duration?)
Transition.fadeWhite(duration?)
Transition.slide(direction?, duration?)
Transition.dissolve(duration?)
Transition.blind(duration?)
Transition.mosaic(duration?)

-- Particle
Particle.create(type, count?, speed?)
Particle.setType(type)
Particle.setParam(key, value)
Particle.start()
Particle.stop()
Particle.clear()

-- BgEffect
BgEffect.setWeather(type?, intensity?)
BgEffect.shake(intensity?, duration?)
BgEffect.stopShake()

-- RichText
RichText.setRuby(base, ruby)
RichText.setBold(enable)
RichText.setShadow(r, g, b, offset)
RichText.setSpacing(lineSpacing, charSpacing)
RichText.render(text, x, y, maxWidth)

-- Gallery
Gallery.unlock(id)
Gallery.isUnlocked(id) → bool
Gallery.show()
Gallery.loadData(ids)
Gallery.saveData() → table

-- MusicRoom
MusicRoom.add(id, title, artist, path)
MusicRoom.show()
MusicRoom.play(path)
MusicRoom.stop()

-- Credits
Credits.addTitle(text)
Credits.addHeading(text)
Credits.addName(text)
Credits.addSmall(text)
Credits.loadFile(path) → bool
Credits.show()
Credits.hide()
Credits.setSpeed(speed)

-- SaveLoad
SaveLoad.showSave()
SaveLoad.showLoad()
SaveLoad.hide()
SaveLoad.setSlotCount(count)

-- Backlog
Backlog.add(speaker?, text, voice?)
Backlog.show()
Backlog.hide()
Backlog.clear()

-- AutoSkip
AutoSkip.setAuto(enable)
AutoSkip.setSkip(enable)
AutoSkip.toggleAuto()
AutoSkip.toggleSkip()
AutoSkip.setAutoSpeed(speed)
AutoSkip.stopAll()

-- flowchart
flowchart.addNode(id, label, type, scriptLabel?, description?)
flowchart.addEdge(fromId, toId, label?)
flowchart.setPosition(id, x, y)
flowchart.markVisited(id)
flowchart.markCurrent(id)
flowchart.autoLayout()
flowchart.show()
flowchart.hide()
flowchart.resetView()
flowchart.getVisited() → table
flowchart.getEndings() → table

-- Ending
Ending.add(id, title, description?, condition?)
Ending.unlock(id)
Ending.isUnlocked(id) → bool
Ending.getCompletion() → float

-- SceneReplay
SceneReplay.add(id, title, description?, scriptName?, startLine?, endLine?, chapter?)
SceneReplay.unlock(id)
SceneReplay.show()
SceneReplay.hide()

-- Glossary
Glossary.add(term, description, category?, reading?)
Glossary.unlock(term)
Glossary.show()
Glossary.hide()
Glossary.showTermDetail(term)

-- Tween
Tween.start(duration)
Tween.wait(duration)

-- System
System.log(message)
System.getScreenSize() → int, int
System.setTitle(title)
System.quit()
System.save(slot)
System.load(slot)
```

---

### 富文本标签

在 `Text.say()` / `Text.narrate()` / `RichText.render()` 的文本中可使用的标签:

| 标签 | 说明 | 示例 |
|------|------|------|
| `{b}...{/b}` | 粗体 | `{b}重要{/b}` |
| `{i}...{/i}` | 斜体 | `{i}强调{/i}` |
| `{color=RRGGBB}...{/color}` | 颜色指定 | `{color=FF0000}红色文字{/color}` |
| `{ruby=注音}文字{/ruby}` | 注音标注 | `{ruby=まほう}魔法{/ruby}` |
| `{size=N}...{/size}` | 字号指定 | `{size=24}大字{/size}` |

---

> **VN Engine Lua API 参考手册** — 基礎API（22个API表，80+个函数）
> 版本 1.0 | C++17 / SDL2 / Lua 5.4


---

## 拡張機能 API（5個の新機能）

### Video — 動画再生（FMV）

動画の読み込み、再生制御、ループ設定、描画領域指定などを行う。
内部的にはフレーム画像シーケンス方式で動画を再生する。

```lua
Video.load(path, fps)           -- 動画を読み込む（fps省略時30）
Video.play()                    -- 再生開始
Video.pause()                   -- 一時停止
Video.stop()                    -- 停止
Video.isPlaying()               -- 再生中か判定 → bool
Video.setLoop(loop)             -- ループ再生設定（true/false）
Video.setRegion(x, y, w, h)     -- 描画領域を設定
Video.setSpeed(speed)           -- 再生速度（0.5～2.0）
Video.seek(frame)               -- 指定フレームへシーク
Video.setAudio(path)            -- 動画に同期して再生する音声パスを設定
```

**使用例:**
```lua
Video.load("videos/op/", 30)
Video.setLoop(true)
Video.setRegion(0, 0, 1280, 720)
Video.play()
-- 動画再生中の待機ループ
while Video.isPlaying() do
    Flow.wait(0.016)
end
Video.stop()
```

---

### Config — 設定永続化

エンジンおよびゲームの設定をINI形式ファイルに保存・読み込みする。
ウィンドウ設定、音量、テキスト速度、言語設定などを永続化できる。

```lua
Config.load()                   -- 設定ファイルを読み込む → bool
Config.save()                   -- 設定ファイルを保存 → bool
Config.setString(key, value)    -- 文字列を設定
Config.setInt(key, value)       -- 整数を設定
Config.setBool(key, value)      -- 真偽値を設定
Config.getString(key, default)  -- 文字列を取得 → string
Config.getInt(key, default)     -- 整数を取得 → int
Config.getBool(key, default)    -- 真偽値を取得 → bool
Config.setWindow(w, h, fs)      -- ウィンドウ設定を一括保存
Config.getWindow()              -- ウィンドウ設定を取得 → w, h, fs
Config.setAudio(master, bgm, voice, se)  -- 音量設定を一括保存
Config.getAudio()               -- 音量設定を取得 → master, bgm, voice, se
Config.setLanguage(lang)        -- 言語設定を保存
Config.getLanguage()            -- 言語設定を取得 → string
```

**使用例:**
```lua
-- 起動時に設定を読み込む
Config.load()

-- 設定変更
Config.setWindow(1920, 1080, true)
Config.setAudio(80, 100, 90, 70)
Config.setLanguage("ja")

-- 保存
Config.save()
```

---

### Window — ウィンドウ拡大・全画面

ウィンドウの全画面切替、解像度変更、描画スケール設定などを行う。

```lua
Window.toggleFullscreen()       -- 全画面切替
Window.setFullscreen(fs)        -- 全画面設定（true/false）
Window.isFullscreen()           -- 全画面か判定 → bool
Window.setResolution(w, h)      -- 解像度を設定
Window.getResolution()          -- 解像度を取得 → w, h
Window.getAvailableResolutions() -- 利用可能解像度リストを取得 → table
Window.setRenderScale(scale)    -- 描画スケールを設定
Window.maximize()               -- ウィンドウ最大化
Window.center()                 -- ウィンドウを中央に配置
```

**使用例:**
```lua
-- 全画面切替
Window.toggleFullscreen()

-- 解像度変更
Window.setResolution(1920, 1080)

-- 利用可能な解像度一覧を表示
local resos = Window.getAvailableResolutions()
for i, r in ipairs(resos) do
    System.log(r.label .. ": " .. r.width .. "x" .. r.height)
end
```

---

### I18n — 多言語対応（i18n）

多言語テキストの読み込み、言語切替、翻訳取得などを行う。
言語ファイルは `key=value` 形式のテキストファイル。

```lua
I18n.load(langCode)             -- 言語ファイルを読み込む → bool
I18n.setLanguage(langCode)      -- 現在の言語を切替 → bool
I18n.getLanguage()              -- 現在の言語コードを取得 → string
I18n.translate(key)             -- 翻訳テキストを取得 → string
I18n.translateFormat(key, ...)  -- パラメータ付き翻訳 → string
I18n.has(key)                   -- 翻訳が存在するか判定 → bool
I18n.add(lang, key, value)      -- 翻訳を動的追加
I18n.getAvailable()             -- 読み込み済み言語リスト → table
```

**使用例:**
```lua
-- 言語ファイルを読み込む
I18n.load("ja")
I18n.load("en")
I18n.load("zh")

-- 設定から言語を切替
local lang = Config.getLanguage()
I18n.setLanguage(lang)

-- 翻訳テキストを取得
local title = I18n.translate("menu.title")
local msg = I18n.translateFormat("dialog.save_confirm", "スロット1")
```

**言語ファイル例（locales/ja.txt）:**
```
menu.title=メインメニュー
menu.new_game=ニューゲーム
menu.load=ロード
dialog.save_confirm={0}に保存しますか？
```

---

### Expression — キャラクター表情差分

基礎立絵に表情差分レイヤーを重ね合わせるシステム。
差分ブレンドモード（Alpha/加算/減算/上書き）、過渡アニメーション、
まばたき、リップシンクに対応。

```lua
Expression.setBase(path)        -- 基礎立絵を設定
Expression.addDiff(name, path, blend, offX, offY)  -- 表情差分を追加
Expression.set(name, duration)  -- 表情を切替（過渡アニメ付き）
Expression.setInstant(name)     -- 表情を瞬間切替
Expression.getCurrent()         -- 現在の表情名を取得 → string
Expression.getAvailable()       -- 利用可能な表情リスト → table
Expression.setVisible(name, vis) -- 差分の表示/非表示
Expression.setAlpha(name, alpha) -- 差分の透明度を設定
Expression.setPosition(x, y)    -- キャラクターの位置を設定
Expression.setBlinking(enabled, interval) -- まばたき設定
Expression.setLipSync(enabled)  -- リップシンク設定
```

**ブレンドモード値:**
| 値 | モード | 説明 |
|----|--------|------|
| 0  | ALPHA  | Alpha混合（通常） |
| 1  | ADD    | 加算（光効果） |
| 2  | SUBTRACT | 減算 |
| 3  | OVERWRITE | 上書き |

**使用例:**
```lua
-- 基礎立絵と表情差分を設定
Expression.setBase("char/hero/base.png")
Expression.addDiff("smile", "char/hero/smile.png", 0, 0, 0)
Expression.addDiff("angry", "char/hero/angry.png", 0, 0, 0)
Expression.addDiff("surprised", "char/hero/surprised.png", 0, 0, 0)

-- 表情切替（0.3秒の過渡アニメ）
Expression.set("smile", 0.3)

-- まばたき有効
Expression.setBlinking(true, 3.0)

-- リップシンク有効（音声再生時）
Expression.setLipSync(true)
```

---

## 编辑器与导出器 API（vn_editor）

### 编辑器项目数据模型

编辑器使用独立的项目数据模型，与引擎运行时分离。项目文件以 Lua 表格式存储。

**数据结构:**

| 结构 | 字段 | 类型 | 说明 |
|------|------|------|------|
| EditorProject | name | string | 项目名 |
| EditorProject | author | string | 作者 |
| EditorProject | version | string | 版本号 |
| EditorProject | startScene | string | 起始场景ID |
| EditorProject | characters | EditorCharacter[] | 角色列表 |
| EditorProject | scenes | EditorScene[] | 场景列表 |
| EditorProject | resources | EditorResource[] | 资源列表 |
| EditorScene | id | string | 场景ID（唯一） |
| EditorScene | name | string | 场景显示名 |
| EditorScene | background | string | 默认背景资源ID |
| EditorScene | bgm | string | 默认BGM资源ID |
| EditorScene | lines | EditorLine[] | 对话行列表 |
| EditorCharacter | id | string | 角色ID（唯一） |
| EditorCharacter | name | string | 显示名 |
| EditorCharacter | colorR/G/B | int | 名字颜色 (0-255) |
| EditorCharacter | expressions | string[] | 表情列表 |
| EditorCharacter | voicePrefix | string | 语音文件前缀 |
| EditorLine | type | LineType | 行类型 |
| EditorLine | text | string | 文本内容 |
| EditorLine | character | string | 角色ID（Say类型） |
| EditorLine | expression | string | 表情（Say类型） |
| EditorLine | resourceId | string | 资源ID（BG/BGM/SFX类型） |
| EditorLine | waitTime | float | 等待时间（Wait类型） |
| EditorLine | fadeTime | float | 淡入淡出时间（FadeIn/FadeOut类型） |
| EditorLine | choices | EditorChoice[] | 选项列表（Choice类型） |
| EditorLine | targetScene | string | 跳转目标（Goto类型） |
| EditorLine | label | string | 标签名（Label类型） |
| EditorLine | endingId/Title/Desc | string | 结局信息（Ending类型） |
| EditorChoice | text | string | 选项文本 |
| EditorChoice | nextScene | string | 跳转目标场景ID |

---

### 对话行类型（LineType）

| 类型 | 说明 | 导出的 Lua 代码 |
|------|------|----------------|
| Narrate | 旁白 | `Text.narrate("text")` |
| Say | 角色说话 | `Character.show(...)` + `Text.say("name", "text")` |
| Choice | 选项分支 | `Flow.branch({...})` + `Flow.getSelection()` |
| Goto | 跳转 | `targetScene()` |
| Wait | 等待 | `Flow.wait(seconds)` |
| BGM | 播放BGM | `Audio.playBGM("id")` |
| BGMStop | 停止BGM | `Audio.stopBGM()` |
| SFX | 音效 | `Audio.playSFX("id")` |
| BG | 背景图 | `Scene.changeBG("id")` |
| FadeIn | 淡入 | `Scene.fadeIn(seconds)` |
| FadeOut | 淡出 | `Scene.fadeOut(seconds)` |
| Ending | 结局 | `Ending.add(...)` + `Flow.endGame()` |
| Label | 标签 | `-- ::label::` |

---

### ProjectFile — 项目文件操作

| 方法 | 参数 | 返回值 | 说明 |
|------|------|--------|------|
| ProjectFile.load() | project, path | bool | 从 Lua 文件加载项目 |
| ProjectFile.save() | project, path | bool | 保存项目到 Lua 文件 |
| ProjectFile.createNew() | name | EditorProject | 创建新项目（含默认场景和旁白角色） |

**使用例:**
```lua
-- 项目文件格式示例（Lua 表）
-- 保存后的 .lua 文件可被编辑器重新加载
project = {
    name = "我的游戏",
    author = "作者名",
    version = "1.0",
    startScene = "scene1",
    characters = { {id="hero", name="主人公"} },
    scenes = { {id="scene1", name="场景1", lines={...}} }
}
```

---

### LuaExporter — Lua 脚本导出器

| 方法 | 参数 | 返回值 | 说明 |
|------|------|--------|------|
| LuaExporter.exportSingleFile() | project, outputPath | bool | 导出为单个 Lua 文件 |
| LuaExporter.exportProject() | project, outputDir | bool | 导出项目到目录（含 main.lua + project_info.txt） |

**导出流程:**
1. 生成文件头注释（项目名、作者、版本）
2. 生成全局状态 `game = {chapter=1, flags={}}`
3. 生成角色定义 `Character.define()` / `Character.addExpression()`
4. 为每个场景生成函数 `function scene_id() ... end`
5. 生成主函数 `function main() ... end`（含 `System.setTitle`、`Scene.fadeIn`、起始场景调用）

**使用例（命令行）:**
```bash
# CLI 模式导出
vn_editor --cli --open myproject.lua --export game.lua

# 运行导出的脚本
vn_engine --script game.lua
```

---

### vn_editor — 命令行参数

| 参数 | 说明 |
|------|------|
| `--new` | 新建项目并启动 GUI |
| `--open <path>` | 打开项目文件 (.lua) |
| `--save <path>` | 保存项目到文件 |
| `--export <path>` | 导出为 Lua 游戏脚本 |
| `--cli` | CLI 模式（无图形界面） |
| `--list` | 列出项目中的场景和角色 |
| `--validate` | 验证项目文件完整性 |
| `--help` | 显示帮助 |

**CLI 模式组合示例:**
```bash
# 新建 → 保存 → 导出
vn_editor --cli --new --save proj.lua --export game.lua

# 打开 → 验证 → 列出场景
vn_editor --cli --open proj.lua --validate --list

# 打开 → 导出
vn_editor --cli --open proj.lua --export game.lua
```

---

### vn_editor — GUI 编辑器操作

**菜单栏:**
| 按钮 | 功能 | 快捷键 |
|------|------|--------|
| 新建 | 创建新项目（弹出对话框输入项目名/作者） | Ctrl+N |
| 打开 | 打开项目文件 | — |
| 保存 | 保存当前项目 | Ctrl+S |
| 导出 | 导出 Lua 脚本（弹出对话框输入路径） | — |

**场景列表（左面板）:**
| 操作 | 说明 |
|------|------|
| 点击场景 | 选中场景，在中央面板显示其对话行 |
| + 添加 | 弹出对话框：输入场景ID/名/背景/BGM |
| - 删除 | 确认删除当前选中场景 |
| ↑ / ↓ | 上移/下移场景（重排序） |

**对话编辑器（中央面板）:**
| 操作 | 说明 |
|------|------|
| 场景属性 | 直接编辑场景ID/名/背景/BGM |
| 点击对话行 | 选中行，在右面板显示编辑器 |
| + 添加行 | 弹出对话框：选择行类型后添加 |
| - 删除行 | 删除当前选中行 |
| 上移/下移 | 调整对话行顺序 |

**属性面板（右面板）:**
| 区域 | 说明 |
|------|------|
| 项目信息 | 显示项目名/作者/版本/起始场景 |
| 场景统计 | 显示行数/说话/旁白/选项统计 |
| 行编辑 | 根据行类型显示对应编辑器（文本/角色/表情/选项/跳转/资源/等待/淡入淡出/结局/标签） |
| 角色管理 | 角色列表（可选中）+ 添加/删除按钮 + 选中角色的ID/名编辑 |

**行类型编辑器:**
| 行类型 | 可编辑字段 |
|--------|-----------|
| Say | 文本、角色ID、表情 |
| Narrate | 文本 |
| Choice | 选项列表（每项：文本 + 跳转目标场景） |
| Goto | 目标场景ID |
| BGM/SFX/BG | 资源ID |
| Wait | 等待时间（秒） |
| FadeIn/FadeOut | 淡入淡出时间（秒） |
| Ending | 结局ID、标题、描述 |
| Label | 标签名 |

---

### 项目验证规则

`--validate` 执行以下检查:

| 检查项 | 说明 |
|--------|------|
| 起始场景存在 | startScene 指向的场景ID必须存在 |
| 场景ID唯一 | 不允许重复的场景ID |
| 跳转目标存在 | Goto 行的 targetScene 必须指向已定义的场景 |
| 选项跳转存在 | Choice 选项的 nextScene 必须指向已定义的场景 |

---

> **VN Engine Lua API 参考手册** — 共 34 個の API 表，150+ 個の関数
> 版本 1.2 | C++17 / SDL2 / Lua 5.4
> 含编辑器/导出器 API（vn_editor v1.0）
