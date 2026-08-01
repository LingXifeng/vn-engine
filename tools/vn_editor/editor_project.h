#pragma once

#include <string>
#include <vector>
#include <unordered_map>

// ============================================================================
// VN 编辑器项目数据模型
// 项目文件使用 Lua 表格式，可通过 Lua 加载/保存
// ============================================================================

// 对话行类型
enum class LineType {
    Narrate,    // 旁白
    Say,        // 角色说话
    Choice,     // 选项分支
    Goto,       // 跳转
    Wait,       // 等待
    BGM,        // 背景音乐
    BGMStop,    // 停止BGM
    SFX,        // 音效
    BG,         // 背景图
    FadeIn,     // 淡入
    FadeOut,    // 淡出
    Ending,     // 结局
    Label       // 标签（用于跳转目标）
};

// 选项（用于 Choice 类型）
struct EditorChoice {
    std::string text;       // 选项文本
    std::string nextScene;  // 跳转目标场景
};

// 对话行
struct EditorLine {
    LineType type = LineType::Say;
    std::string text;           // 文本内容
    std::string character;      // 角色ID
    std::string expression;     // 表情
    std::string resourceId;     // 资源ID（背景/BGM/SFX）
    float waitTime = 0.0f;      // 等待时间
    float fadeTime = 0.0f;      // 淡入淡出时间
    std::vector<EditorChoice> choices;  // 选项列表
    std::string targetScene;    // 跳转目标
    std::string label;          // 标签名
    // 结局
    std::string endingId;
    std::string endingTitle;
    std::string endingDesc;
};

// 场景
struct EditorScene {
    std::string id;             // 场景ID（唯一）
    std::string name;           // 场景名（显示用）
    std::string background;     // 默认背景
    std::string bgm;            // 默认BGM
    std::vector<EditorLine> lines;  // 对话行列表
};

// 角色定义
struct EditorCharacter {
    std::string id;             // 角色ID（唯一）
    std::string name;           // 显示名
    int colorR = 255;           // 名字颜色
    int colorG = 255;
    int colorB = 255;
    std::vector<std::string> expressions;  // 表情列表
    std::string voicePrefix;    // 语音文件前缀
};

// 资源定义
struct EditorResource {
    std::string id;             // 资源ID
    std::string path;           // 文件路径
    std::string type;           // 类型: image/audio/font
};

// VN 编辑器项目
struct EditorProject {
    std::string name;           // 项目名
    std::string author;         // 作者
    std::string version;        // 版本
    std::string startScene;     // 起始场景ID

    std::vector<EditorCharacter> characters;
    std::vector<EditorScene> scenes;
    std::vector<EditorResource> resources;

    // 查找场景
    EditorScene* findScene(const std::string& id);
    const EditorScene* findScene(const std::string& id) const;

    // 查找角色
    EditorCharacter* findCharacter(const std::string& id);
    const EditorCharacter* findCharacter(const std::string& id) const;

    // 添加场景
    EditorScene& addScene(const std::string& id, const std::string& name);

    // 添加角色
    EditorCharacter& addCharacter(const std::string& id, const std::string& name);

    // 删除场景
    bool removeScene(const std::string& id);

    // 获取行类型名称
    static const char* lineTypeName(LineType type);
    static LineType lineTypeFromName(const std::string& name);
};

// 项目文件加载/保存（使用 Lua）
class ProjectFile {
public:
    // 从 Lua 文件加载项目
    static bool load(EditorProject& project, const std::string& path);

    // 保存项目到 Lua 文件
    static bool save(const EditorProject& project, const std::string& path);

    // 创建新项目
    static EditorProject createNew(const std::string& name);
};
