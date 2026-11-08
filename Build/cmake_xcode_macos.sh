#!/bin/sh

cmake -S . -B XCode_MacOS -GXcode -D"CMAKE_OSX_ARCHITECTURES=x86_64;arm64" -DCMAKE_OSX_DEPLOYMENT_TARGET=26.0
open XCode_MacOS/JoltPhysics.xcodeproj
