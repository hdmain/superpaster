# Spaste

[![Build and publish](https://github.com/hdmain/superpaster/actions/workflows/build-deb.yml/badge.svg)](https://github.com/hdmain/superpaster/actions/workflows/build-deb.yml)

**Download:** [hdmain.github.io/superpaster](https://hdmain.github.io/superpaster/)

Lightweight Linux clipboard history manager built with **C++20** and **Qt 6**.

Launching Spaste opens the **settings** window. Press **Super+V** for a frameless overlay of the 10 most recent clipboard items. History lives in RAM only and is never written to disk.

## Install

Download from the site:

**https://hdmain.github.io/superpaster/**

- `.deb` package — `sudo apt install ./spaste-latest.deb`
- standalone Linux binary — `chmod +x spaste-linux-x86_64 && ./spaste-linux-x86_64`

If Qt packages are missing on your distro, install them first:

```bash
sudo apt install libqt6core6 libqt6gui6 libqt6widgets6 libqt6svg6 libqt6svgwidgets6 qt6-qpa-plugins
```

CI publishes the site, binary, and `.deb` on every push to `main`. GitHub Releases also receive the packages when you publish a tag.

## Features

- Settings window on startup (also via desktop launcher / tray)
- Overlay clipboard picker: frameless, fixed position, always on top
- Light & dark themes (default follows the system color scheme)
- Transparent, clean UI with Lucide SVG icons
- System tray quick access
- Keyboard navigation in the overlay (↑↓, Enter, Esc)

## Dependencies

On Debian/Ubuntu:

```bash
sudo apt install build-essential cmake \
  qt6-base-dev qt6-svg-dev \
  libx11-dev \
  fonts-inter   # optional
```

On Fedora:

```bash
sudo dnf install cmake gcc-c++ \
  qt6-qtbase-devel qt6-qtsvg-devel \
  libX11-devel
```

On Arch:

```bash
sudo pacman -S cmake base-devel qt6-base qt6-svg libx11
```

## Build

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j"$(nproc)"
```

Run:

```bash
./build/spaste
```

Install (optional):

```bash
sudo cmake --install build
```

Build a `.deb` locally:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr
cmake --build build -j"$(nproc)"
cd build && cpack -G DEB
```

This installs the binary, `spaste.desktop` launcher, and app icon so you can open Spaste from the application menu.

## Usage

| Action | Result |
|--------|--------|
| Start Spaste | Opens settings |
| Super + V | Toggle clipboard overlay |
| Click / Enter on an item | Copies it back to the clipboard |
| Esc / click outside | Closes the overlay |
| Theme combo | System / Light / Dark |

## Notes

- **Super+V** is registered through GNOME/Pop!_OS desktop keybindings on Wayland (X11 grabs do not receive Super keys there). Spaste also frees the default notification binding for Super+V while it runs.
- Clipboard contents are held in process memory only. Quitting Spaste clears history.
- Icons are Lucide SVGs bundled under `resources/icons/`.

## Project layout

```
src/
  app/           Application wiring, tray
  clipboard/     In-memory history (max 10)
  hotkeys/       Super+V (X11)
  theme/         System / light / dark QSS
  ui/            Settings + overlay windows
resources/
  icons/         SVG icons
  styles/        light.qss, dark.qss
packaging/       .desktop entry
.github/         CI workflow (build .deb)
```
