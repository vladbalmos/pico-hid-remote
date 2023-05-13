.PHONY: build clean debug install-debug install

build:
	mkdir -p build
	cd build && cmake -DCYW43_LWIP=0 -DPICO_BOARD=pico_w .. && make -j4

debug:
	mkdir -p debug
	cd debug && DEBUG_MODE=1 cmake -DPICO_BOARD=pico_w .. && make -j4

bt-debug:
	mkdir -p bt-debug
	cd bt-debug && DEBUG_MODE=1 BT_DEBUG_MODE=1 cmake -DPICO_BOARD=pico_w .. && make -j4

clean:
	rm -rf build
	rm -rf debug
	rm -rf bt-debug

install: build
	cd build && pico-install.sh bt-experiments

install-debug: debug
	cd debug && pico-install.sh bt-experiments

install-bt-debug: bt-debug
	cd bt-debug && pico-install.sh bt-experiments
