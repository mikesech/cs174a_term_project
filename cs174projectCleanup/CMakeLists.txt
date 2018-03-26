cmake_minimum_required (VERSION 2.8)
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
pkg_search_module(SDL       REQUIRED sdl)
pkg_search_module(SDL_MIXER REQUIRED SDL_mixer)
pkg_search_module(GLEW      REQUIRED glew)
pkg_search_module(IL        REQUIRED IL)
find_package     (OpenGL    REQUIRED)
find_package     (GLUT      REQUIRED)

include_directories(
  ${SDL_INCLUDE_DIRS}
  ${OPENGL_INCLUDE_DIR}
  ${GLUT_INCLUDE_DIR}
  ${GLEW_INCLUDE_DIRS}
  ${SDL_MIXER_INCLUDE_DIRS}
  ${IL_INCLUDE_DIRS}
)

set(CMAKE_BUILD_TYPE RelWithDebInfo) # add debugging information, optimize

add_executable(${PROJECT_NAME} ${sources})
target_link_libraries(${PROJECT_NAME}
  ${SDL_LDFLAGS}
  ${OPENGL_LIBRARIES}
  ${GLUT_LIBRARIES}
  ${GLEW_LDFLAGS}
  ${SDL_MIXER_LDFLAGS}
  ${IL_LDFLAGS}
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

# brew packages
#   cmake
#   pkg-config
#   sdl
#   sdl_mixer
#   glew
#   devil
#   freeglut
