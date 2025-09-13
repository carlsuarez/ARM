#!/bin/bash

# Build everything
./scripts/build.sh || exit 1

# Run QEMU
echo "Starting QEMU..."
qemu-system-arm \
    -M integratorcp \
    -cpu arm926 \
    -kernel build/kernel.elf \
    -drive file=image/fat32.img,format=raw,if=sd \
    -nographic \
    -serial mon:stdio \
    -audiodev none,id=snd0 \
    -D ./qemu.log \
    -d in_asm,mmu,guest_errors,unimp,int