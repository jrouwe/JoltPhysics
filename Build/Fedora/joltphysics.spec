# SPDX-FileCopyrightText: © 2021-2026 Jorrit Rouwe
# SPDX-FileCopyrightText: © 2026 Igal Alkon
# SPDX-License-Identifier: MIT

Name:           joltphysics
Version:        5.6.0
Release:        1%{?dist}
Summary:        Multi-core friendly rigid body physics and collision detection library
License:        MIT
URL:            https://github.com/jrouwe/JoltPhysics
Source:         %{name}-%{version}.tar.gz

Requires:       vulkan-loader

BuildRequires:  cmake >= 3.20
BuildRequires:  gcc-c++
BuildRequires:  make
BuildRequires:  ninja-build
BuildRequires:  vulkan-headers
BuildRequires:  vulkan-loader-devel

%description
Jolt Physics is a multi-core friendly rigid body physics and collision
detection library written in C++. Suitable for games and VR applications.

This package contains the shared Jolt Physics library variant.

%package static
Summary:        Static library for JoltPhysics engine (Release)
Requires:       %{name}-devel = %{version}-%{release}

%description static
This package contains the static Jolt Physics library variant.

%package staticdebug
Summary:        Static debug library for JoltPhysics engine (Release)
Requires:       %{name}-devel = %{version}-%{release}

%description staticdebug
This package contains the static Jolt Physics library variant.

%package devel
Summary:        Development files for JoltPhysics engine
Requires:       %{name}%{?_isa} = %{version}-%{release}
Requires:       cmake
Requires:       vulkan-headers

%description devel
Header files, CMake configuration and debug libraries.

# Path to the Vulkan SDK (adjust if your installation differs)
# Only the tools we need from the LunarG SDK. Do NOT set VULKAN_SDK.

%global vulkan_sdk_version 1.4.350.0
%global vulkan_sdk_root    %{getenv:HOME}/VulkanSDK/%{vulkan_sdk_version}
%global vulkan_sdk_bin %{vulkan_sdk_root}/x86_64/bin

%prep
%autosetup -n joltphysics-%{version}

%build
if [ ! -x "%{vulkan_sdk_bin}/dxc" ]; then
  echo "ERROR: dxc not found at %{vulkan_sdk_bin}/dxc"
  echo "Install the LunarG Vulkan SDK or update the path."
  exit 1
else
  echo "Using dxc from: %{vulkan_sdk_bin}/dxc"
fi

export PATH="%{vulkan_sdk_bin}${PATH:+:$PATH}"

cd Build

# Common options used by all builds
jolt_common_opts=(
  -DCMAKE_CXX_COMPILER=g++
  -DCMAKE_INSTALL_PREFIX=%{_prefix}
  -DCMAKE_INSTALL_LIBDIR=%{_libdir}
  -DENABLE_INSTALL=ON
  -DTARGET_UNIT_TESTS=OFF
  -DTARGET_HELLO_WORLD=OFF
  -DTARGET_PERFORMANCE_TEST=OFF
  -DTARGET_SAMPLES=OFF
  -DTARGET_VIEWER=OFF
  -DJPH_USE_VK=ON
  -DJPH_USE_DX12=OFF
  -DJPH_USE_MTL=OFF
  -DJPH_USE_CPU_COMPUTE=ON
  -DINTERPROCEDURAL_OPTIMIZATION=ON
  -DGENERATE_DEBUG_SYMBOLS=ON
  -DPROFILER_IN_DEBUG_AND_RELEASE=OFF
  -DDEBUG_RENDERER_IN_DEBUG_AND_RELEASE=ON
  -DCPP_RTTI_ENABLED=ON
  -DCPP_EXCEPTIONS_ENABLED=ON
)

# ----------------------------------------------------------------------
# 1) Release + Static: libJolt.a
# ----------------------------------------------------------------------
cmake -S . -B Linux_Release \
  "${jolt_common_opts[@]}" \
  -DCMAKE_BUILD_TYPE=Release \
  -DTARGET_UNIT_TESTS=ON \
  -DJPH_BUILD_SHARED_LIBS=OFF \
  -DBUILD_SHARED_LIBS=OFF

cmake --build Linux_Release --parallel %{_smp_build_ncpus}

# ----------------------------------------------------------------------
# 2) Release + Shared: libJolt.so
# ----------------------------------------------------------------------
cmake -S . -B Linux_Release_shared \
  "${jolt_common_opts[@]}" \
  -DCMAKE_BUILD_TYPE=Release \
  -DTARGET_UNIT_TESTS=ON \
  -DJPH_BUILD_SHARED_LIBS=ON \
  -DBUILD_SHARED_LIBS=ON

cmake --build Linux_Release_shared --parallel %{_smp_build_ncpus}

# ----------------------------------------------------------------------
# 3) Debug + Static: libJolt-d.a
# ----------------------------------------------------------------------
cmake -S . -B Linux_Debug \
  "${jolt_common_opts[@]}" \
  -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_DEBUG_POSTFIX=-d \
  -DENABLE_ALL_WARNINGS=OFF \
  -DJPH_BUILD_SHARED_LIBS=OFF \
  -DBUILD_SHARED_LIBS=OFF

cmake --build Linux_Debug --parallel %{_smp_build_ncpus}

# ----------------------------------------------------------------------
# 4) Debug + Shared: libJolt-d.so
# ----------------------------------------------------------------------
cmake -S . -B Linux_Debug_shared \
  "${jolt_common_opts[@]}" \
  -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_DEBUG_POSTFIX=-d \
  -DENABLE_ALL_WARNINGS=OFF \
  -DJPH_BUILD_SHARED_LIBS=ON \
  -DBUILD_SHARED_LIBS=ON

cmake --build Linux_Debug_shared --parallel %{_smp_build_ncpus}

%check
. "%{vulkan_sdk_root}/setup-env.sh"

cd Build

# Run the unit tests from the Release shared build
if [ -x Linux_Release_shared/UnitTests ]; then
  ./Linux_Release_shared/UnitTests
elif [ -x Linux_Release/UnitTests ]; then
  ./Linux_Release/UnitTests
else
  echo "ERROR: UnitTests binary not found"
  exit 1
fi

%install
. "%{vulkan_sdk_root}/setup-env.sh"
cd Build

# Install Debug builds first (adds the -d libraries)
DESTDIR=%{buildroot} cmake --install Linux_Debug
DESTDIR=%{buildroot} cmake --install Linux_Debug_shared

# Save the debug config file so it survives the next install
cp -a %{buildroot}%{_libdir}/cmake/Jolt/JoltConfig-debug.cmake \
      %{buildroot}%{_libdir}/cmake/Jolt/JoltConfig-debug.cmake.saved

# Install Release builds last
DESTDIR=%{buildroot} cmake --install Linux_Release
DESTDIR=%{buildroot} cmake --install Linux_Release_shared

# Restore the debug config
mv %{buildroot}%{_libdir}/cmake/Jolt/JoltConfig-debug.cmake.saved \
   %{buildroot}%{_libdir}/cmake/Jolt/JoltConfig-debug.cmake

# Keep only the real header files
find %{buildroot}%{_includedir}/Jolt -type f \
     ! \( -name '*.h' -o -name '*.hpp' -o -name '*.inl' \) -delete
find %{buildroot}%{_includedir}/Jolt -type d -empty -delete

%post -p /sbin/ldconfig
%postun -p /sbin/ldconfig

%files
%license LICENSE
# Only the Release shared library
%{_libdir}/libJolt.so*
%{_datadir}/Jolt/

%files static
# Only the Release static library
%{_libdir}/libJolt.a

%files staticdebug
# Only the Debug static library
%{_libdir}/libJolt-d.a

%files devel
%{_includedir}/Jolt/
%{_libdir}/cmake/Jolt/
# Debug libraries (the ones with -d postfix)
%{_libdir}/libJolt-d.so*
# Include the docs in the devel package
%doc README.md Docs/

%changelog
