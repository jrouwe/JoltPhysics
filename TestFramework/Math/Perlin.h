#pragma once

// Taken from: https://github.com/nothings/stb/blob/master/stb_perlin.h
//
// stb_perlin.h - v0.3 - perlin noise
// public domain single-file C implementation by Sean Barrett
//
// LICENSE
//
// See cpp file.
//
// Contributors:
//    Jack Mott - additional noise functions
//
// float  PerlinNoise3(float x,
//                     float y,
//                     float z,
//                     int   x_wrap = 0,
//                     int   y_wrap = 0,
//                     int   z_wrap = 0)
//
// This function computes a random value at the coordinate (x,y,z).
// Adjacent random values are continuous but the noise fluctuates
// its randomness with period 1, i.e. takes on wholly unrelated values
// at integer points. Specifically, this implements Ken Perlin's
// revised noise function from 2002.
//
// The "wrap" parameters can be used to create wraparound noise that
// wraps at powers of two. The numbers MUST be powers of two. Specify
// 0 to mean "don't care". (The noise always wraps every 256 due
// details of the implementation, even if you ask for larger or no
// wrapping.)
//
// Fractal Noise:
//
// Three common fractal noise functions are included, which produce
// a wide variety of nice effects depending on the parameters
// provided. Note that each function will call PerlinNoise3
// 'octaves' times, so this parameter will affect runtime.
//
// float PerlinRidgeNoise3(float x, float y, float z,
//                         float lacunarity, float gain, float offset, int octaves,
//                         int x_wrap, int y_wrap, int z_wrap);
//
// float PerlinFBMNoise3(float x, float y, float z,
//                       float lacunarity, float gain, int octaves,
//                       int x_wrap, int y_wrap, int z_wrap);
//
// float PerlinTurbulenceNoise3(float x, float y, float z,
//                              float lacunarity, float gain,int octaves,
//                              int x_wrap, int y_wrap, int z_wrap);
//
// Typical values to start playing with:
//     octaves    =   6     -- number of "octaves" of noise3() to sum
//     lacunarity = ~ 2.0   -- spacing between successive octaves (use exactly 2.0 for wrapping output)
//     gain       =   0.5   -- relative weighting applied to each successive octave
//     offset     =   1.0?  -- used to invert the ridges, may need to be larger, not sure
//
float PerlinNoise3(float x, float y, float z, int x_wrap, int y_wrap, int z_wrap);
float PerlinRidgeNoise3(float x, float y, float z, float lacunarity, float gain, float offset, int octaves, int x_wrap, int y_wrap, int z_wrap);
float PerlinFBMNoise3(float x, float y, float z, float lacunarity, float gain, int octaves,int x_wrap, int y_wrap, int z_wrap);
float PerlinTurbulenceNoise3(float x, float y, float z, float lacunarity, float gain, int octaves, int x_wrap, int y_wrap, int z_wrap);
