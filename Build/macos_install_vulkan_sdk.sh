set -e
if [ -z $1 ] 
then
	echo "Need to specify installation diretory"
	exit
fi

VULKAN_TEMP=/tmp/vulkan_sdk_install
mkdir ${VULKAN_TEMP}
curl -L -o ${VULKAN_TEMP}/vulkan_sdk.dmg https://sdk.lunarg.com/sdk/download/latest/mac/vulkan_sdk.dmg?Human=true
unzip ${VULKAN_TEMP}/vulkan_sdk.dmg -d ${VULKAN_TEMP}
${VULKAN_TEMP}/vulkansdk-macOS*.app/Contents/MacOS/vulkansdk-macOS* --root $1 --accept-licenses --default-answer --confirm-command install
rm -rf ${VULKAN_TEMP}
