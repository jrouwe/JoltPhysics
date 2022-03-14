#!/bin/sh

cmake -S . -B XCode_MacOS -GXcode
open XCode_MacOS/JoltPhysics.xcodeproj
