# mFile

mFile is a lightweight file manager inspired by Windows Explorer, written in C++.

- **Windows:** native Win32 application (`mfile.cpp`, `mfile.rc`, `build.bat`)
- **Linux:** native GTK3 port available in the [`linux/`](linux/) directory (not feature-identical to the Windows version)

## Features (Windows)

- Explorer-style interface with a folder tree sidebar and details view
- **This PC** view (default on startup) for browsing drives
- Navigation history (Back/Forward/Up), address bar, and refresh
- File operations: open, copy, cut, paste, delete, rename, create new folder, and view properties
- Drag and drop support
- Context menus for files, folders, and background actions (including copy name/path, open in Explorer/Command Prompt)
- Sorting and grouping options (name, date, type, size), including date-based groups in Downloads
- Search/filter within the current folder and recursive search (Enter/Find)
- Dark mode toggle
- Embedded app/toolbar/sidebar icons for portable use
- Archive support:
  - Built-in ZIP extraction fallback (Shell ZIP)
  - Optional [7-Zip](https://www.7-zip.org/) integration for extract/compress/browse of supported archive formats, GUI operations, and password prompts when using 7-Zip
- Large-folder handling: directory listings are capped (up to 50,000 items shown) with a status notice to avoid excessive memory/UI load on very large folders or drives

## Download (Windows)

Prebuilt Windows binaries may be available on the [Releases](https://github.com/icreaterandomapps/mfile/releases) page.

## Building (Windows)

### Requirements
- Windows (desktop)
- **MinGW.org GCC 6.3.0** (with `g++` and `windres`) in your `PATH`

### Build
From the project directory, run:
```bat
build.bat
```

This compiles resources (`mfile.rc`) and builds `mfile.exe` (statically links libgcc/libstdc++ where possible).

## Linux

A native Linux (GTK3) port is available in [`linux/`](linux/), split into two directories:

- **[`linux/debian-ubuntu/`](linux/debian-ubuntu/)** — Debian/Ubuntu build files and (when available) a prebuilt x86_64 binary, plus build/run instructions.
- **[`linux/arch/`](linux/arch/)** — Arch Linux build files/source (`Makefile`, `PKGBUILD`, etc.).

**Arch users:** you must **compile mFile yourself on Arch Linux** for proper compatibility. See [`linux/arch/README.md`](linux/arch/README.md) for dependencies and build instructions.

**Debian/Ubuntu users:** see [`linux/debian-ubuntu/README.md`](linux/debian-ubuntu/README.md) for dependencies, build instructions, and prebuilt binary notes.

> The Linux port provides core file manager functionality but does not have full feature parity with the Windows version.

## Notes

- 7-Zip (Windows) is optional. If available, mFile can use it for additional archive formats and GUI-driven operations; without it, built-in ZIP extraction is used for `.zip` files.
- When a directory contains more items than the safety cap, mFile shows a partial listing and indicates this in the status bar.

## Screenshots

_Screenshots to be added._

