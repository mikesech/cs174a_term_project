{
  description = "Fall 2011 UCLA 174A Graphics I Term Project";
  inputs = {
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, flake-utils }:
  flake-utils.lib.eachDefaultSystem (system: {
    packages.cs174a_term_project =
      with nixpkgs.legacyPackages.${system};
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
  });
}
