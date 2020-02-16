#! /bin/bash

set -x
set -e

if [ "$CI" == "" ] && [ -d /dev/shm ]; then
    BUILD_DIR=/dev/shm/vvflow-appimage
    mkdir -p $BUILD_DIR
else
    BUILD_DIR=$(mktemp -d vvflow-appimage-XXXXXX)
fi

REPO_ROOT=$(dirname $(readlink -f $(dirname $0)))
pushd "$BUILD_DIR"
cmake "$REPO_ROOT" -DCMAKE_INSTALL_PREFIX=/usr
make -j && make install DESTDIR=AppDir
popd

if [ ! -e ./linuxdeploy-x86_64.AppImage ]; then
	wget https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage
	chmod +x linuxdeploy*.AppImage
fi

export VERSION=$(git describe --tags)
export LD_LIBRARY_PATH="$BUILD_DIR/AppDir/usr/lib/:$LD_LIBRARY_PATH"

./linuxdeploy-x86_64.AppImage \
	--appdir "$BUILD_DIR/AppDir" \
	--icon-file "$REPO_ROOT/appimage/vvflow.svg" \
	--desktop-file "$REPO_ROOT/appimage/vvflow.desktop" \
	--custom-apprun "$REPO_ROOT/appimage/AppRun" \
	--output appimage \
