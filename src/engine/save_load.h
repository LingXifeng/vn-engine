#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <ctime>
#include "script_engine.h"

// 存档槽信息
struct SaveSlot {
    int slot = 0;
    std::string title;          // 存档标题（通常是当前对话）
    std::string timestamp;       // 存档时间
    std::string scriptName;      // 当前脚本名
    int scriptLine = 0;          // 当前脚本行
    std::unordered_map<std::string, std::string> variables;  // 游戏变量
    bool empty = true;
};

// 存档系统
class SaveLoad {
public:
    SaveLoad(ScriptEngine* script);
    ~SaveLoad();

    // 设置存档目录
    void setSaveDir(const std::string& dir) { m_saveDir = dir; }

    // 保存/加载
    bool save(int slot, const std::string& title, const std::string& scriptName, int scriptLine);
    bool load(int slot);
    bool deleteSave(int slot);

    // 获取存档列表
    std::vector<SaveSlot> getSaveList();
    SaveSlot getSaveInfo(int slot);
    bool hasSave(int slot);

    // 自动存档
    bool autoSave(const std::string& scriptName, int scriptLine);
    bool loadAutoSave();

    // 快速存档
    bool quickSave(const std::string& scriptName, int scriptLine);
    bool quickLoad();

private:
    ScriptEngine* m_script;
    std::string m_saveDir = "saves";

    std::string getSavePath(int slot) const;
    std::string getCurrentTime() const;

    // 简单的键值对序列化
    std::string serialize(const SaveSlot& slot);
    bool deserialize(const std::string& data, SaveSlot& slot);
};
