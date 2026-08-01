#include "localization.h"
#include <filesystem>
#include <algorithm>

namespace fs = std::filesystem;

const std::string Localization::EMPTY_STRING = "";

Localization::Localization() {}

Localization::~Localization() {}

std::string Localization::trim(const std::string& str) {
    size_t start = str.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    size_t end = str.find_last_not_of(" \t\r\n");
    return str.substr(start, end - start + 1);
}

bool Localization::loadLanguage(const std::string& langCode) {
    std::string filePath = m_localeDir + "/" + langCode + ".lang";
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "Localization: Failed to load language file: " << filePath << std::endl;
        return false;
    }

    auto& langData = m_languages[langCode];
    langData.clear();

    std::string line;
    int lineNum = 0;
    while (std::getline(file, line)) {
        lineNum++;
        line = trim(line);
        if (line.empty() || line[0] == '#' || line[0] == ';') continue;

        size_t eqPos = line.find('=');
        if (eqPos == std::string::npos) continue;

        std::string key = trim(line.substr(0, eqPos));
        std::string value = trim(line.substr(eqPos + 1));

        // 处理转义字符
        std::string unescaped;
        for (size_t i = 0; i < value.size(); i++) {
            if (value[i] == '\\' && i + 1 < value.size()) {
                char next = value[i + 1];
                if (next == 'n') { unescaped += '\n'; i++; }
                else if (next == 't') { unescaped += '\t'; i++; }
                else if (next == '\\') { unescaped += '\\'; i++; }
                else unescaped += value[i];
            } else {
                unescaped += value[i];
            }
        }

        langData[key] = unescaped;
    }

    file.close();
    std::cout << "Localization: Loaded language '" << langCode
              << "' with " << langData.size() << " entries." << std::endl;
    return true;
}

void Localization::unloadLanguage(const std::string& langCode) {
    m_languages.erase(langCode);
    if (m_currentLang == langCode) {
        m_currentLang.clear();
    }
}

void Localization::unloadAll() {
    m_languages.clear();
    m_currentLang.clear();
}

bool Localization::setLanguage(const std::string& langCode) {
    if (m_languages.find(langCode) == m_languages.end()) {
        // 尝试自动加载
        if (!loadLanguage(langCode)) {
            return false;
        }
    }
    m_currentLang = langCode;
    std::cout << "Localization: Switched to language '" << langCode << "'" << std::endl;
    return true;
}

std::vector<std::string> Localization::getLoadedLanguages() const {
    std::vector<std::string> langs;
    langs.reserve(m_languages.size());
    for (const auto& [code, data] : m_languages) {
        langs.push_back(code);
    }
    return langs;
}

const std::string& Localization::translate(const std::string& key) const {
    auto langIt = m_languages.find(m_currentLang);
    if (langIt == m_languages.end()) {
        return EMPTY_STRING;
    }

    auto keyIt = langIt->second.find(key);
    if (keyIt == langIt->second.end()) {
        return EMPTY_STRING;
    }

    return keyIt->second;
}

std::string Localization::translateFormat(const std::string& key,
                                          const std::vector<std::string>& args) const {
    const std::string& text = translate(key);
    if (text.empty()) return key;  // 返回 key 作为 fallback

    std::string result;
    for (size_t i = 0; i < text.size(); i++) {
        if (text[i] == '{' && i + 1 < text.size()) {
            size_t closePos = text.find('}', i + 1);
            if (closePos != std::string::npos) {
                std::string indexStr = text.substr(i + 1, closePos - i - 1);
                try {
                    int index = std::stoi(indexStr);
                    if (index >= 0 && index < static_cast<int>(args.size())) {
                        result += args[index];
                    } else {
                        result += text.substr(i, closePos - i + 1);
                    }
                } catch (...) {
                    result += text.substr(i, closePos - i + 1);
                }
                i = closePos;
                continue;
            }
        }
        result += text[i];
    }

    return result;
}

bool Localization::hasTranslation(const std::string& key) const {
    auto langIt = m_languages.find(m_currentLang);
    if (langIt == m_languages.end()) return false;
    return langIt->second.find(key) != langIt->second.end();
}

void Localization::addTranslation(const std::string& langCode,
                                  const std::string& key,
                                  const std::string& value) {
    m_languages[langCode][key] = value;
}

bool Localization::saveLanguage(const std::string& langCode) const {
    auto langIt = m_languages.find(langCode);
    if (langIt == m_languages.end()) return false;

    std::string filePath = m_localeDir + "/" + langCode + ".lang";
    std::ofstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "Localization: Failed to save language file: " << filePath << std::endl;
        return false;
    }

    file << "# Language: " << langCode << std::endl;
    file << "# Auto-generated" << std::endl;
    file << std::endl;

    for (const auto& [key, value] : langIt->second) {
        // 转义换行符
        std::string escaped;
        for (char c : value) {
            if (c == '\n') escaped += "\\n";
            else if (c == '\t') escaped += "\\t";
            else if (c == '\\') escaped += "\\\\";
            else escaped += c;
        }
        file << key << "=" << escaped << std::endl;
    }

    file.close();
    return true;
}

const std::unordered_map<std::string, std::string>& Localization::getAllTranslations() const {
    static const std::unordered_map<std::string, std::string> EMPTY_MAP;
    auto langIt = m_languages.find(m_currentLang);
    if (langIt == m_languages.end()) return EMPTY_MAP;
    return langIt->second;
}

std::vector<std::string> Localization::getAvailableLanguages() const {
    std::vector<std::string> langs;

    if (!fs::exists(m_localeDir)) return langs;

    for (const auto& entry : fs::directory_iterator(m_localeDir)) {
        if (!entry.is_regular_file()) continue;
        std::string ext = entry.path().extension().string();
        if (ext == ".lang") {
            std::string stem = entry.path().stem().string();
            langs.push_back(stem);
        }
    }

    std::sort(langs.begin(), langs.end());
    return langs;
}
