{
  description = "Reproducible Clang environment and builds for SimCourse";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-26.05";
    cache-research = {
      url = "github:timoko3/cacheResearch";
      inputs.nixpkgs.follows = "nixpkgs";
    };
  };

  outputs =
    inputs@{ nixpkgs, ... }:
    let
      systems = [
        "x86_64-linux"
        "aarch64-linux"
      ];
      forAllSystems = nixpkgs.lib.genAttrs systems;
      pkgsFor =
        system:
        import nixpkgs {
          inherit system;
          overlays = [ inputs.cache-research.overlays.default ];
        };
      dependenciesFor =
        system:
        let
          pkgs = pkgsFor system;
        in
        {
          cli11 = pkgs.cli11;
          elfio = (pkgs.elfio.override { stdenv = pkgs.clangStdenv; }).overrideAttrs {
            cmakeFlags = [
              "-DELFIO_BUILD_EXAMPLES=OFF"
              "-DELFIO_BUILD_TESTS=OFF"
            ];
            nativeCheckInputs = [ ];
            doCheck = false;
          };
          gtest = pkgs.gtest.override { stdenv = pkgs.clangStdenv; };
          llvm = pkgs.llvmPackages_23.llvm;
          cache = pkgs.cache-research-library;
        };
      packagesFor =
        system:
        let
          pkgs = pkgsFor system;
          dependencies = dependenciesFor system;
          mkSimulator =
            pkgs.clangStdenv.mkDerivation {
              pname = "simcourse";
              version = "1.0.0";
              src = pkgs.lib.fileset.toSource {
                root = ./.;
                fileset = pkgs.lib.fileset.unions [
                  ./CMakeLists.txt
                  ./main.cpp
                  ./simulator.hpp
                  ./isa
                  ./toyFunctionalSim
                  ./tests
                ];
              };
              nativeBuildInputs = [
                pkgs.cmake
                pkgs.ninja
              ];
              buildInputs = [
                dependencies.cli11
                dependencies.elfio
                dependencies.cache
                dependencies.gtest
                dependencies.llvm
                dependencies.llvm.dev
              ];
              cmakeFlags = [
                "-DCMAKE_BUILD_TYPE=Release"
                "-DBUILD_TESTING=ON"
                "-DFETCHCONTENT_FULLY_DISCONNECTED=ON"
              ];
              doCheck = true;
              checkPhase = ''
                runHook preCheck
                ctest --output-on-failure --no-tests=error --parallel "$NIX_BUILD_CORES"
                runHook postCheck
              '';
              meta = {
                description = "Functional ISA simulator";
                mainProgram = "simcourse";
                platforms = systems;
              };
            };
        in
        {
          default = mkSimulator;
        };
    in
    {
      packages = forAllSystems packagesFor;
      checks = forAllSystems (system: {
        build = (packagesFor system).default;
      });
      devShells = forAllSystems (
        system:
        let
          pkgs = pkgsFor system;
        in
        {
          default = (pkgs.mkShell.override { stdenv = pkgs.clangStdenv; }) {
            inputsFrom = [ (packagesFor system).default ];
            packages = [
              pkgs.git
              pkgs.python3
              pkgs.ruby
              pkgs.llvmPackages_23.clang-tools
              pkgs.nixfmt
            ];
          };
        }
      );
      formatter = forAllSystems (system: (pkgsFor system).nixfmt-tree);
    };
}
