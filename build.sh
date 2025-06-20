#!/bin/bash

# Clean previous builds
make -f Makefile.kernel clean
make -f Makefile.user clean

# Build kernel and libraries
echo "Building kernel and libraries..."
make -f Makefile.kernel all || exit 1

# Build user program
echo "Building user program..."
make -f Makefile.user all || exit 2

# Create FAT32 image
echo "Creating FAT32 image..."
IMG=image/fat32.img
MNT_DIR=image/mnt

mkdir -p $MNT_DIR
rm -f $IMG

# Create a 64MB image (131072 sectors × 512 bytes = 64MB)
dd if=/dev/zero of=$IMG bs=512 count=131072 status=none

# Format as FAT32 explicitly
sudo mkfs.vfat -F 32 $IMG

# Mount the image
sudo mount -o loop $IMG $MNT_DIR

# Copy files to image
echo "Copying files to image..."
sudo mkdir -p $MNT_DIR/libc
sudo cp build/libuser.so $MNT_DIR/libc/
sudo cp build/user/user.elf $MNT_DIR/
sudo cp test.txt $MNT_DIR

# Unmount and clean up
sudo umount $MNT_DIR
sudo chown -R $(whoami):$(id -gn) $IMG $MNT_DIR

echo "Build complete. Image created at $IMG"
