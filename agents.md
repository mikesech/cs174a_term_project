# cs174a_term_project

cs174a_term_project is first/third-person shooter written as a final project for
the CS174A at UCLA. It is predominantly written in C++ (with some GLSL, HTML, and
JavaScript) and can be compiled into two forms:
- A native application using SDL and OpenGL
- A browser application using Emscripten and WebGL

Running the application requires a video display as well as a keyboard and either
a mouse or a gamepad. (Technically, a gamepad would be sufficient except that there's
no way to quit via the gamepad.) The `README` file in the repo root contains a control
guide.

## Build process

For both forms, CMake is used as the build system. A Nix flake (`flake.nix`) is also
present in the repo's root directory, which can be used to build both forms. Unless
Nix is already available, it's probably makes more sense to use CMake directly.

### Building the native application directly

The CMakeLists.txt file lives at `cs174projectCleanup/CMakeLists.txt`. At the bottom
of the file is a list of the project's build dependencies for the Nix, apt, and Homebrew
package managers.

After creating and entering a build directory, perform the following steps:
1. `cmake <path to CMakeLists.txt>`
2. `make`, optionally with a `-j` argument to allow for parallel building

Normally, the runtime resources will be copied into the build directory. As a result,
it will be possible to execute the resulting executable `cs174a_term_project` in its place.

If, instead, `CMAKE_INSTALL_PREFIX` is set when invoking `cmake`, the runtime resources
will not be copied. Instead, run `make install` to install the program to the prefix and
run it from there.

### Building the web application directly

The CMakeLists.txt file lives at `cs174projectCleanup/CMakeLists.txt`. The Emscripten
form of this project takes advantage of Emscripten's ports system for dependencies, so
they will be automatically installed as part of the build process. Only Emscripten
and CMake needs to be installed as a prerequisite.

After creating and entering a build directory, perform the following steps:
1. `emcmake cmake <path to CMakeLists.txt>`
2. `emmake make`, optionally with a `-j` argument to allow for parallel building

As with the native application, the runtime resources will normally be copied into
the build directory, so that the directory can be served directly. The application's
page will be called `cs174a_term_project.html`; no `index.html` will be generated.

If, instead, `CMAKE_INSTALL_PREFIX` is set when invoking `cmake`, the runtime resources
will not be copied. Instead, run `make install` to install the program to the prefix and
run it from there.

### Using Nix

The flake provides the following packages:
- `cs174a_term_project` (default): the native application
- `emscripten`: the web application
- `dev-environment`: when used with `nix shell`, provides a development environment for
building the web application
