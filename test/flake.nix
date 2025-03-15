{
  inputs.waygreet.url = "..";

  outputs = inputs@{ self, waygreet }: let
    nixpkgs = waygreet.inputs.nixpkgs;
    system = "x86_64-linux";
    pkgs = import nixpkgs { inherit system; };
  in {
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
            waygreet.packages.${system}.default
        ];

        environment.variables = {
            "WLR_RENDERER" = "pixman";
        };

        programs.sway.enable = true;
        services.xserver = {
          enable = true;
          displayManager = {
            session = [{
              manage = "desktop";
              name = "xterm";
              start = ''
                ${pkgs.xterm}/bin/xterm -ls &
                waitPID=$!
              '';
            }];
          };
        };

        users.users.test = {
          isNormalUser = true;
          uid = 1000;
          extraGroups = [ "wheel" "networkmanager" ];
          password = "test";
        };
        virtualisation = {
          #qemu.options = [ "-vga virtio -display gtk,gl=on" ];
          cores = 4;
          memorySize = 4096;
          diskSize = 16384;
          resolution = { x = 1024; y = 768; };
        };
        system.stateVersion = "25.05";
      }];
    };
    packages.${system}.default = self.nixosConfigurations.vm.config.system.build.vm;
    apps.${system}.default = {
      type = "app";
      program = "${self.packages.${system}.default}/bin/run-nixos-vm";
    };
  };
}
