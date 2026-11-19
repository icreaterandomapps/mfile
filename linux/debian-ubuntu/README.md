# mFile (Linux) — Debian / Ubuntu

This folder contains the **Debian/Ubuntu** side of the Linux port of mFile (GTK3).

## Prebuilt binary (Debian/Ubuntu)

A prebuilt binary may be included:
- `mfile-linux-amd64-debian-ubuntu` (x86_64, dynamically linked, built on Ubuntu/Debian)

Run it (on a compatible Debian/Ubuntu system with required GTK3 runtime libraries):
```bash
chmod +x ./mfile-linux-amd64-debian-ubuntu
./mfile-linux-amd64-debian-ubuntu
```

> Compatibility depends on your system's GTK3/glibc/library versions. If it does not run on your system, build it locally using the steps below.

## Build locally (Debian / Ubuntu)

### Dependencies
```bash
sudo apt update
sudo apt install build-essential pkg-config libgtk-3-dev
```

### Build
```bash
make
# or
chmod +x build.sh
./build.sh
```

### Run
```bash
./mfile
```

## Notes
- This is a native Linux (GTK3) port and does not have full feature parity with the Windows version.
- `xdg-open` is used to open files; `gio` is used for trash support when available.
