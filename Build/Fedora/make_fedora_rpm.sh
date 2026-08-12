#!/bin/sh
# SPDX-FileCopyrightText: © 2021-2026 Jorrit Rouwe
# SPDX-FileCopyrightText: © 2026 Igal Alkon
# SPDX-License-Identifier: MIT
set -e

# Get version slug
package_name=joltphysics
version_hash=$(git describe --tags --match "v*" | sed 's/^v//' | sed 's/-/./g')
echo "** repository hash: ${version_hash} ..."

# Create dir tree for rpmbuild in user dir
rpmdev-setuptree

# Archive repository
(cd ../.. && git archive --format=tar.gz --prefix=${package_name}-${version_hash}/ -o ~/rpmbuild/SOURCES/${package_name}-${version_hash}.tar.gz HEAD)
echo "** created archive: ~/rpmbuild/SOURCES/${package_name}-${version_hash}.tar.gz"
sleep 2

# Replace spec version
sed -i "s/Version:.\+/Version: ${version_hash}/g" ${package_name}.spec
echo "** building package version: ${version_hash}"

# Check dependencies
sudo dnf builddep -y ${package_name}.spec

# Build package, let it automatically download extra sources
rpmbuild --define "debug_package %{nil}" --clean -bb ${package_name}.spec

echo "** packages for ${package_name}-${version_hash} complete:"
ls ~/rpmbuild/RPMS/$(uname -m)/${package_name}-*${version_hash}*.rpm | cat
