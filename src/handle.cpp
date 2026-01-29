/** @copyright 2025 Sean Kasun */

#include "handle.h"
#include <filesystem>
#include <fstream>

Handle::Handle(const std::string &filename) {
  data = pos = nullptr;
  std::ifstream f(filename, std::ios::in | std::ios::binary);
  if (!f.is_open()) {
    return;
  }
  length = std::filesystem::file_size(filename);
  data = new uint8_t[length];
  f.read(reinterpret_cast<char*>(data), length);
  f.close();
  pos = data;
  alloc = true;
}

Handle::Handle(uint8_t *data, uint32_t len) : data(data), length(len) {
  pos = data;
  alloc = false;
}

Handle::~Handle() {
  if (alloc) {
    delete [] data;
  }
}

bool Handle::isOpen() const {
  return data != nullptr;
}

int64_t Handle::tell() const {
  return pos - data;
}

uint8_t Handle::r8() {
  return *pos++;
}

uint16_t Handle::r16() {
  uint16_t r = *pos++;
  r |= *pos++ << 8;
  return r;
}

uint32_t Handle::r32() {
  uint32_t r = *pos++;
  r |= *pos++ << 8;
  r |= *pos++ << 16;
  r |= *pos++ << 24;
  return r;
}

uint64_t Handle::r64() {
  uint64_t r = r32();
  r |= static_cast<uint64_t>(r32()) << 32;
  return r;
}

float Handle::rf() {
  union {
    float f;
    uint32_t l;
  } fl;
  fl.l = r32();
  return fl.f;
}

double Handle::rd() {
  union {
    double d;
    uint64_t l;
  } dl;
  dl.l = r64();
  return dl.d;
}

std::string Handle::read(int len) {
  std::string s(reinterpret_cast<char const *>(pos), len);
  pos += len;
  return s;
}

std::string Handle::rcs() {
  std::string r;
  char ch;
  while ((ch = *pos++) != 0) {
    r += ch;
  }
  return r;
}

std::string Handle::rs() {
  uint32_t len = 0;
  int shift = 0;
  uint8_t u7;
  do {
    u7 = *pos++;
    len |= (u7 & 0x7f) << shift;
    shift += 7;
  } while (u7 & 0x80);
  return read(len);
}

uint8_t *Handle::readBytes(int length) {
  uint8_t *oldpos = pos;
  pos += length;
  return oldpos;
}

void Handle::skip(int64_t length) {
  pos += length;
}

void Handle::seek(int64_t p) {
  pos = data + p;
}
