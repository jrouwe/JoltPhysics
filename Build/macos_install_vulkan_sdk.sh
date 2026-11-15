#!/bin/bash
# Installs the Vulkan SDK for macOS.
# Usage: macos_install_vulkan_sdk.sh <installation directory> [version], e.g. macos_install_vulkan_sdk.sh ~/vulkan_sdk 1.4.363.1
# If no version is given, the latest version is installed.
# If the installation directory already contains the SDK (e.g. restored from a CI cache), the install is skipped.
set -e
if [ -z "$1" ]
then
	echo "Need to specify installation directory"
	exit 1
fi

VULKAN_VERSION=${2:-latest}

if [ -f "$1/setup-env.sh" ]; then
	echo "Vulkan SDK already present in $1, skipping install"
	exit 0
fi

VULKAN_TEMP=/tmp/vulkan_sdk_install
rm -rf ${VULKAN_TEMP}
mkdir ${VULKAN_TEMP}
curl -s -L -o ${VULKAN_TEMP}/vulkan_sdk.zip "https://sdk.lunarg.com/sdk/download/${VULKAN_VERSION}/mac/vulkan_sdk.dmg?Human=true"
unzip -q ${VULKAN_TEMP}/vulkan_sdk.zip -d ${VULKAN_TEMP}
${VULKAN_TEMP}/vulkansdk-macOS*.app/Contents/MacOS/vulkansdk-macOS* --root "$1" --accept-licenses --default-answer --confirm-command install
rm -rf ${VULKAN_TEMP}
