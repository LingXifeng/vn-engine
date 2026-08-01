// pak_tool.cpp — 资源打包工具
// 用法:
//   pak_tool pack <目录> <输出.pak>     将目录打包成 .pak 文件
//   pak_tool list <文件.pak>            列出 .pak 中的所有资源
//   pak_tool info <文件.pak>            显示 .pak 文件信息
//   pak_tool extract <文件.pak> <名称> <输出路径>  从 .pak 中提取单个资源
//
// 示例:
//   pak_tool pack scripts/ resources.pak
//   pak_tool list resources.pak
//   pak_tool extract resources.pak scripts/main.lua main.lua

#include "../src/engine/resource_package.h"
#include <iostream>
#include <fstream>
#include <filesystem>

static void printUsage() {
    std::cout << "VN Engine Package Tool\n"
              << "======================\n\n"
              << "Usage:\n"
              << "  pak_tool pack <dir> <output.pak>\n"
              << "      Pack a directory into a .pak file\n\n"
              << "  pak_tool list <file.pak>\n"
              << "      List all resources in a .pak file\n\n"
              << "  pak_tool info <file.pak>\n"
              << "      Show .pak file information\n\n"
              << "  pak_tool extract <file.pak> <name> <output_path>\n"
              << "      Extract a single resource from .pak\n\n"
              << "Examples:\n"
              << "  pak_tool pack scripts/ resources.pak\n"
              << "  pak_tool list resources.pak\n"
              << "  pak_tool extract resources.pak scripts/main.lua main.lua\n";
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printUsage();
        return 1;
    }

    std::string command = argv[1];

    if (command == "pack") {
        if (argc < 4) {
            std::cerr << "Error: pack requires <dir> <output.pak>\n";
            return 1;
        }
        std::string dir = argv[2];
        std::string output = argv[3];

        PackageWriter writer;
        int count = writer.pack(dir, output);
        if (count > 0) {
            std::cout << "Packed " << count << " files into " << output << std::endl;
            auto fileSize = std::filesystem::file_size(output);
            std::cout << "Output size: " << (fileSize / 1024) << " KB" << std::endl;
            return 0;
        } else {
            std::cerr << "Error: " << writer.getError() << std::endl;
            return 1;
        }
    }
    else if (command == "list") {
        if (argc < 3) {
            std::cerr << "Error: list requires <file.pak>\n";
            return 1;
        }
        PackageReader reader;
        if (!reader.open(argv[2])) {
            std::cerr << "Error: Failed to open " << argv[2] << std::endl;
            return 1;
        }
        auto names = reader.list();
        std::cout << "Files in " << argv[2] << " (" << names.size() << " total):\n";
        for (const auto& name : names) {
            const auto* entry = reader.getEntry(name);
            std::cout << "  " << name << "  (" << entry->size << " bytes, crc=0x"
                      << std::hex << entry->crc32 << std::dec << ")\n";
        }
        return 0;
    }
    else if (command == "info") {
        if (argc < 3) {
            std::cerr << "Error: info requires <file.pak>\n";
            return 1;
        }
        PackageReader reader;
        if (!reader.open(argv[2])) {
            std::cerr << "Error: Failed to open " << argv[2] << std::endl;
            return 1;
        }
        auto fileSize = std::filesystem::file_size(argv[2]);
        std::cout << "Package: " << argv[2] << "\n"
                  << "Version: " << reader.getVersion() << "\n"
                  << "Files: " << reader.getFileCount() << "\n"
                  << "Package size: " << fileSize << " bytes (" << (fileSize / 1024) << " KB)\n";
        return 0;
    }
    else if (command == "extract") {
        if (argc < 5) {
            std::cerr << "Error: extract requires <file.pak> <name> <output_path>\n";
            return 1;
        }
        PackageReader reader;
        if (!reader.open(argv[2])) {
            std::cerr << "Error: Failed to open " << argv[2] << std::endl;
            return 1;
        }
        std::string name = argv[3];
        std::string outputPath = argv[4];

        if (!reader.has(name)) {
            std::cerr << "Error: Resource '" << name << "' not found in package\n";
            return 1;
        }

        auto data = reader.read(name);
        std::ofstream out(outputPath, std::ios::binary);
        out.write(reinterpret_cast<const char*>(data.data()), data.size());
        out.close();

        std::cout << "Extracted '" << name << "' (" << data.size() << " bytes) to " << outputPath << std::endl;
        return 0;
    }
    else {
        std::cerr << "Unknown command: " << command << std::endl;
        printUsage();
        return 1;
    }
}
