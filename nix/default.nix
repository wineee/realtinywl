{ stdenv
, lib
, nix-filter
, cmake
, pkg-config
, wayland-scanner
, qttools
, wrapQtAppsHook
, qtbase
, qwlroots
, waylib
, wayland
, wayland-protocols
, wlr-protocols
, pixman
, libinput
}:

stdenv.mkDerivation (finalAttrs: {
  pname = "waygreet";
  version = "0.0.1";

  src = nix-filter.filter {
    root = ./..;

    exclude = [
      ".git"
      "LICENSES"
      "README.md"
      (nix-filter.matchExt "nix")
    ];
  };

  nativeBuildInputs = [
    cmake
    pkg-config
    qttools
    wayland-scanner
    wrapQtAppsHook
  ];

  buildInputs = [
    qtbase
    qwlroots
    waylib
    wayland
    wayland-protocols
    wlr-protocols
    pixman
    libinput
  ];

  meta = {
    description = "Wayland greeter for greetd";
    homepage = "https://github.com/wineee/WayGreet";
    license = with lib.licenses; [ gpl3Plus ];
    platforms = lib.platforms.linux;
    maintainers = with lib.maintainers; [ rewine ];
  };
})

