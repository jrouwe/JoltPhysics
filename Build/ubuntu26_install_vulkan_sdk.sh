#!/bin/bash
# Installs the Vulkan SDK from the LunarG Linux tarball (the Ubuntu apt packages are no longer updated).
# Usage: ubuntu26_install_vulkan_sdk.sh [version], e.g. ubuntu26_install_vulkan_sdk.sh 1.4.363.1
# If no version is given, 1.4.363.1 is installed.
# The SDK is extracted to ~/vulkan/<version>.
set -e

VULKAN_VERSION=${1:-1.4.363.1}
VULKAN_DIR=$HOME/vulkan

# Mesa provides the Vulkan drivers, the SDK tarball does not include them
# libx11-dev provides the X11 headers used by TestFramework (the vulkan-sdk apt package used to pull these in)
sudo apt update
sudo apt install -y mesa-vulkan-drivers libx11-dev

# Download and extract the SDK
mkdir -p "$VULKAN_DIR"
wget -O "$VULKAN_DIR/vulkansdk-linux-x86_64-$VULKAN_VERSION.tar.xz" "https://sdk.lunarg.com/sdk/download/$VULKAN_VERSION/linux/vulkansdk-linux-x86_64-$VULKAN_VERSION.tar.xz"
tar -xf "$VULKAN_DIR/vulkansdk-linux-x86_64-$VULKAN_VERSION.tar.xz" -C "$VULKAN_DIR"

echo "Vulkan SDK $VULKAN_VERSION installed in $VULKAN_DIR/$VULKAN_VERSION"
echo "To set up the environment, run: source $VULKAN_DIR/$VULKAN_VERSION/setup-env.sh"
echo "Run this in each new shell that needs the SDK (or add it to ~/.bashrc)."
