rm -rf build
cmake -S . -B build -DCMAKE_BUILD_TYPE=PPROF
cmake --build build

CPUPROFILE=towers.pprof build/towers
pprof --web build/towers towers.pprof

