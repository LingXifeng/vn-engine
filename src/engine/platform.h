#pragma once

/**
 * platform.h — 跨平台工具標頭
 *
 * 提供 Windows / Linux / macOS 通用的平台抽象：
 * - 數學常數（M_PI 在 MSVC 不預設定義）
 * - 路徑分隔符
 * - 目錄創建
 * - 平台檢測巨集
 */

#include <string>
#include <filesystem>

// ============================================================================
// 平台檢測
// ============================================================================
#if defined(_WIN32) || defined(_WIN64)
    #define VN_PLATFORM_WINDOWS 1
    #define VN_PATH_SEPARATOR '\\'
    #define VN_PATH_SEPARATOR_STR "\\"
#else
    #define VN_PLATFORM_WINDOWS 0
    #define VN_PATH_SEPARATOR '/'
    #define VN_PATH_SEPARATOR_STR "/"
#endif

// ============================================================================
// 數學常數（MSVC 不預設定義 M_PI）
// ============================================================================
#ifndef M_PI
    #define M_PI 3.14159265358979323846
#endif
#ifndef M_PI_2
    #define M_PI_2 1.57079632679489661923
#endif

// ============================================================================
// 跨平台目錄操作
// ============================================================================
namespace vn::platform {

/**
 * 遞迴創建目錄（等價於 mkdir -p）
 * @return true 如果目錄已存在或創建成功
 */
inline bool createDirectory(const std::string& path) {
    if (path.empty()) return true;
    std::error_code ec;
    std::filesystem::create_directories(path, ec);
    return !ec;
}

/**
 * 檢查路徑是否存在
 */
inline bool pathExists(const std::string& path) {
    return std::filesystem::exists(path);
}

/**
 * 檢查路徑是否為目錄
 */
inline bool isDirectory(const std::string& path) {
    return std::filesystem::is_directory(path);
}

/**
 * 獲取目錄部分（等價於 dirname）
 */
inline std::string dirName(const std::string& path) {
    if (path.empty()) return ".";
    size_t pos = path.find_last_of("/\\");
    if (pos == std::string::npos) return ".";
    if (pos == 0) return "/";
    return path.substr(0, pos);
}

/**
 * 獲取文件名部分（等價於 basename）
 */
inline std::string baseName(const std::string& path) {
    if (path.empty()) return "";
    size_t pos = path.find_last_of("/\\");
    if (pos == std::string::npos) return path;
    return path.substr(pos + 1);
}

/**
 * 拼接路徑（自動處理分隔符）
 */
inline std::string joinPath(const std::string& a, const std::string& b) {
    if (a.empty()) return b;
    if (b.empty()) return a;
    char last = a.back();
    if (last == '/' || last == '\\') return a + b;
    return a + VN_PATH_SEPARATOR_STR + b;
}

} // namespace vn::platform
