source $setup

cmake-cross $src \
  -DCMAKE_INSTALL_PREFIX=$out

make
make install

$host-strip $out/bin/*
cp version.txt $out/

if [ $os = "linux" ]; then
  cp $dejavu/ttf/DejaVuSans.ttf $out/bin/
fi
