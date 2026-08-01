#include "resource_package.h"
#include <iostream>
#include <filesystem>
#include <cstring>
#include <algorithm>

namespace fs = std::filesystem;

// ============================================================
// PackageReader 实现
// ============================================================

PackageReader::~PackageReader() {
    close();
}

bool PackageReader::open(const std::string& path) {
    m_file.open(path, std::ios::binary);
    if (!m_file.is_open()) {
        std::cerr << "PackageReader: Failed to open " << path << std::endl;
        return false;
    }

    if (!readHeader()) {
        close();
        return false;
    }

    if (!readIndex()) {
        close();
        return false;
    }

    std::cout << "PackageReader: Opened " << path << " (" << m_header.fileCount << " files)" << std::endl;
    return true;
}

void PackageReader::close() {
    if (m_file.is_open()) {
        m_file.close();
    }
    m_entries.clear();
    m_header = PackageHeader{};
}

bool PackageReader::readHeader() {
    m_file.read(reinterpret_cast<char*>(&m_header), sizeof(PackageHeader));
    if (m_file.gcount() != sizeof(PackageHeader)) {
        std::cerr << "PackageReader: Failed to read header" << std::endl;
        return false;
    }

    if (std::memcmp(m_header.magic, "VNPK", 4) != 0) {
        std::cerr << "PackageReader: Invalid magic (not a .pak file)" << std::endl;
        return false;
    }

    if (m_header.version != 1) {
        std::cerr << "PackageReader: Unsupported version " << m_header.version << std::endl;
        return false;
    }

    return true;
}

bool PackageReader::readIndex() {
    // 索引紧跟在 header 之后
    m_file.seekg(sizeof(PackageHeader), std::ios::beg);

    for (uint32_t i = 0; i < m_header.fileCount; i++) {
        PackageEntry entry;

        // 读取名称长度
        uint16_t nameLen = 0;
        m_file.read(reinterpret_cast<char*>(&nameLen), sizeof(nameLen));
        if (m_file.gcount() != sizeof(nameLen)) {
            std::cerr << "PackageReader: Failed to read entry name length" << std::endl;
            return false;
        }

        // 读取名称
        entry.name.resize(nameLen);
        m_file.read(entry.name.data(), nameLen);
        if (m_file.gcount() != nameLen) {
            std::cerr << "PackageReader: Failed to read entry name" << std::endl;
            return false;
        }

        // 读取 offset, size, crc32
        m_file.read(reinterpret_cast<char*>(&entry.offset), sizeof(entry.offset));
        m_file.read(reinterpret_cast<char*>(&entry.size), sizeof(entry.size));
        m_file.read(reinterpret_cast<char*>(&entry.crc32), sizeof(entry.crc32));

        if (m_file.gcount() != sizeof(entry.crc32)) {
            std::cerr << "PackageReader: Failed to read entry metadata" << std::endl;
            return false;
        }

        m_entries[entry.name] = entry;
    }

    return true;
}

bool PackageReader::has(const std::string& name) const {
    return m_entries.find(name) != m_entries.end();
}

std::vector<uint8_t> PackageReader::read(const std::string& name) const {
    auto it = m_entries.find(name);
    if (it == m_entries.end()) {
        return {};
    }

    const auto& entry = it->second;
    std::vector<uint8_t> data(entry.size);

    m_file.clear();
    m_file.seekg(entry.offset, std::ios::beg);
    m_file.read(reinterpret_cast<char*>(data.data()), entry.size);

    if (static_cast<uint64_t>(m_file.gcount()) != entry.size) {
        std::cerr << "PackageReader: Failed to read full data for " << name << std::endl;
        return {};
    }

    return data;
}

size_t PackageReader::readTo(const std::string& name, void* buffer, size_t bufferSize) const {
    auto it = m_entries.find(name);
    if (it == m_entries.end()) return 0;

    const auto& entry = it->second;
    size_t toRead = std::min(static_cast<size_t>(entry.size), bufferSize);

    m_file.clear();
    m_file.seekg(entry.offset, std::ios::beg);
    m_file.read(static_cast<char*>(buffer), toRead);

    return static_cast<size_t>(m_file.gcount());
}

std::vector<std::string> PackageReader::list() const {
    std::vector<std::string> names;
    names.reserve(m_entries.size());
    for (const auto& [name, entry] : m_entries) {
        names.push_back(name);
    }
    return names;
}

const PackageEntry* PackageReader::getEntry(const std::string& name) const {
    auto it = m_entries.find(name);
    if (it == m_entries.end()) return nullptr;
    return &it->second;
}

// ============================================================
// PackageWriter 实现
// ============================================================

int PackageWriter::pack(const std::string& dirPath, const std::string& outputPath) {
    m_entries.clear();
    m_filePaths.clear();

    int count = collectDirectory(dirPath, "");
    if (count == 0) {
        m_error = "No files found in " + dirPath;
        return 0;
    }

    if (!write(outputPath)) {
        return 0;
    }

    return count;
}

int PackageWriter::collectDirectory(const std::string& dirPath, const std::string& prefix) {
    int count = 0;

    if (!fs::exists(dirPath) || !fs::is_directory(dirPath)) {
        m_error = "Directory not found: " + dirPath;
        return 0;
    }

    for (const auto& entry : fs::directory_iterator(dirPath)) {
        std::string name = entry.path().filename().string();

        if (entry.is_directory()) {
            std::string newPrefix = prefix.empty() ? name : prefix + "/" + name;
            count += collectDirectory(entry.path().string(), newPrefix);
        } else if (entry.is_regular_file()) {
            std::string archiveName = prefix.empty() ? name : prefix + "/" + name;
            m_filePaths.push_back(entry.path().string());
            m_entries.push_back(PackageEntry{archiveName, 0, 0, 0});
            count++;
        }
    }

    return count;
}

bool PackageWriter::addFile(const std::string& filePath, const std::string& archiveName) {
    if (!fs::exists(filePath) || !fs::is_regular_file(filePath)) {
        m_error = "File not found: " + filePath;
        return false;
    }
    m_filePaths.push_back(filePath);
    m_entries.push_back(PackageEntry{archiveName, 0, 0, 0});
    return true;
}

bool PackageWriter::write(const std::string& outputPath) {
    if (m_entries.empty()) {
        m_error = "No files to write";
        return false;
    }

    // 先计算每个文件的大小和 CRC32
    for (size_t i = 0; i < m_entries.size(); i++) {
        std::ifstream f(m_filePaths[i], std::ios::binary);
        if (!f.is_open()) {
            m_error = "Failed to open: " + m_filePaths[i];
            return false;
        }
        f.seekg(0, std::ios::end);
        m_entries[i].size = static_cast<uint64_t>(f.tellg());
        f.seekg(0, std::ios::beg);

        // 计算 CRC32
        std::vector<uint8_t> fileData(m_entries[i].size);
        f.read(reinterpret_cast<char*>(fileData.data()), m_entries[i].size);
        m_entries[i].crc32 = crc32(fileData.data(), fileData.size());
    }

    // 计算索引区大小
    uint64_t indexSize = 0;
    for (const auto& e : m_entries) {
        indexSize += sizeof(uint16_t) + e.name.size() + sizeof(uint64_t) + sizeof(uint64_t) + sizeof(uint32_t);
    }

    // 数据区起始偏移
    uint64_t dataOffset = sizeof(PackageHeader) + indexSize;

    // 计算每个文件的数据偏移
    uint64_t currentOffset = dataOffset;
    for (auto& e : m_entries) {
        e.offset = currentOffset;
        currentOffset += e.size;
    }

    // 写入 .pak 文件
    std::ofstream out(outputPath, std::ios::binary);
    if (!out.is_open()) {
        m_error = "Failed to create output file: " + outputPath;
        return false;
    }

    // 写入 Header
    PackageHeader header;
    header.fileCount = static_cast<uint32_t>(m_entries.size());
    header.indexSize = indexSize;
    header.dataOffset = dataOffset;
    out.write(reinterpret_cast<const char*>(&header), sizeof(header));

    // 写入索引
    for (const auto& e : m_entries) {
        uint16_t nameLen = static_cast<uint16_t>(e.name.size());
        out.write(reinterpret_cast<const char*>(&nameLen), sizeof(nameLen));
        out.write(e.name.data(), e.name.size());
        out.write(reinterpret_cast<const char*>(&e.offset), sizeof(e.offset));
        out.write(reinterpret_cast<const char*>(&e.size), sizeof(e.size));
        out.write(reinterpret_cast<const char*>(&e.crc32), sizeof(e.crc32));
    }

    // 写入数据
    for (size_t i = 0; i < m_entries.size(); i++) {
        std::ifstream f(m_filePaths[i], std::ios::binary);
        if (!f.is_open()) {
            m_error = "Failed to reopen: " + m_filePaths[i];
            return false;
        }
        // 分块写入，避免大文件一次性读入内存
        constexpr size_t CHUNK = 65536;
        std::vector<char> buffer(CHUNK);
        while (f) {
            f.read(buffer.data(), CHUNK);
            std::streamsize bytesRead = f.gcount();
            if (bytesRead > 0) {
                out.write(buffer.data(), bytesRead);
            }
        }
    }

    out.close();
    return true;
}

uint32_t PackageWriter::crc32(const uint8_t* data, size_t len) {
    // CRC32 (IEEE 802.3)
    static uint32_t table[256];
    static bool tableInit = false;

    if (!tableInit) {
        for (uint32_t i = 0; i < 256; i++) {
            uint32_t c = i;
            for (int j = 0; j < 8; j++) {
                if (c & 1) {
                    c = 0xEDB88320U ^ (c >> 1);
                } else {
                    c >>= 1;
                }
            }
            table[i] = c;
        }
        tableInit = true;
    }

    uint32_t crc = 0xFFFFFFFFU;
    for (size_t i = 0; i < len; i++) {
        crc = table[(crc ^ data[i]) & 0xFF] ^ (crc >> 8);
    }
    return crc ^ 0xFFFFFFFFU;
}
