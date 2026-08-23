#!/usr/bin/env bash
set -e

# xv6-riscv helper build script
# Licensed under the Apache License, Version 2.0 (the "License"); 
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#     http://www.apache.org/licenses/LICENSE-2.0

usage() {
    echo "Usage: $0 {build|qemu|qemu-gdb|clean|distclean}"
    exit 1
}

case "$1" in
    build)
        echo "==> Compiling xv6-riscv kernel..."
        make
        ;;
    qemu)
        echo "==> Building and running xv6 in QEMU..."
        make qemu
        ;;
    qemu-gdb)
        echo "==> Starting QEMU in GDB debug mode (waiting for connection)..."
        make qemu-gdb
        ;;
    clean)
        echo "==> Cleaning object files and build artifacts..."
        make clean
        ;;
    distclean)
        echo "==> Performing deep clean (removing build dir & disk images)..."
        make clean
        rm -f fs.img .gdbinit
        ;;
    transfer)
        echo "==> Transferring the image files to destdir : $DESTDIR"
        
        cp -v ./fs.img "$DESTDIR/"
        cp -v ./kernel/kernel "$DESTDIR/xv6-kernel"
        echo "==> Transfer complete."
        ;;
    *)
        usage
        ;;
esac