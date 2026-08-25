#!/usr/bin/env bash
set -e

# xv6-riscv helper build script
# Licensed under the Apache License, Version 2.0 (the "License"); 
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#     http://www.apache.org/licenses/LICENSE-2.0

usage() {
    echo "Usage: $0 {build|image|qemu|qemu-gdb|clean|distclean}"
    exit 1
}

case "$1" in
    build)
        echo "==> Compiling xv6-riscv kernel..."
        make
        ;;
    image)
        echo "==> Building xv6-riscv bootable image..."
        make
        make fs.img

        echo "==> Creating combined xv6.img (kernel + filesystem)..."
        KERNEL_SIZE=$(stat -c%s kernel/kernel)
        PADDED_KERNEL_SIZE=$(( ((KERNEL_SIZE + 0x1FFFFF) / 0x200000) * 0x200000 ))
        FS_SIZE=$(stat -c%s fs.img)
        TOTAL=$(( PADDED_KERNEL_SIZE + FS_SIZE ))

        truncate -s $TOTAL xv6.img
        dd if=kernel/kernel of=xv6.img bs=4096 conv=notrunc
        dd if=fs.img of=xv6.img bs=4096 seek=$((PADDED_KERNEL_SIZE / 4096)) conv=notrunc
        rm -f fs.img

        echo "==> Bootable image created: xv6.img ($(du -h xv6.img | cut -f1))"
        echo "==> Kernel: ${KERNEL_SIZE} bytes (padded to ${PADDED_KERNEL_SIZE})"
        echo "==> Filesystem: ${FS_SIZE} bytes at offset ${PADDED_KERNEL_SIZE}"
        echo "==> To boot:"
        echo "    qemu-system-riscv64 -machine virt -bios none -kernel xv6.img \\"
        echo "      -m 128M -smp 3 -nographic \\"
        echo "      -global virtio-mmio.force-legacy=false \\"
        echo "      -drive file=xv6.img,if=none,format=raw,id=x0,offset=$PADDED_KERNEL_SIZE \\"
        echo "      -device virtio-blk-device,drive=x0,bus=virtio-mmio-bus.0"
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
        rm -f fs.img xv6.img .gdbinit
        ;;
    transfer)
        echo "==> Transferring the image files to destdir : $DESTDIR"
        
        cp -v ./fs.img "$DESTDIR/"
        cp -v ./kernel/kernel "$DESTDIR/xv6-kernel"
        echo "==> Transfer complete."
        ;;
    push)
        echo "==> Pushing to github"
        git push origin master
        ;;
    *)
        usage
        ;;
esac