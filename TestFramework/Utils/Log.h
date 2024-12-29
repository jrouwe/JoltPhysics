// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

/// Pop up an alert
extern void Alert(const char *inFMT, ...);

/// Print an error message and terminate the application
extern void FatalError [[noreturn]] (const char *inFMT, ...);

/// Implementation of trace that traces to the TTY
extern void TraceImpl(const char *inFMT, ...);
