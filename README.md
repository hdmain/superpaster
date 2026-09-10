# Spaste

[![Build .deb](https://github.com/hdmain/superpaster/actions/workflows/build-deb.yml/badge.svg)](https://github.com/hdmain/superpaster/actions/workflows/build-deb.yml)

Lightweight Linux clipboard history manager built with **C++20** and **Qt 6**.

Launching Spaste opens the **settings** window. Press **Super+V** for a frameless overlay of the 10 most recent clipboard items. History lives in RAM only and is never written to disk.

## Install (.deb)

Download the latest `.deb` from [Releases](https://github.com/hdmain/superpaster/releases) or from the [Actions](https://github.com/hdmain/superpaster/actions/workflows/build-deb.yml) artifacts, then:

```bash
sudo apt install ./spaste_*.deb
```

CI builds a Debian package on every push to `main`. Publishing a GitHub Release attaches the `.deb` automatically.

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

- **Super+V** uses an X11 global grab. On pure Wayland sessions without an XWayland display, register the same shortcut in your compositor to run/raise Spaste, or use **Open overlay** in settings.
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
