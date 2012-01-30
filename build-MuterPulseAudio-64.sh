cd MuterPulseAudio
autoreconf -i -f
./configure --enable-debug=no --enable-m64=yes
make clean
make
cp src/.libs/libMuterPulseAudio.so ../extension/modules/ctypes-binary/libMuterPulseAudio-64.so