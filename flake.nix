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
      buildEmscriptenPackage = prev.buildEmscriptenPackage.override {
        pkgs = final // { stdenv = final.stdenvNoCC; };
      };
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

    packages.emscripten =
      with pkgs;
      let
        emscriptenCache = import ./emscripten-cache.nix {
          inherit pkgs;
          fetchTargets = "sdl2_image-jpg sdl2_image-png sdl2_ttf sdl2_mixer_mp3";
          fetchHash = "sha256-1iiGqOrDesdGKC2xTHIM1+2LZfP0IgUBKamihool6qQ=";
          buildArguments = "-O3 --use-port=sdl2 --use-port=sdl2_ttf --use-port=sdl2_mixer -sSDL2_MIXER_FORMATS=wav,mp3 --use-port=sdl2_image:formats=png,jpg -sMIN_WEBGL_VERSION=2 -sMAX_WEBGL_VERSION=2 -sOFFSCREEN_FRAMEBUFFER=1";
        };
      in buildEmscriptenPackage {
        name = "cs174a_term_project";
        src = ./cs174projectCleanup;
        nativeBuildInputs = [
          cmake
          emscriptenCache
          removeReferencesTo
        ];
        allowedReferences = [];

        configurePhase = ''
          mkdir build
          emcmake cmake -DCMAKE_BUILD_TYPE=Release -S . -B build -DCMAKE_INSTALL_PREFIX="$out"
        '';

        buildPhase = ''
          EM_CACHE=${emscriptenCache} EM_FROZEN_CACHE=1 emmake make -C build -j$(nproc)
        '';

        installPhase = ''
          make -C build install
        '';

        fixupPhase = ''
          remove-references-to -t ${emscripten} -t ${emscriptenCache} "$out"/share/cs174a_term_project/cs174a_term_project.wasm
        '';

        checkPhase = "";
      };

    # We provide an explicit devShells definition so that we don't
    # use the cache from above in a dev shell.
    devShells.emscripten =
      with pkgs;
      mkShellNoCC {
        packages = [
          cmake
          emscripten
        ];

        shellHook = ''export EM_CACHE="$HOME/.emscripten_cache"'';
      };

    apps.emscripten = {
      type = "app";
      program = with pkgs; toString (writeShellScript "emscriptenRunWrapper" "${emscripten}/bin/emrun ${packages.emscripten}/share/cs174a_term_project/cs174a_term_project.html \"$@\"");
    };
  });
}
