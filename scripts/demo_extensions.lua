-- ============================================================
-- 拡張機能デモスクリプト
-- 5個の新機能: Video, Config, Window, I18n, Expression
-- ============================================================

System.log("=== 拡張機能デモ開始 ===")

-- ----------------------------------------------------------
-- 1. Config — 設定の保存・読み込み
-- ----------------------------------------------------------
System.log("[1] Config 永続化テスト")

-- 設定を書き込む
Config.setString("game.title", "VN Engine Demo")
Config.setInt("game.version", 100)
Config.setBool("game.debug", true)
Config.setWindow(1280, 720, false)
Config.setAudio(80, 100, 90, 70)
Config.setLanguage("ja")

-- 保存
local saved = Config.save()
System.log("  Config saved: " .. tostring(saved))

-- 読み込み
Config.setString("game.title", "")  -- クリア
local loaded = Config.load()
System.log("  Config loaded: " .. tostring(loaded))

-- 読み込んだ値を確認
local title = Config.getString("game.title", "default")
System.log("  game.title = " .. title)

local w, h, fs = Config.getWindow()
System.log("  window = " .. w .. "x" .. h .. " fullscreen=" .. tostring(fs))

local master, bgm, voice, se = Config.getAudio()
System.log("  audio = master:" .. master .. " bgm:" .. bgm .. " voice:" .. voice .. " se:" .. se)

System.log("")

-- ----------------------------------------------------------
-- 2. I18n — 多言語対応
-- ----------------------------------------------------------
System.log("[2] i18n 多言語テスト")

-- 翻訳を動的追加
I18n.add("ja", "greeting", "こんにちは")
I18n.add("ja", "farewell", "さようなら")
I18n.add("en", "greeting", "Hello")
I18n.add("en", "farewell", "Goodbye")
I18n.add("zh", "greeting", "你好")
I18n.add("zh", "farewell", "再见")

-- 日本語に切替
I18n.setLanguage("ja")
local ja_greeting = I18n.translate("greeting")
System.log("  ja greeting: " .. ja_greeting)

-- 英語に切替
I18n.setLanguage("en")
local en_greeting = I18n.translate("greeting")
System.log("  en greeting: " .. en_greeting)

-- 中国語に切替
I18n.setLanguage("zh")
local zh_greeting = I18n.translate("greeting")
System.log("  zh greeting: " .. zh_greeting)

-- 元に戻す
I18n.setLanguage("ja")
System.log("  current language: " .. I18n.getLanguage())

System.log("")

-- ----------------------------------------------------------
-- 3. Window — ウィンドウ情報
-- ----------------------------------------------------------
System.log("[3] Window ウィンドウテスト")

local isFs = Window.isFullscreen()
System.log("  fullscreen: " .. tostring(isFs))

local rw, rh = Window.getResolution()
System.log("  resolution: " .. rw .. "x" .. rh)

-- 利用可能な解像度一覧
local resos = Window.getAvailableResolutions()
System.log("  available resolutions: " .. #resos)
for i = 1, math.min(#resos, 5) do
    System.log("    " .. resos[i].label .. ": " .. resos[i].width .. "x" .. resos[i].height)
end

System.log("")

-- ----------------------------------------------------------
-- 4. Expression — 表情差分（APIテスト）
-- ----------------------------------------------------------
System.log("[4] Expression 表情差分テスト")

-- 利用可能な表情一覧（空のはず）
local exprs = Expression.getAvailable()
System.log("  available expressions: " .. #exprs)

-- 現在の表情
local current = Expression.getCurrent()
System.log("  current expression: " .. current)

-- まばたき設定
Expression.setBlinking(true, 3.0)
System.log("  blinking enabled")

-- リップシンク設定
Expression.setLipSync(true)
System.log("  lip sync enabled")

-- 位置設定
Expression.setPosition(640.0, 360.0)
System.log("  position set to (640, 360)")

System.log("")

-- ----------------------------------------------------------
-- 5. Video — 動画再生（APIテスト）
-- ----------------------------------------------------------
System.log("[5] Video 動画再生テスト")

-- ループ設定
Video.setLoop(true)
System.log("  loop: " .. tostring(true))

-- 再生速度
Video.setSpeed(1.0)
System.log("  speed: 1.0")

-- 描画領域
Video.setRegion(0, 0, 1280, 720)
System.log("  region: 0,0,1280,720")

-- 再生状態確認
local playing = Video.isPlaying()
System.log("  isPlaying: " .. tostring(playing))

System.log("")

-- ----------------------------------------------------------
-- 完了
-- ----------------------------------------------------------
System.log("=== 拡張機能デモ完了 ===")
