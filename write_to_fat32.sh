#!/bin/bash

set -e  # Exit on error
set -u  # Exit on undefined variables

# Config
WORK_DIR="./image"
IMAGE="$WORK_DIR/fat32.img"
MOUNT_DIR="$WORK_DIR/mount"
LABEL="MYDISK"
SIZE_MB=32

# File contents
declare -A FILES=(
    ["hello.txt"]="Hello from FAT32!"
    ["notes.txt"]=$'These are some random notes.\nLine 2 of notes.'
    ["log.txt"]=$'Log start...\nEvent 1: OK\nEvent 2: OK\nLog end.'
    ["docs/info.txt"]="This is a nested info file."
    ["logs/2025/summary.txt"]="If you're visiting this page, you're likely here because you're searching for a random sentence. Sometimes a random word just isn't enough, and that is where the random sentence generator comes into play. By inputting the desired number, you can make a list of as many random sentences as you want or need. Producing random sentences can be helpful in a number of different ways. For writers, a random sentence can help them get their creative juices flowing. Since the topic of the sentence is completely unknown, it forces the writer to be creative when the sentence appears. There are a number of different ways a writer can use the random sentence for creativity. The most common way to use the sentence is to begin a story. Another option is to include it somewhere in the story. A much more difficult challenge is to use it to end a story. In any of these cases, it forces the writer to think creatively since they have no idea what sentence will appear from the tool."
    ["big/bigfile.txt"]="__BIGFILE__"
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

echo "[*] Writing files to image..."
for filepath in "${!FILES[@]}"; do
    fullpath="$MOUNT_DIR/$filepath"
    dirpath=$(dirname "$fullpath")
    mkdir -p "$dirpath"

    if [ "${FILES[$filepath]}" == "__BIGFILE__" ]; then
        echo "[+] Creating big file at $filepath (about 2KB)..."
        head -c 2048 /dev/urandom | base64 > "$fullpath"
    else
        echo -e "${FILES[$filepath]}" > "$fullpath"
    fi

    touch -t 202301010000 "$fullpath"
done

echo "[*] Syncing file system..."
sync "$MOUNT_DIR"

echo "[✔] FAT32 image created at $IMAGE with nested directories and large file content."
ls -lh "$IMAGE"
