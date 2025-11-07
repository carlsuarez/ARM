# ARM OS Project on IntegratorCP / ARM926EJ-S

## Overview
This repository implements a simple operating system kernel and supporting components for the ARM926EJ-S processor, running on the IntegratorCP board (or equivalent emulated via QEMU).  
The project is developed as part of a Computer Engineering course at the University of Florida.

## Repository Structure
- **common/** — Common utilities, constants, and architecture-specific definitions  
- **include/** — Header files for the kernel, user code, and system interfaces  
- **kernel/** — Core kernel source code: initialization, scheduling (if any), system calls, and interrupt handling  
- **home/** — Optional directory for user applications or sample code  
- **scripts/** — Build scripts, toolchain invocation, and QEMU launch scripts  
- **user/** — User-mode programs that run on top of the kernel  
- **.gitignore** — Standard ignores for build artifacts and temporary files

## Features
- Bootloader and early system initialization for ARM926EJ-S  
- Exception and interrupt vector setup  
- Basic memory management (if implemented)  
- Kernel and user-mode separation with syscalls and traps  
- Simple user-mode program execution  
- Emulation support using QEMU

## Getting Started

### Prerequisites
- ARM cross-compiler `arm-none-eabi-gcc`
- GNU Make
- QEMU with ARM support
- Linux or WSL environment

### Build and Run
# Clone the repository
git clone https://github.com/carlsuarez/ARM.git
cd ARM

# Build and Run in QEMU
./scripts/run.sh
