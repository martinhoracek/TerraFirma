add_executable(${PROJECT_NAME})

add_executable(embed embed.cpp)
file(GLOB shaderbins shaders/compiled/*)
add_custom_command(
  OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/shaders.cpp ${CMAKE_CURRENT_SOURCE_DIR}/shaders.h
  COMMAND embed shaders/compiled shaders.cpp shaders.h
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  DEPENDS ${shaderbins}
)

file(GLOB assetjsons "${PROJECT_SOURCE_DIR}/assets/jsons/*")
add_custom_command(
  OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/assets.cpp ${CMAKE_CURRENT_SOURCE_DIR}/assets.h
  COMMAND embed "${PROJECT_SOURCE_DIR}/assets/jsons" assets.cpp assets.h str
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  DEPENDS ${assetjsons}
)

file(GLOB ttfbins "${PROJECT_SOURCE_DIR}/assets/ttfs/*")
add_custom_command(
  OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/ttfs.cpp ${CMAKE_CURRENT_SOURCE_DIR}/ttfs.h
  COMMAND embed "${PROJECT_SOURCE_DIR}/assets/ttfs" ttfs.cpp ttfs.h
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  DEPENDS ${ttfbins}
)

target_sources(${PROJECT_NAME} PRIVATE
  main.cpp
  bestiary.cpp bestiary.h
  filedialogfont.cpp filedialogfont.h
  findchests.cpp findchests.h
  gui.cpp gui.h
  handle.cpp handle.h
  hilitewin.cpp hilitewin.h
  infowin.cpp infowin.h
  json.cpp json.h
  l10n.cpp l10n.h
  killwin.cpp killwin.h
  map.cpp map.h
  pipelines.cpp pipelines.h
  renderer.cpp renderer.h
  settings.cpp settings.h
  steamconfig.cpp steamconfig.h
  terrafirma.cpp terrafirma.h
  textures.cpp textures.h
  tiles.cpp tiles.h
  uvrules.cpp uvrules.h
  world.cpp world.h
  worldheader.cpp worldheader.h
  worldinfo.cpp worldinfo.h
  assets.cpp assets.h
  ttfs.cpp ttfs.h
  shaders.cpp shaders.h
  lzx.c lzx.h
)

target_link_libraries(${PROJECT_NAME} PRIVATE vendor)
