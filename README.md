# mFile

mFile is a native Win32 file manager for Windows, inspired by Windows Explorer and written in C++.

## Features

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

## Download

Prebuilt binaries are available on the [Releases](https://github.com/icreaterandomapps/mfile/releases) page.

## Requirements

- Windows (desktop)
- For building from source:
  - **MinGW.org GCC 6.3.0** (with `g++` and `windres`)
  - Windows development libraries available in the MinGW environment

## Building

1. Install **MinGW.org GCC 6.3.0** and ensure its `bin` directory is in your `PATH`.
2. From the project directory, run:

```bat
build.bat
```

This compiles resources (`mfile.rc`) and builds `mfile.exe` (statically links libgcc/libstdc++ where possible).

## Notes

- 7-Zip is optional. If available, mFile can use it for additional archive formats and GUI-driven operations; without it, built-in ZIP extraction is used for `.zip` files.
- When a directory contains more items than the safety cap, mFile shows a partial listing and indicates this in the status bar.

## Screenshots

_Screenshots to be added._

