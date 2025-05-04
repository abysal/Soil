#pragma once

#include <RmlUi/Config/Config.h>
#include <RmlUi/Core/FileInterface.h>
#include <RmlUi/Core/Types.h>
#include <bit>
#include <cstddef>
#include <cstdio>
#include <fstream>
#include <spanstream>
#include <unordered_map>
#include <variant>

#include <stdio.h>

namespace soil {

    class FilesystemShim : public Rml::FileInterface {
    public:
        FilesystemShim();
        FilesystemShim& operator=(const FilesystemShim&) = delete;
        FilesystemShim& operator=(FilesystemShim&&)      = delete;
        FilesystemShim(const FilesystemShim&)            = delete;
        FilesystemShim(FilesystemShim&&)                 = delete;
        ~FilesystemShim() override                       = default;

        Rml::FileHandle Open(const std::string& path) override;
        void Close(Rml::FileHandle handle) override { this->file_lookup.erase(handle); }

        size_t Read(void* out_buffer, size_t count, Rml::FileHandle handle) override;

        bool Seek(Rml::FileHandle file, long offset, int origin) override;

        size_t Tell(Rml::FileHandle handle) override;

    private:
        std::unordered_map<Rml::FileHandle, std::variant<std::ifstream, std::ispanstream>>
            file_lookup;

        std::unordered_map<Rml::String, std::span<char>> baked_files;
        Rml::FileHandle                                  next_handle{1};
    };

    class SimpleShim : public Rml::FileInterface {
    public:
        Rml::FileHandle Open(const std::string& path) override {
            return std::bit_cast<Rml::FileHandle>(std::fopen(path.c_str(), "rb"));
        }
        void Close(Rml::FileHandle handle) override { fclose(std::bit_cast<FILE*>(handle)); }

        size_t Read(void* out_buffer, size_t count, Rml::FileHandle handle) override {
            return std::fread(out_buffer, 1, count, std::bit_cast<FILE*>(handle));
        }

        bool Seek(Rml::FileHandle file, long offset, int origin) override {
            return std::fseek(std::bit_cast<FILE*>(file), offset, origin);
        }

        size_t Tell(Rml::FileHandle handle) override {
            return static_cast<size_t>(std::ftell(std::bit_cast<FILE*>(handle)));
        }
    };

} // namespace soil