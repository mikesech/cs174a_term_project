cmake_minimum_required (VERSION 3.6)
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
  world.cpp                   WorldEntity.cpp
)
# Source files omitted: obj_loader/objTester.cpp test_main.cpp

include(FindPkgConfig)
pkg_check_modules(SDL        REQUIRED IMPORTED_TARGET sdl)
pkg_check_modules(SDL_MIXER  REQUIRED IMPORTED_TARGET SDL_mixer)
pkg_check_modules(SDL_TTF    REQUIRED IMPORTED_TARGET SDL_ttf)
pkg_check_modules(GLEW       REQUIRED IMPORTED_TARGET glew)
pkg_check_modules(IL         REQUIRED IMPORTED_TARGET IL)
find_package     (OpenGL     REQUIRED)
find_package     (GLUT       REQUIRED)

# XXX: We aren't adding the non-include cflags. Is that a problem?

include_directories(
  ${GLUT_INCLUDE_DIR}
)

set(CMAKE_BUILD_TYPE RelWithDebInfo) # add debugging information, optimize

add_executable(${PROJECT_NAME} ${sources})
target_link_libraries(${PROJECT_NAME}
  OpenGL::GL
  ${GLUT_LIBRARIES}
  PkgConfig::SDL
  PkgConfig::SDL_MIXER
  PkgConfig::SDL_TTF
  PkgConfig::GLEW
  PkgConfig::IL
)

file(INSTALL resources DESTINATION .)

# apt-get packages
#   cmake
#   pkg-config
#   freeglut3-dev
#   libdevil-dev
#   libglew-dev
#   libsdl1.2-dev
#   libsdl-mixer1.2-dev
#   libsdl-ttf2.0-dev

# brew packages
#   cmake
#   pkg-config
#   sdl
#   sdl_mixer
#   sdl_ttf
#   glew
#   devil
