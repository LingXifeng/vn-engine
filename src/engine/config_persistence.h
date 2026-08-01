#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>

// 配置持久化系统
// 保存/加载引擎和游戏配置到 INI 格式文件
class ConfigPersistence {
public:
    ConfigPersistence();
    ~ConfigPersistence();

    // 设置配置文件路径
    void setConfigPath(const std::string& path) { m_configPath = path; }
    const std::string& getConfigPath() const { return m_configPath; }

    // 加载配置文件
    bool load();
    // 保存配置文件
    bool save();

    // 通用键值操作
    void setString(const std::string& key, const std::string& value);
    void setInt(const std::string& key, int value);
    void setFloat(const std::string& key, float value);
    void setBool(const std::string& key, bool value);

    std::string getString(const std::string& key, const std::string& defaultValue = "") const;
    int getInt(const std::string& key, int defaultValue = 0) const;
    float getFloat(const std::string& key, float defaultValue = 0.0f) const;
    bool getBool(const std::string& key, bool defaultValue = false) const;

    // 检查键是否存在
    bool hasKey(const std::string& key) const;
    // 删除键
    void removeKey(const std::string& key);
    // 清空所有配置
    void clear();

    // 获取所有键
    std::vector<std::string> getKeys() const;

    // 批量设置默认值（仅当键不存在时设置）
    void setDefaults(const std::unordered_map<std::string, std::string>& defaults);

    // 引擎配置快捷方法
    void setWindowConfig(int width, int height, bool fullscreen);
    void getWindowConfig(int& width, int& height, bool& fullscreen);

    void setAudioConfig(int masterVol, int bgmVol, int voiceVol, int seVol);
    void getAudioConfig(int& masterVol, int& bgmVol, int& voiceVol, int& seVol);

    void setTextSpeed(int speed);  // 0=瞬間, 1=遅い, 2=普通, 3=速い
    int getTextSpeed() const;

    void setLanguage(const std::string& lang);
    std::string getLanguage() const;

private:
    std::string m_configPath = "config.ini";
    std::unordered_map<std::string, std::string> m_data;

    // INI 解析/序列化
    std::string trim(const std::string& str);
    std::string escapeValue(const std::string& value);
    std::string unescapeValue(const std::string& value);
};
