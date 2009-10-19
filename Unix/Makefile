ifeq ($(origin SOLUTION_DIR), undefined)
	SOLUTION_DIR := $(shell pwd)/
endif
PROJECT_DIR_VRAMREACTOR := $(dir $(lastword $(MAKEFILE_LIST)))
clean::
	rm -f libVRAM_Reactor.so*
	rm -f libVRAM_Reactor

libVRAM_Reactor :
	cd $(PROJECT_DIR_VRAMREACTOR)/../source && make
	cp $(PROJECT_DIR_VRAMREACTOR)/../source/libVRAM_Reactor.so* .
	cd $(PROJECT_DIR_VRAMREACTOR)/../source && make -i clean
	touch libVRAM_Reactor

