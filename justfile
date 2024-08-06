
default: build

configure:
    mkdir -p build/ ;\
    cd build/ ;\
    cmake -GNinja -D CMAKE_BUILD_TYPE=Release -D USE_MKL=ON -D BUILD_DOCUMENTATION=On -D CMAKE_INSTALL_PREFIX=${CONDA_PREFIX} ../

build: configure
    cd build && ninja

install: build
    cd build && ninja install

test: unit-test integration-test

unit-test: build
    cd build && ctest

integration-test: build get-data
    cd test/integration; \
    python run_tests.py --echo-log-on-failure --timeout=600 -F --stats stats.csv cases/*

get-data:
    cd test/integration && python get_data.py