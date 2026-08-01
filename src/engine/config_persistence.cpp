#include "config_persistence.h"

ConfigPersistence::ConfigPersistence() {}

ConfigPersistence::~ConfigPersistence() {}

std::string ConfigPersistence::trim(const std::string& str) {
    size_t start = str.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    size_t end = str.find_last_not_of(" \t\r\n");
    return str.substr(start, end - start + 1);
}

std::string ConfigPersistence::escapeValue(const std::string& value) {
    std::string result;
    for (char c : value) {
        if (c == '\n') result += "\\n";
        else if (c == '\r') result += "\\r";
        else if (c == '\\') result += "\\\\";
        else if (c == '=') result += "\\=";
        else result += c;
    }
    return result;
}

std::string ConfigPersistence::unescapeValue(const std::string& value) {
    std::string result;
    for (size_t i = 0; i < value.size(); i++) {
        if (value[i] == '\\' && i + 1 < value.size()) {
            char next = value[i + 1];
            if (next == 'n') { result += '\n'; i++; }
            else if (next == 'r') { result += '\r'; i++; }
            else if (next == '\\') { result += '\\'; i++; }
            else if (next == '=') { result += '='; i++; }
            else result += value[i];
        } else {
            result += value[i];
        }
    }
    return result;
}

bool ConfigPersistence::load() {
    std::ifstream file(m_configPath);
    if (!file.is_open()) {
        std::cout << "ConfigPersistence: No config file found at " << m_configPath
                  << ", using defaults." << std::endl;
        return false;
    }

    m_data.clear();
    std::string line;
    while (std::getline(file, line)) {
        line = trim(line);
        if (line.empty() || line[0] == '#' || line[0] == ';') continue;

        size_t eqPos = line.find('=');
        if (eqPos == std::string::npos) continue;

        std::string key = trim(line.substr(0, eqPos));
        std::string value = unescapeValue(trim(line.substr(eqPos + 1)));
        m_data[key] = value;
    }

    file.close();
    std::cout << "ConfigPersistence: Loaded " << m_data.size() << " config entries." << std::endl;
    return true;
}

bool ConfigPersistence::save() {
    std::ofstream file(m_configPath);
    if (!file.is_open()) {
        std::cerr << "ConfigPersistence: Failed to write config to " << m_configPath << std::endl;
        return false;
    }

    file << "# VN Engine Configuration" << std::endl;
    file << "# Auto-generated, do not edit manually" << std::endl;
    file << std::endl;

    for (const auto& [key, value] : m_data) {
        file << key << "=" << escapeValue(value) << std::endl;
    }

    file.close();
    std::cout << "ConfigPersistence: Saved " << m_data.size() << " config entries." << std::endl;
    return true;
}

void ConfigPersistence::setString(const std::string& key, const std::string& value) {
    m_data[key] = value;
}

void ConfigPersistence::setInt(const std::string& key, int value) {
    m_data[key] = std::to_string(value);
}

void ConfigPersistence::setFloat(const std::string& key, float value) {
    m_data[key] = std::to_string(value);
}

void ConfigPersistence::setBool(const std::string& key, bool value) {
    m_data[key] = value ? "true" : "false";
}

std::string ConfigPersistence::getString(const std::string& key, const std::string& defaultValue) const {
    auto it = m_data.find(key);
    if (it == m_data.end()) return defaultValue;
    return it->second;
}

int ConfigPersistence::getInt(const std::string& key, int defaultValue) const {
    auto it = m_data.find(key);
    if (it == m_data.end()) return defaultValue;
    try { return std::stoi(it->second); }
    catch (...) { return defaultValue; }
}

float ConfigPersistence::getFloat(const std::string& key, float defaultValue) const {
    auto it = m_data.find(key);
    if (it == m_data.end()) return defaultValue;
    try { return std::stof(it->second); }
    catch (...) { return defaultValue; }
}

bool ConfigPersistence::getBool(const std::string& key, bool defaultValue) const {
    auto it = m_data.find(key);
    if (it == m_data.end()) return defaultValue;
    const std::string& v = it->second;
    return (v == "true" || v == "1" || v == "yes" || v == "on");
}

bool ConfigPersistence::hasKey(const std::string& key) const {
    return m_data.find(key) != m_data.end();
}

void ConfigPersistence::removeKey(const std::string& key) {
    m_data.erase(key);
}

void ConfigPersistence::clear() {
    m_data.clear();
}

std::vector<std::string> ConfigPersistence::getKeys() const {
    std::vector<std::string> keys;
    keys.reserve(m_data.size());
    for (const auto& [key, value] : m_data) {
        keys.push_back(key);
    }
    return keys;
}

void ConfigPersistence::setDefaults(const std::unordered_map<std::string, std::string>& defaults) {
    for (const auto& [key, value] : defaults) {
        if (!hasKey(key)) {
            m_data[key] = value;
        }
    }
}

void ConfigPersistence::setWindowConfig(int width, int height, bool fullscreen) {
    setInt("window.width", width);
    setInt("window.height", height);
    setBool("window.fullscreen", fullscreen);
}

void ConfigPersistence::getWindowConfig(int& width, int& height, bool& fullscreen) {
    width = getInt("window.width", 1280);
    height = getInt("window.height", 720);
    fullscreen = getBool("window.fullscreen", false);
}

void ConfigPersistence::setAudioConfig(int masterVol, int bgmVol, int voiceVol, int seVol) {
    setInt("audio.master", masterVol);
    setInt("audio.bgm", bgmVol);
    setInt("audio.voice", voiceVol);
    setInt("audio.se", seVol);
}

void ConfigPersistence::getAudioConfig(int& masterVol, int& bgmVol, int& voiceVol, int& seVol) {
    masterVol = getInt("audio.master", 128);
    bgmVol = getInt("audio.bgm", 96);
    voiceVol = getInt("audio.voice", 128);
    seVol = getInt("audio.se", 96);
}

void ConfigPersistence::setTextSpeed(int speed) {
    setInt("text.speed", speed);
}

int ConfigPersistence::getTextSpeed() const {
    return getInt("text.speed", 2);
}

void ConfigPersistence::setLanguage(const std::string& lang) {
    setString("language", lang);
}

std::string ConfigPersistence::getLanguage() const {
    return getString("language", "ja");
}
