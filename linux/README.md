# mFile (Linux)

This directory is split into two sides:

- **`debian-ubuntu/`** — Debian/Ubuntu build files and a prebuilt x86_64 binary (built on Ubuntu/Debian, when available).
- **`arch/`** — Arch Linux build files (`Makefile`, `PKGBUILD`, source).

## Arch users
**Arch users must compile mFile themselves** on Arch Linux for proper compatibility. See [`arch/README.md`](arch/README.md).

## Debian / Ubuntu users
See [`debian-ubuntu/README.md`](debian-ubuntu/README.md) for dependencies, build instructions, and the prebuilt binary (if included).

## Notes
- Both sides contain the same GTK3 Linux port source (`mfile.cpp`) so each can be built independently.
- This Linux port is not feature-identical to the Windows Win32 version.
