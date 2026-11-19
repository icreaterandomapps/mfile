# mFile (Linux) — Arch Linux

This folder contains the **Arch Linux** side of the Linux port of mFile (GTK3).

## Important (Arch users)
**You must compile mFile yourself on Arch Linux.**  
A binary built on Debian/Ubuntu is not guaranteed to work on Arch due to library/ABI differences, so use the source in this folder to build a native Arch binary.

## Build on Arch Linux

### Dependencies
```bash
sudo pacman -S --needed base-devel pkgconf gtk3
```

### Build
```bash
make
```

### Run
```bash
./mfile
```

## Packaging (optional)
A `PKGBUILD` is included in this folder. You can adapt/use it with your preferred Arch packaging workflow if you want to create/install a package locally.

## Notes
- This is a native Linux (GTK3) port and does not have full feature parity with the Windows version.
- `xdg-open` is used to open files; `gio` is used for trash support when available.
