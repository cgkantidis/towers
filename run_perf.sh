rm -rf build
cmake -S . -B build -DCMAKE_BUILD_TYPE=PERF
cmake --build build

perf record -F 1000 -g build/towers
perf script report flamegraph --allow-download
xdg-open flamegraph.html

