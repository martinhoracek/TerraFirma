/** @copyright 2025 Sean Kasun */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <dirent.h>
#include <memory.h>
#include <ctype.h>

static void dump(char *dir, char *name, FILE *cpp, FILE *h, bool str) {
  if (name[0] == '.') {
    return;
  }
  char fullname[1024];
  int dirlen = strlen(dir);
  int namelen = strlen(name);
  memcpy(fullname, dir, dirlen);
  fullname[dirlen] = '/';
  memcpy(fullname + dirlen + 1, name, strlen(name) + 1);
  FILE *f = fopen(fullname, "rb");
  fseek(f, 0, SEEK_END);
  size_t len = ftell(f);
  fseek(f, 0, SEEK_SET);
  uint8_t *data = (uint8_t *)malloc(len);
  fread(data, len, 1, f);
  fclose(f);
  for (int i = 0; i < namelen; i++) {
    char ch = name[i];
    if (!isalnum(ch)) {
      ch = '_';
    }
    fullname[i] = ch;
  }
  fullname[namelen] = 0;
  if (str) {
    fprintf(h, "extern const char %s[];\n", fullname);
    fprintf(cpp, "const char %s[]={", fullname);
  } else {
    fprintf(h, "extern const uint8_t %s[];\nextern size_t %s_length;\n", fullname, fullname);
    fprintf(cpp, "const uint8_t %s[]={", fullname);
  }
  for (int i = 0; i < len; i++) {
    fprintf(cpp, "0x%02x,", data[i]);
  }
  if (str) {
    fprintf(cpp, "0");
  }
  fprintf(cpp, "};\n");
  if (!str) {
    fprintf(cpp, "size_t %s_length=sizeof(%s);\n", fullname, fullname);
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

  DIR *dir = opendir(argv[1]);
  if (!dir) {
    fprintf(stderr, "Failed to open %s\n", argv[1]);
    return -1;
  }
  struct dirent *entry;
  while ((entry = readdir(dir)) != NULL) {
    dump(argv[1], entry->d_name, cpp, h, argc == 5);
  }
  closedir(dir);
  fclose(cpp);
  fclose(h);
}
