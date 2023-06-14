{
  description = "Fall 2011 UCLA 174A Graphics I Term Project";

  outputs = { self, nixpkgs }: {

    packages.aarch64-darwin.cs174a_term_project =
      with nixpkgs.legacyPackages.aarch64-darwin;
      stdenv.mkDerivation {
        name = "cs174a_term_project";
        src = ./cs174projectCleanup;
        nativeBuildInputs = [
          cmake
          pkg-config
        ];
        buildInputs = [
          libdevil-nox
          glew
          SDL2
          SDL2_mixer
          SDL2_ttf
        ];
      };

    packages.aarch64-darwin.default =
      self.packages.aarch64-darwin.cs174a_term_project;

  };
}
