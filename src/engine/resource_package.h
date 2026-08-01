#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <cstdint>
#include <fstream>

// ============================================================
// .pak 资源包格式 (VNPK - VN Engine Package)
// ============================================================
//
// 文件结构:
//   [Header 32 bytes]
//   [Index Section]
//   [Data Section]
//
// Header (32 bytes):
//   magic       : 4 bytes  = "VNPK"
//   version     : uint32   = 1
//   fileCount   : uint32
//   indexSize   : uint64   (索引区字节数)
//   dataOffset  : uint64   (数据区起始偏移)
//   reserved    : uint32   = 0
//
// Index Entry (变长):
//   nameLength  : uint16
//   name        : char[nameLength]  (UTF-8, 无 null 结尾)
//   dataOffset  : uint64            (绝对偏移)
//   dataSize    : uint64
//   crc32       : uint32
//
// Data Section:
//   原始文件数据，顺序拼接
// ============================================================

// 资源条目
struct PackageEntry {
    std::string name;
    uint64_t offset = 0;
    uint64_t size = 0;
    uint32_t crc32 = 0;
};

// 包文件头
struct PackageHeader {
    char magic[4] = {'V', 'N', 'P', 'K'};
    uint32_t version = 1;
    uint32_t fileCount = 0;
    uint64_t indexSize = 0;
    uint64_t dataOffset = 0;
    uint32_t reserved = 0;
};

// ============================================================
// PackageReader — 运行时读取 .pak 文件
// ============================================================
class PackageReader {
public:
    PackageReader() = default;
    ~PackageReader();

    // 打开 .pak 文件
    bool open(const std::string& path);

    // 关闭
    void close();

    // 是否已打开
    bool isOpen() const { return m_file.is_open(); }

    // 检查资源是否存在
    bool has(const std::string& name) const;

    // 读取资源数据，返回原始字节
    std::vector<uint8_t> read(const std::string& name) const;

    // 读取资源数据到调用方缓冲区，返回实际读取字节数
    size_t readTo(const std::string& name, void* buffer, size_t bufferSize) const;

    // 列出所有资源名
    std::vector<std::string> list() const;

    // 获取资源条目信息
    const PackageEntry* getEntry(const std::string& name) const;

    // 获取文件数量
    uint32_t getFileCount() const { return m_header.fileCount; }

    // 获取版本
    uint32_t getVersion() const { return m_header.version; }

private:
    mutable std::ifstream m_file;
    PackageHeader m_header;
    std::unordered_map<std::string, PackageEntry> m_entries;

    bool readHeader();
    bool readIndex();
};

// ============================================================
// PackageWriter — 打包目录为 .pak 文件
// ============================================================
class PackageWriter {
public:
    PackageWriter() = default;
    ~PackageWriter() = default;

    // 将目录打包成 .pak 文件
    // dirPath: 源目录
    // outputPath: 输出 .pak 文件路径
    // 返回打包的文件数量，失败返回 0
    int pack(const std::string& dirPath, const std::string& outputPath);

    // 添加单个文件到包中（pack 前调用）
    // filePath: 文件系统路径
    // archiveName: 包内名称（如 "scripts/main.lua"）
    bool addFile(const std::string& filePath, const std::string& archiveName);

    // 写入 .pak 文件
    bool write(const std::string& outputPath);

    // 获取最后错误信息
    std::string getError() const { return m_error; }

private:
    std::vector<PackageEntry> m_entries;
    std::vector<std::string> m_filePaths;  // 对应的文件系统路径
    std::string m_error;

    // 递归收集目录下所有文件
    int collectDirectory(const std::string& dirPath, const std::string& prefix);

    // 计算 CRC32
    static uint32_t crc32(const uint8_t* data, size_t len);
};
