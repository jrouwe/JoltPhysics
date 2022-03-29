// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Jolt/Core/Reference.h>

class Surface;

/// Image routines, loads a Targa (TGA) file.
Ref<Surface> LoadTGA(istream &inStream);
