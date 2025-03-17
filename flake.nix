{
  description = "A basic flake to help develop treeland";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
    nix-filter.url = "github:numtide/nix-filter";
  };

  outputs =
    {
      self,
      nixpkgs,
      flake-utils,
      nix-filter,
    }@input:
    flake-utils.lib.eachSystem [ "x86_64-linux" "aarch64-linux" "riscv64-linux" ] (
      system:
      let
        pkgs = nixpkgs.legacyPackages.${system};

        waygreet = pkgs.qt6Packages.callPackage ./nix {
          nix-filter = nix-filter.lib;
        };
      in
      {
        packages.default = waygreet;

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
              export QT_PLUGIN_PATH=${
                makeQtpluginPath (
                  with pkgs.qt6;
                  [
                    qtbase
                    qtdeclarative
                    qtsvg
                  ]
                )
              }
              export QML2_IMPORT_PATH=${makeQmlpluginPath (with pkgs; with qt6; [ qtdeclarative ])}
              export QML_IMPORT_PATH=$QML2_IMPORT_PATH
            '';
        };

        apps.default = {
          type = "app";
          program = "${self.nixosConfigurations."${system}".vm.config.system.build.vm}/bin/run-nixos-vm";
        };

        nixosConfigurations.vm = nixpkgs.lib.nixosSystem {
          inherit system;
          modules = [

            {
              imports = [ "${nixpkgs}/nixos/modules/virtualisation/qemu-vm.nix" ];

              services.greetd.enable = true;
              services.greetd.settings.default_session = {
                #command = "cage -s -mlast -- regreet";
                command = "waygreet";
                user = "greeter";
              };
              programs.regreet.enable = true;

              environment.systemPackages = with pkgs; [
                foot
                xterm
                cage
                greetd.regreet
                waygreet
              ];

              environment.variables = {
                "WLR_RENDERER" = "pixman";
              };

              programs.sway.enable = true;
              services.xserver = {
                enable = true;
                displayManager = {
                  session = [
                    {
                      manage = "desktop";
                      name = "xterm";
                      start = ''
                        ${pkgs.xterm}/bin/xterm -ls &
                        waitPID=$!
                      '';
                    }
                  ];
                };
              };

              users.users.test = {
                isNormalUser = true;
                uid = 1000;
                extraGroups = [
                  "wheel"
                  "networkmanager"
                ];
                password = "test";
              };
              virtualisation = {
                #qemu.options = [ "-vga virtio -display gtk,gl=on" ];
                cores = 4;
                memorySize = 4096;
                diskSize = 16384;
                resolution = {
                  x = 1024;
                  y = 768;
                };
              };
              system.stateVersion = "25.05";
            }
          ];
        };
      }
    );
}
