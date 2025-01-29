{
  description = "Fall 2011 UCLA 174A Graphics I Term Project";
  inputs = {
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, flake-utils }:
  flake-utils.lib.eachDefaultSystem (system:
  let
    pkgs = (nixpkgs.legacyPackages.${system}.extend (final: prev: {
      jre = final.jre_headless;
    }));
  in rec {
    packages.cs174a_term_project =
      with pkgs;
      stdenv.mkDerivation {
        name = "cs174a_term_project";
        src = ./cs174projectCleanup;
        nativeBuildInputs = [
          cmake
          pkg-config
        ];
        buildInputs = [
          glew
          SDL2
          SDL2_mixer
          SDL2_ttf
          SDL2_image
        ];
      };

    packages.default =
      self.packages.${system}.cs174a_term_project;

    packages.emscriptenPortsCache =
      with pkgs;
      runCommand "emscriptenPortsCache" {
        nativeBuildInputs = [
          emscripten
          curl
          cacert
        ];
        outputHashMode = "recursive";
        outputHash = "sha256-B04JpZYEbLVLN6OH+5nJz12Iu5ks06/gPiOG8JCGtx8=";
      } ''
        mkdir cache
        touch dummy.c
        EM_CACHE=$(pwd)/cache emcc --use-port=sdl2 --use-port=sdl2_ttf --use-port=sdl2_mixer -sSDL2_MIXER_FORMATS=wav,mp3 --use-port=sdl2_image:formats=png,jpg -sMIN_WEBGL_VERSION=2 -sMAX_WEBGL_VERSION=2 -sOFFSCREEN_FRAMEBUFFER=1 dummy.c

        mkdir $out
        mv cache/ports $out
      '';

    packages.emscriptenCache =
      with pkgs;
      runCommand "emscriptenCache" {
        nativeBuildInputs = [ emscripten packages.emscriptenPortsCache ];
      } ''
        mkdir $out
        ln -s ${packages.emscriptenPortsCache}/ports $out/ports
        touch dummy.c
        EM_CACHE=$out emcc --use-port=sdl2 --use-port=sdl2_ttf --use-port=sdl2_mixer -sSDL2_MIXER_FORMATS=wav,mp3 --use-port=sdl2_image:formats=png,jpg -sMIN_WEBGL_VERSION=2 -sMAX_WEBGL_VERSION=2 -sOFFSCREEN_FRAMEBUFFER=1 dummy.c
      '';

    packages.emscripten =
      with pkgs;
      buildEmscriptenPackage {
        name = "cs174a_term_project";
        src = ./cs174projectCleanup;
        nativeBuildInputs = [
          cmake
          packages.emscriptenCache
        ];

        configurePhase = ''
          mkdir build
          emcmake cmake -S . -B build
        '';

        buildPhase = ''
          cp -R ${packages.emscriptenCache} cache
          chmod -R +w cache
          EM_CACHE=$(realpath cache) emmake make -C build -j
        '';

        installPhase = ''
          mkdir $out
          mv build/cs174a_term_project.data $out
          mv build/cs174a_term_project.js   $out
          mv build/cs174a_term_project.wasm $out
          mv build/cs174a_term_project.html $out
        '';

        checkPhase = "";
      };

    # We provide an explicit devShells definition so that we don't
    # use the cache from above in a dev shell.
    devShells.emscripten =
      with pkgs;
      mkShell {
        packages = [
          cmake
          emscripten
        ];

        shellHook = ''export EM_CACHE="$HOME/.emscripten_cache"'';
      };

    apps.emscripten = {
      type = "app";
      program = with pkgs; toString (writeShellScript "emscriptenRunWrapper" "${emscripten}/bin/emrun ${packages.emscripten}/cs174a_term_project.html \"$@\"");
    };
  });
}
