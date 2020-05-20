em++ -std=c++11  --memory-init-file 0 ^
 BitsPut.cpp PrintLog.cpp RingBuffer.cpp Rijndael.cpp FlvParse.cpp Mp4Parse.cpp DashConvert.cpp Vs4DashConvert.cpp main.cpp ^
 -s ALLOW_MEMORY_GROWTH=1 -s TOTAL_MEMORY=32*1024*1024 -s USE_ZLIB=1 -s DISABLE_EXCEPTION_CATCHING=0 ^
 -s TOTAL_STACK=5*1024*1024 -s NO_EXIT_RUNTIME=1 ^
 --post-js  viewboxplayerc.js ^
 -O3 -o main.js ^
 -s "EXTRA_EXPORTED_RUNTIME_METHODS=['ccall', 'cwrap']" -s LEGACY_VM_SUPPORT=1 ^
 -s EXPORTED_FUNCTIONS="['_main', '__Z2aaPci', '__Z2abPci', '__Z2acPci', '__Z2a_v', '__Z2adi', '__Z2aePc', '__Z2afv', '__Z2agPfi', '__Z2ahi', '__Z2aid', '__Z2aji', '__Z3a__v', '__Z2bbv', '__Z2aKv', '__Z2akv']"