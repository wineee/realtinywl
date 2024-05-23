{
  description = "A basic flake to help develop treeland";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixpkgs-unstable";
    flake-utils.url = "github:numtide/flake-utils";
    nix-filter.url = "github:numtide/nix-filter";
    waylib = {
      url = "github:vioken/waylib";
      inputs.nixpkgs.follows = "nixpkgs";
      inputs.flake-utils.follows = "flake-utils";
      inputs.nix-filter.follows = "nix-filter";
    };
  };

  outputs = { self, nixpkgs, flake-utils, nix-filter, waylib }@input:
    flake-utils.lib.eachSystem [ "x86_64-linux" "aarch64-linux" "riscv64-linux" ]
      (system:
        let
          pkgs = nixpkgs.legacyPackages.${system};

          tinywl = pkgs.qt6Packages.callPackage ./nix {
            nix-filter = nix-filter.lib;
            waylib = waylib.packages.${system}.default;
          };
        in
        {
          packages.default = tinywl;

          devShells.default = pkgs.mkShell {
            inputsFrom = [
              self.packages.${system}.default
            ];

            shellHook =
              let
                makeQtpluginPath = pkgs.lib.makeSearchPathOutput "out" pkgs.qt6.qtbase.qtPluginPrefix;
                makeQmlpluginPath = pkgs.lib.makeSearchPathOutput "out" pkgs.qt6.qtbase.qtQmlPrefix;
              in
              ''
                #export QT_LOGGING_RULES="*.debug=true;qt.*.debug=false"
                #export WAYLAND_DEBUG=1
                export QT_PLUGIN_PATH=${makeQtpluginPath (with pkgs.qt6; [ qtbase qtdeclarative qtquick3d qtimageformats qtwayland qt5compat qtsvg ])}
                export QML2_IMPORT_PATH=${makeQmlpluginPath (with pkgs; with qt6; [ qtdeclarative qtquick3d qt5compat deepin.dtk6declarative ] )}
                export QML_IMPORT_PATH=$QML2_IMPORT_PATH
              '';
          };
        }
      );
}
