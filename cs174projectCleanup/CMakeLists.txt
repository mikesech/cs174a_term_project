cmake_minimum_required (VERSION 3.27)
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
  # It turns out that the resources directory contains 8.17 MB of unused data.
  # Instead of preloading the whole thing, we'll preload just the files we need.
  #
  # The shaders are omitted here because they are patched and not taken directly
  # from the resources directory.
  set(bundled_resources
    ClosedDoorTex.png         FreeMonoBold.ttf          cannon.wav
    cl1.mp3                   cube.obj                  curvy.wav
    death.wav                 diamondPlateNormal.jpg    fireball.wav
    floorNormal.jpg           grassTexture.jpg          grenade.obj
    jump.wav
    mechNormal2.jpg           mechTexture.png           plane.obj
    rockFlat.obj              rockMed.obj               rockTall.obj
    sphere.obj                tankBase.obj              tankTexture.png
    tankTop.obj               treesTest.png             turretBarrel.obj
    turretBase.obj            turretTexture.png         turretTop.obj
    tvcube.obj                wall.obj                  wanderingBase.obj
    wanderingTexture.png      wanderingTop.obj
  )
  # These resources are made available alongside the HTML, WASM, etc. files.
  set(web_resources
    favicon.png               hud.png                   intro_screen.jpg
    404.html
  )

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

  set(USE_WEBP_TEXTURES OFF CACHE BOOL "If ON, compresses all textures with cwebp before bundling")
  if(USE_WEBP_TEXTURES)
    add_compile_definitions(USE_WEBP_TEXTURES)
    find_program(CWEBP cwebp DOC "cwebp is required to recompress textures" REQUIRED)
    macro(partition_out src regex output)
      set(${output} ${${src}})
      list(FILTER ${output} INCLUDE REGEX ${regex})
      list(FILTER ${src}    EXCLUDE REGEX ${regex})
    endmacro()
    partition_out(bundled_resources "\\.png$" bundled_pngs)
    partition_out(bundled_resources "\\.jpg$" bundled_jpgs)

    file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/compressed_resources")
    foreach(r IN LISTS bundled_pngs)
      add_custom_command(
        OUTPUT compressed_resources/${r}.webp
        COMMAND ${CWEBP} -lossless -q 100 "${CMAKE_CURRENT_SOURCE_DIR}/resources/${r}" -o compressed_resources/${r}.webp
        COMMENT "Compressing ${r} with cwebp"
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/resources/${r}"
        VERBATIM
      )
    endforeach()
    foreach(r IN LISTS bundled_jpgs)
      add_custom_command(
        OUTPUT compressed_resources/${r}.webp
        COMMAND ${CWEBP} -q 85 "${CMAKE_CURRENT_SOURCE_DIR}/resources/${r}" -o compressed_resources/${r}.webp
        COMMENT "Compressing ${r} with cwebp"
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/resources/${r}"
        VERBATIM
      )
    endforeach()
    set(compressed_resources ${bundled_pngs};${bundled_jpgs})
    list(
      TRANSFORM compressed_resources
      REPLACE "^(.+)$" "compressed_resources/\\0.webp"
    )
  endif()

  add_executable(${PROJECT_NAME} ${sources} PhongShader_vertex.glsl PhongShader_fragment.glsl ${compressed_resources})
  target_compile_options(${PROJECT_NAME} PRIVATE
    --use-port=sdl2
    --use-port=sdl2_mixer
    -sSDL2_MIXER_FORMATS=wav,mp3
    --use-port=sdl2_image
  )
  list(
    TRANSFORM bundled_resources
    REPLACE "^(.+)$" "SHELL:--preload-file ${CMAKE_CURRENT_SOURCE_DIR}/resources/\\0@/resources/\\0"
    OUTPUT_VARIABLE preload_file_args
  )
  list(
    TRANSFORM compressed_resources
    REPLACE "^compressed_resources/(.+)$" "SHELL:--preload-file ${CMAKE_BINARY_DIR}/compressed_resources/\\1@/resources/\\1"
    OUTPUT_VARIABLE preload_file_args2
  )
  target_link_options(${PROJECT_NAME} PRIVATE
    --use-port=sdl2
    --use-port=sdl2_mixer
    -sSDL2_MIXER_FORMATS=wav,mp3
    --use-port=sdl2_image
    -sMIN_WEBGL_VERSION=2
    -sMAX_WEBGL_VERSION=2
    -sOFFSCREEN_FRAMEBUFFER=1
    -sALLOW_MEMORY_GROWTH
    -sINITIAL_MEMORY=67108864
    -sLZ4=1 # GitHub Pages doesn't use HTTP compression for the .data file
    -sEXPORTED_FUNCTIONS=_invokeMain,_setFullscreen,_setPause
    -sEXPORTED_RUNTIME_METHODS=setValue
    -Wno-unused-main # We don't invoke main directly from JS
    -sNO_MINIFY_HTML # Minification doesn't work out-of-the-box with Nix's emscripten
    --use-preload-plugins
    "SHELL:--preload-file ${CMAKE_BINARY_DIR}/PhongShader_vertex.glsl@/resources/PhongShader_vertex.glsl"
    "SHELL:--preload-file ${CMAKE_BINARY_DIR}/PhongShader_fragment.glsl@/resources/PhongShader_fragment.glsl"
    ${preload_file_args}
    ${preload_file_args2}
    "--shell-file=${CMAKE_CURRENT_SOURCE_DIR}/shell.html"
  )
  list(
    TRANSFORM bundled_resources
    PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/resources/"
    OUTPUT_VARIABLE absolute_bundled_resources
  )
  set_target_properties(${PROJECT_NAME} PROPERTIES
    LINK_DEPENDS "${absolute_bundled_resources};${CMAKE_CURRENT_SOURCE_DIR}/shell.html"
    ADDITIONAL_CLEAN_FILES "${PROJECT_NAME}.data;${PROJECT_NAME}.js;${PROJECT_NAME}.wasm"
  )
  set(CMAKE_EXECUTABLE_SUFFIX .html)

  if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    foreach(r IN LISTS web_resources)
      file(CREATE_LINK "${CMAKE_CURRENT_SOURCE_DIR}/resources/${r}" "${r}" SYMBOLIC)
    endforeach()
  else()
    include(GNUInstallDirs)
    install(TARGETS ${PROJECT_NAME} DESTINATION "${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}")
    install(
      FILES
        "$<LIST:TRANSFORM,.wasm;.js;.data,PREPEND,${CMAKE_BINARY_DIR}/${PROJECT_NAME}>"
        "$<LIST:TRANSFORM,${web_resources},PREPEND,${CMAKE_CURRENT_SOURCE_DIR}/resources/>"
      DESTINATION "${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}"
    )
  endif()
else ()

find_package(GLEW       REQUIRED)
find_package(SDL2       CONFIG REQUIRED)
find_package(SDL2_image CONFIG REQUIRED)
find_package(SDL2_mixer CONFIG REQUIRED)
find_package(SDL2_ttf   CONFIG REQUIRED)
find_package(OpenGL     REQUIRED)

set(CMAKE_BUILD_TYPE RelWithDebInfo) # add debugging information, optimize

add_executable(${PROJECT_NAME} WIN32 ${sources})
target_link_libraries(${PROJECT_NAME} PRIVATE
  GLEW::GLEW
  $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
  $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
  $<IF:$<TARGET_EXISTS:SDL2_image::SDL2_image>,SDL2_image::SDL2_image,SDL2_image::SDL2_image-static>
  $<IF:$<TARGET_EXISTS:SDL2_mixer::SDL2_mixer>,SDL2_mixer::SDL2_mixer,SDL2_mixer::SDL2_mixer-static>
  $<IF:$<TARGET_EXISTS:SDL2_ttf::SDL2_ttf>,SDL2_ttf::SDL2_ttf,SDL2_ttf::SDL2_ttf-static>
)

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  file(INSTALL resources DESTINATION .)
else()
  include(GNUInstallDirs)
  add_compile_definitions(CMAKE_INSTALL_FULL_DATADIR="${CMAKE_INSTALL_FULL_DATADIR}")
  install(TARGETS ${PROJECT_NAME} RUNTIME)
  if(WIN32)
    install(FILES $<TARGET_RUNTIME_DLLS:${PROJECT_NAME}> TYPE BIN)
    # While some transitive dependencies are included in TARGET_RUNTIME_DLLS,
    # many aren't. We can use the install-time GET_RUNTIME_DEPENDENCIES tool to
    # get their dependencies. (This will be a bit redundant, but that's okay.)
    install(CODE "
      message(CHECK_START [[Detecting additional runtime library dependencies]])
      file(GET_RUNTIME_DEPENDENCIES
        RESOLVED_DEPENDENCIES_VAR resolved_dlls
        UNRESOLVED_DEPENDENCIES_VAR unresolved_dlls
        LIBRARIES $<TARGET_RUNTIME_DLLS:${PROJECT_NAME}>
        POST_EXCLUDE_REGEXES [[[/\\\\]system32[/\\\\].*\\.dll]]
      )
      list(REMOVE_ITEM resolved_dlls $<TARGET_RUNTIME_DLLS:${PROJECT_NAME}>)
      list(LENGTH resolved_dlls resolved_dlls_count)
      message(CHECK_PASS \"done (\${resolved_dlls_count} found)\")
      file(INSTALL \${resolved_dlls} DESTINATION \"${CMAKE_INSTALL_FULL_BINDIR}\")
    ")
  endif()
  install(DIRECTORY resources TYPE DATA)
endif()

endif()

# nix packages
#   cmake
#   glew
#   SDL2
#   SDL2_mixer
#   SDL2_ttf
#   SDL2_image

# apt-get packages
#   cmake
#   libglew-dev
#   libsdl2-dev
#   libsdl2-mixer-dev
#   libsdl2-ttf-dev
#   libsdl2-image-dev

# brew packages
#   cmake
#   sdl2
#   sdl2_mixer
#   sdl2_ttf
#   sdl2_image
#   glew

# vcpkg packages
#   sdl2
#   sdl2_mixer[core,mpg123]
#   sdl2_ttf
#   sdl2_image[core,libjpeg-turbo]
#   glew
