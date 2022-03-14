#!/bin/sh

cmake -S . -B XCode_iOS -DTARGET_HELLO_WORLD=OFF -DTARGET_PERFORMANCE_TEST=OFF -DCMAKE_SYSTEM_NAME=iOS -GXcode
open XCode_iOS/JoltPhysics.xcodeproj
