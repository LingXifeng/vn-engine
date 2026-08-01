#include "save_load.h"
#include <iostream>
#include <filesystem>
#include <cstring>

SaveLoad::SaveLoad(ScriptEngine* script) : m_script(script) {}
SaveLoad::~SaveLoad() {}

std::string SaveLoad::getSavePath(int slot) const {
    return m_saveDir + "/save_" + std::to_string(slot) + ".dat";
}

std::string SaveLoad::getCurrentTime() const {
    time_t now = time(nullptr);
    struct tm* t = localtime(&now);
    char buf[64];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", t);
    return std::string(buf);
}

std::string SaveLoad::serialize(const SaveSlot& s) {
    std::ostringstream oss;
    oss << "VN_SAVE_V1\n";
    oss << "slot=" << s.slot << "\n";
    oss << "title=" << s.title << "\n";
    oss << "timestamp=" << s.timestamp << "\n";
    oss << "script=" << s.scriptName << "\n";
    oss << "line=" << s.scriptLine << "\n";
    oss << "[variables]\n";
    for (const auto& [key, value] : s.variables) {
        oss << key << "=" << value << "\n";
    }
    oss << "[end]\n";
    return oss.str();
}

bool SaveLoad::deserialize(const std::string& data, SaveSlot& s) {
    std::istringstream iss(data);
    std::string line;
    bool inVariables = false;

    if (!std::getline(iss, line) || line != "VN_SAVE_V1") return false;

    while (std::getline(iss, line)) {
        if (line == "[variables]") { inVariables = true; continue; }
        if (line == "[end]") break;

        size_t eq = line.find('=');
        if (eq == std::string::npos) continue;
        std::string key = line.substr(0, eq);
        std::string value = line.substr(eq + 1);

        if (inVariables) {
            s.variables[key] = value;
        } else {
            if (key == "slot") s.slot = std::stoi(value);
            else if (key == "title") s.title = value;
            else if (key == "timestamp") s.timestamp = value;
            else if (key == "script") s.scriptName = value;
            else if (key == "line") s.scriptLine = std::stoi(value);
        }
    }
    s.empty = false;
    return true;
}

bool SaveLoad::save(int slot, const std::string& title,
                    const std::string& scriptName, int scriptLine) {
    // 确保目录存在
    std::filesystem::create_directories(m_saveDir);

    SaveSlot s;
    s.slot = slot;
    s.title = title;
    s.timestamp = getCurrentTime();
    s.scriptName = scriptName;
    s.scriptLine = scriptLine;
    s.empty = false;

    // 收集 Lua 全局变量（简化：只保存已知变量）
    // 实际实现中可以遍历 Lua 全局表

    std::string data = serialize(s);
    std::ofstream file(getSavePath(slot));
    if (!file) {
        std::cerr << "Failed to write save file: " << getSavePath(slot) << std::endl;
        return false;
    }
    file << data;
    file.close();
    return true;
}

bool SaveLoad::load(int slot) {
    std::ifstream file(getSavePath(slot));
    if (!file) return false;

    std::stringstream ss;
    ss << file.rdbuf();
    file.close();

    SaveSlot s;
    if (!deserialize(ss.str(), s)) return false;

    // 恢复 Lua 变量
    for (const auto& [key, value] : s.variables) {
        // 尝试解析为数字
        try {
            double num = std::stod(value);
            m_script->setGlobal(key, LuaValue(num));
        } catch (...) {
            m_script->setGlobal(key, LuaValue(value));
        }
    }

    std::cout << "[Load] slot " << slot << ": " << s.title
              << " (" << s.timestamp << ")" << std::endl;
    return true;
}

bool SaveLoad::deleteSave(int slot) {
    return std::remove(getSavePath(slot).c_str()) == 0;
}

std::vector<SaveSlot> SaveLoad::getSaveList() {
    std::vector<SaveSlot> list;
    for (int i = 0; i < 20; i++) {  // 最多 20 个存档槽
        SaveSlot s = getSaveInfo(i);
        if (!s.empty) list.push_back(s);
    }
    return list;
}

SaveSlot SaveLoad::getSaveInfo(int slot) {
    SaveSlot s;
    s.slot = slot;
    s.empty = true;

    std::ifstream file(getSavePath(slot));
    if (!file) return s;

    std::stringstream ss;
    ss << file.rdbuf();
    file.close();

    deserialize(ss.str(), s);
    return s;
}

bool SaveLoad::hasSave(int slot) {
    std::ifstream file(getSavePath(slot));
    return file.good();
}

bool SaveLoad::autoSave(const std::string& scriptName, int scriptLine) {
    return save(99, "Auto Save", scriptName, scriptLine);  // slot 99 = auto
}

bool SaveLoad::loadAutoSave() {
    return load(99);
}

bool SaveLoad::quickSave(const std::string& scriptName, int scriptLine) {
    return save(98, "Quick Save", scriptName, scriptLine);  // slot 98 = quick
}

bool SaveLoad::quickLoad() {
    return load(98);
}
