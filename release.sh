# Release an AppImage
mkdir build
cd build

cmake .. -DCMAKE_INSTALL_PREFIX=/usr
make -j$(nproc)
make install DESTDIR=AppDir

cd ..
# environment-specific options
export QMAKE=/home/jefffu/Qt5.12.11/5.12.11/gcc_64/bin/qmake
export LD_LIBRARY_PATH=/home/jefffu/Qt5.12.11/5.12.11/gcc_64/lib
./linuxdeploy-x86_64.AppImage --appdir build/AppDir --plugin qt --output appimage
rm -r build
unset QMAKE
unset LD_LIBRARY_PATH