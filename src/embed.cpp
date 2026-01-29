/** @copyright 2025 Sean Kasun */

#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <filesystem>
#include <memory.h>
#include <ctype.h>

static void dump(const std::filesystem::path &fullname, FILE *cpp, FILE *h, bool str) {
  FILE *f = fopen(fullname.string().c_str(), "rb");
  fseek(f, 0, SEEK_END);
  size_t len = ftell(f);
  fseek(f, 0, SEEK_SET);
  uint8_t *data = (uint8_t *)malloc(len);
  fread(data, len, 1, f);
  fclose(f);
  char name[1024];
  int i = 0;
  for (auto ch : fullname.filename().string()) {
    if (!isalnum(ch)) {
      ch = '_';
    }
    name[i++] = ch;
  }
  name[i] = 0;
  if (str) {
    fprintf(h, "extern const char %s[];\n", name);
    fprintf(cpp, "const char %s[]={", name);
  } else {
    fprintf(h, "extern const uint8_t %s[];\nextern size_t %s_length;\n", name, name);
    fprintf(cpp, "const uint8_t %s[]={", name);
  }
  for (int i = 0; i < len; i++) {
    fprintf(cpp, "0x%02x,", data[i]);
  }
  if (str) {
    fprintf(cpp, "0");
  }
  fprintf(cpp, "};\n");
  if (!str) {
    fprintf(cpp, "size_t %s_length=sizeof(%s);\n", name, name);
  }
}

int main(int argc, char **argv) {
  if (argc < 4) {
    fprintf(stderr, "Usage: %s folder out.cpp out.h [str]\n", argv[0]);
    return -1;
  }

  FILE *cpp = fopen(argv[2], "wb");
  if (!cpp) {
    fprintf(stderr, "Failed to create %s\n", argv[2]);
    return -1;
  }
  FILE *h = fopen(argv[3], "wb");
  if (!h) {
    fprintf(stderr, "Failed to create %s\n", argv[3]);
    return -1;
  }

  fprintf(cpp, "#include \"%s\"\n", argv[3]);
  fprintf(h, "#pragma once\n#include <cstdint>\n#include <cstddef>\n");


  const std::filesystem::directory_iterator dir(argv[1]);
  for (const auto &file : dir) {
    dump(file.path(), cpp, h, argc == 5);
  }
  fclose(cpp);
  fclose(h);
}
