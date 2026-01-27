/** @copyright 2025 Sean Kasun */

#pragma once

#include <string>
#include <cstdint>

class Handle {
  public:
    explicit Handle(const std::string &filename);
    Handle(uint8_t *data, uint32_t len);
    ~Handle();

    bool isOpen() const;
    bool eof() const;
    int64_t tell() const;
    
    uint8_t r8();
    uint16_t r16();
    uint32_t r32();
    uint64_t r64();
    float rf();
    double rd();
    std::string read(int length);
    std::string rcs();
    std::string rs();
    uint8_t *readBytes(int length);
    void seek(int64_t pos);
    void skip(int64_t length);

    int64_t length;

  private:
    uint8_t *data, *pos;
    bool alloc = false;
};
