source $setup

cmake-cross $src \
  -DCMAKE_INSTALL_PREFIX=$out

make

make install

$host-strip $out/bin/*
