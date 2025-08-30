clean:
	rm -rf build

build: clean
	mkdir -p build && cd build && cmake -G Ninja -DCMAKE_BUILD_TYPE=Debug .. && cmake --build . -j

run: build
	./build/notepad
