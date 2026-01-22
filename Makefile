.PHONY: build run

build:
	mkdir -p build
	cd build && cmake .. && make

run:
	@./build/main