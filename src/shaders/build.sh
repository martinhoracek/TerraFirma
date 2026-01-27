#!/bin/sh

# download shadercross from
# https://nightly.link/libsdl-org/SDL_shadercross/workflows/main/main

shadercross="${HOME}/Downloads/SDL3_shadercross-3.0.0-linux-x64/bin/shadercross"
if [ ! -f "$shadercross" ]; then
  shadercross="${HOME}/Downloads/SDL3_shadercross-3.0.0-darwin-arm64-x64/bin/shadercross"
fi

shaders=`find . -type f -name "*.vert" -o -name "*.frag"`

for file in $shaders; do
  out="${file/./compiled}"
  glslc -o "${out}.spv" $file
  ${shadercross} ${out}.spv -d MSL -o ${out}.msl
  ${shadercross} ${out}.spv -d DXIL -o ${out}.dxil
done
