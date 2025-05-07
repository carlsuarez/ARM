#!/bin/bash

set -e  # Exit on error
set -u  # Exit on undefined variables

# Config
WORK_DIR="./image"
IMAGE="$WORK_DIR/fat32.img"
MOUNT_DIR="$WORK_DIR/mount"
LABEL="MYDISK"
SIZE_MB=32

# Text files to write
declare -A FILES=(
    ["hello.txt"]="Hello from FAT32!"
    ["notes.txt"]=$'These are some random notes.\nLine 2 of notes.'
    ["log.txt"]=$'Log start...\nEvent 1: OK\nEvent 2: OK\nLog end.'
)

cleanup() {
    echo "[*] Cleaning up..."
    if mountpoint -q "$MOUNT_DIR"; then
        sudo umount "$MOUNT_DIR" || true
    fi
    if [ -d "$MOUNT_DIR" ]; then
        sudo rmdir "$MOUNT_DIR" || true
    fi
}

trap cleanup EXIT

echo "[*] Setting up workspace in $WORK_DIR..."
mkdir -p "$WORK_DIR"
mkdir -p "$MOUNT_DIR"

echo "[*] Creating ${SIZE_MB}MB FAT32 image at $IMAGE..."
dd if=/dev/zero of="$IMAGE" bs=1M count=$SIZE_MB status=progress

echo "[*] Formatting image as FAT32 with label '$LABEL'..."
mkfs.vfat -F 32 -n "$LABEL" "$IMAGE"

echo "[*] Mounting image at $MOUNT_DIR..."
sudo mount -o loop,uid=$(id -u),gid=$(id -g) "$IMAGE" "$MOUNT_DIR"

echo "[*] Writing text files to image..."
for filename in "${!FILES[@]}"; do
    echo -e "${FILES[$filename]}" > "$MOUNT_DIR/$filename"
    # Set file timestamps to a consistent value
    touch -t 202301010000 "$MOUNT_DIR/$filename"
done

echo "[*] Syncing file system..."
sync "$MOUNT_DIR"

echo "[✔] FAT32 image created at $IMAGE with files inside."
ls -lh "$IMAGE"