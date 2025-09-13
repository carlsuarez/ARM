#!/bin/bash

# Build kernel and libraries
echo "Building kernel and libraries..."
cd kernel
make clean
make || exit 1
cd ..

# Build user program
echo "Building user libraries..."
cd user
make clean
make || exit 2
cd ..

echo "Building user program..."
cd home
make clean
make || exit 3
cd ..

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
sudo cp build/libuser.so $MNT_DIR
sudo cp build/home/main.elf $MNT_DIR

# Unmount and clean up
sudo umount $MNT_DIR
sudo chown -R $(whoami):$(id -gn) $IMG $MNT_DIR

echo "Build complete. Image created at $IMG"
