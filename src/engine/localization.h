#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>

// 多语言支持系统 (i18n)
// 支持加载多语言文本文件，运行时切换语言
class Localization {
public:
    Localization();
    ~Localization();

    // 设置语言文件目录
    void setLocaleDir(const std::string& dir) { m_localeDir = dir; }
    const std::string& getLocaleDir() const { return m_localeDir; }

    // 加载语言文件 (格式: key=value 每行一个)
    bool loadLanguage(const std::string& langCode);
    // 卸载语言
    void unloadLanguage(const std::string& langCode);
    // 卸载所有语言
    void unloadAll();

    // 切换当前语言
    bool setLanguage(const std::string& langCode);
    const std::string& getCurrentLanguage() const { return m_currentLang; }

    // 获取已加载的语言列表
    std::vector<std::string> getLoadedLanguages() const;

    // 翻译文本 (核心方法)
    // key 格式: "section.key" 或 "key"
    const std::string& translate(const std::string& key) const;

    // 带参数的翻译 (支持 {0}, {1}, ... 占位符)
    std::string translateFormat(const std::string& key,
                                const std::vector<std::string>& args) const;

    // 检查翻译是否存在
    bool hasTranslation(const std::string& key) const;

    // 添加/更新翻译条目 (运行时动态添加)
    void addTranslation(const std::string& langCode,
                        const std::string& key,
                        const std::string& value);

    // 保存语言文件
    bool saveLanguage(const std::string& langCode) const;

    // 获取当前语言的所有键值对
    const std::unordered_map<std::string, std::string>& getAllTranslations() const;

    // 便捷方法
    // 获取可用语言列表（从目录扫描）
    std::vector<std::string> getAvailableLanguages() const;

private:
    std::string m_localeDir = "locales";
    std::string m_currentLang = "ja";  // 默认日语

    // langCode -> (key -> value)
    std::unordered_map<std::string, std::unordered_map<std::string, std::string>> m_languages;

    // 空字符串引用（用于未找到翻译时返回）
    static const std::string EMPTY_STRING;

    std::string trim(const std::string& str);
};
