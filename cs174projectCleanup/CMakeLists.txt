cmake_minimum_required (VERSION 3.18)
project (cs174a_term_project)

set(sources
  CameraEntity.cpp            CollisionBox.cpp          Engine.cpp
  Framebuffer.cpp             GameEntity.cpp            general.cpp
  InitShader.cpp              main.cpp                  mobileEntity.cpp
  obj_loader/list.cpp         obj_loader/obj_parser.cpp obj_loader/objLoader.cpp
  obj_loader/string_extra.cpp Player.cpp                Quaternion.cpp
  RenderObject.cpp            RenderObjectManager.cpp   SoundPlayer.cpp
  Teleporter.cpp              Text2D.cpp                Texture.cpp
  TextureManager.cpp          TVScreen.cpp              WanderingEnemy.cpp
  world.cpp                   WorldEntity.cpp           Gamepad.cpp
)
# Source files omitted: obj_loader/objTester.cpp test_main.cpp

if (DEFINED EMSCRIPTEN)
  find_program(SED sed DOC "sed is required to patch GLSL code" REQUIRED)
  # It might be better to patch the GLSL files using configure_file. We'd have
  # to do that for all builds, though, and it would make the installation of
  # resources for non-emscripten builds more complicated.
  add_custom_command(
    OUTPUT PhongShader_vertex.glsl
    COMMAND ${SED} "1s/.*/#version 300 es/" ${CMAKE_CURRENT_SOURCE_DIR}/resources/PhongShader_vertex.glsl > PhongShader_vertex.glsl
    COMMENT "Patching PhongShader_vertex.glsl for OpenGL ES"
    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/resources/PhongShader_vertex.glsl"
    VERBATIM
  )
  add_custom_command(
    OUTPUT PhongShader_fragment.glsl
    COMMAND ${SED} "1s/.*/#version 300 es/" ${CMAKE_CURRENT_SOURCE_DIR}/resources/PhongShader_fragment.glsl > PhongShader_fragment.glsl
    COMMENT "Patching PhongShader_fragment.glsl for OpenGL ES"
    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/resources/PhongShader_fragment.glsl"
    VERBATIM
  )

  add_executable(${PROJECT_NAME} ${sources} PhongShader_vertex.glsl PhongShader_fragment.glsl)
  target_compile_options(${PROJECT_NAME} PRIVATE
    --use-port=sdl2
    --use-port=sdl2_ttf
    --use-port=sdl2_mixer
    -sSDL2_MIXER_FORMATS=wav,mp3
    --use-port=sdl2_image:formats=png,jpg
  )
  target_link_options(${PROJECT_NAME} PRIVATE
    --use-port=sdl2
    --use-port=sdl2_ttf
    --use-port=sdl2_mixer
    -sSDL2_MIXER_FORMATS=wav,mp3
    --use-port=sdl2_image:formats=png,jpg
    -sMIN_WEBGL_VERSION=2
    -sMAX_WEBGL_VERSION=2
    -sOFFSCREEN_FRAMEBUFFER=1
    -sALLOW_MEMORY_GROWTH
    -sINITIAL_MEMORY=67108864
    -sEXPORTED_FUNCTIONS=_invokeMain,_setFullscreen
    -Wno-unused-main # We don't invoke main directly from JS
    -sNO_MINIFY_HTML # Minification doesn't work out-of-the-box with Nix's emscripten
    "SHELL:--preload-file ${CMAKE_BINARY_DIR}/PhongShader_vertex.glsl@/resources/PhongShader_vertex.glsl"
    "SHELL:--preload-file ${CMAKE_BINARY_DIR}/PhongShader_fragment.glsl@/resources/PhongShader_fragment.glsl"
    "SHELL:--preload-file ${CMAKE_SOURCE_DIR}/resources@/resources"
    "--shell-file=${CMAKE_CURRENT_SOURCE_DIR}/shell.html"
  )
  file(GLOB_RECURSE resource_files "${CMAKE_SOURCE_DIR}/resources/*")
  set_target_properties(${PROJECT_NAME} PROPERTIES
    LINK_DEPENDS "${resource_files};${CMAKE_CURRENT_SOURCE_DIR}/shell.html"
    ADDITIONAL_CLEAN_FILES "${PROJECT_NAME}.data;${PROJECT_NAME}.js;${PROJECT_NAME}.wasm"
  )
  set(CMAKE_EXECUTABLE_SUFFIX .html)

  # TODO: Move the web resources out of the resources directory so that they aren't
  #       delivered in the data file unnecessarily.
  if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    file(CREATE_LINK "${CMAKE_SOURCE_DIR}/resources/favicon.png" "favicon.png" SYMBOLIC)
  else()
    include(GNUInstallDirs)
    install(TARGETS ${PROJECT_NAME} DESTINATION "${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}")
    install(
      FILES
        "$<LIST:TRANSFORM,.wasm;.js;.data,PREPEND,${CMAKE_BINARY_DIR}/${PROJECT_NAME}>"
        "resources/favicon.png"
      DESTINATION "${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}"
    )
  endif()
else ()

include(FindPkgConfig)
pkg_check_modules(SDL2       REQUIRED IMPORTED_TARGET sdl2)
pkg_check_modules(SDL2_MIXER REQUIRED IMPORTED_TARGET SDL2_mixer)
pkg_check_modules(SDL2_TTF   REQUIRED IMPORTED_TARGET SDL2_ttf)
pkg_check_modules(SDL2_IMAGE REQUIRED IMPORTED_TARGET SDL2_image)
pkg_check_modules(GLEW       REQUIRED IMPORTED_TARGET glew)
find_package     (OpenGL     REQUIRED)

set(CMAKE_BUILD_TYPE RelWithDebInfo) # add debugging information, optimize

add_executable(${PROJECT_NAME} ${sources})
target_link_libraries(${PROJECT_NAME}
  OpenGL::GL
  PkgConfig::SDL2
  PkgConfig::SDL2_MIXER
  PkgConfig::SDL2_TTF
  PkgConfig::SDL2_IMAGE
  PkgConfig::GLEW
)

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  file(INSTALL resources DESTINATION .)
else()
  include(GNUInstallDirs)
  add_compile_definitions(CMAKE_INSTALL_FULL_DATADIR="${CMAKE_INSTALL_FULL_DATADIR}")
  install(TARGETS ${PROJECT_NAME} RUNTIME)
  install(DIRECTORY resources TYPE DATA)
endif()

endif()

# nix packages
#   cmake
#   pkg-config
#   glew
#   SDL2
#   SDL2_mixer
#   SDL2_ttf
#   SDL2_image

# apt-get packages
#   cmake
#   pkg-config
#   libglew-dev
#   libsdl2-dev
#   libsdl2-mixer-dev
#   libsdl2-ttf-dev
#   libsdl2-image-dev

# brew packages
#   cmake
#   pkg-config
#   sdl2
#   sdl2_mixer
#   sdl2_ttf
#   sdl2_image
#   glew
