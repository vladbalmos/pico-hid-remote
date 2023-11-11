# pico-instal.sh is actual a wrapper script for the following command:
# openocd -f interface/cmsis-dap.cfg -c "adapter speed 5000" -f target/rp2040.cfg -s tcl -c "program hidremote.elf verify reset exit"

.PHONY: build clean debug install-debug install

build:
	mkdir -p build
	cd build && cmake -DCYW43_LWIP=0 -DPICO_BOARD=pico_w .. && make -j4

# Show debug messages
debug:
	mkdir -p debug
	cd debug && DEBUG_MODE=1 cmake -DPICO_BOARD=pico_w .. && make -j4

# Show btstack debug messages
bt-debug:
	mkdir -p bt-debug
	cd bt-debug && DEBUG_MODE=1 BT_DEBUG_MODE=1 cmake -DPICO_BOARD=pico_w .. && make -j4

clean:
	rm -rf build
	rm -rf debug
	rm -rf bt-debug

install: build
	cd build && pico-install.sh hidremote

install-debug: debug
	cd debug && pico-install.sh hidremote

install-bt-debug: bt-debug
	cd bt-debug && pico-install.sh hidremote
