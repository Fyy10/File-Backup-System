# Release an AppImage
mkdir build
cd build

cmake .. -DCMAKE_INSTALL_PREFIX=/usr
make -j$(nproc)
make install DESTDIR=AppDir

cd ..
./linuxdeploy-x86_64.AppImage --appdir build/AppDir --output appimage
rm -r build