HOST_CC ?= cc
HOST_CFLAGS ?= -std=c89 -Wall -Wextra -Werror -pedantic -Iinclude -Isrc
AMIGA_CC ?= m68k-amigaos-gcc
AMIGA_CFLAGS ?= -m68000 -Os -Wall -Wextra -Iinclude -Isrc

TARGET := AmiGuardAE
HOST_SOURCES := src/main.c src/core.c src/arexx_dispatch.c src/arexx_amiga.c src/scanner_bridge.c src/signature_bridge.c src/result_store.c src/checksum.c src/identify.c
AMIGA_SOURCES := $(HOST_SOURCES)
TEST_TARGET := build/test_dispatch

.PHONY: all host amiga check test clean

all: host

host: $(TARGET)

$(TARGET): $(HOST_SOURCES) include/amiguard_ae.h src/arexx_dispatch.h src/arexx_amiga.h src/scanner_bridge.h src/signature_bridge.h src/result_store.h src/checksum.h src/identify.h
	$(HOST_CC) $(HOST_CFLAGS) $(HOST_SOURCES) -o $@

amiga:
	$(AMIGA_CC) $(AMIGA_CFLAGS) $(AMIGA_SOURCES) -o $(TARGET).amiga

$(TEST_TARGET): tests/test_dispatch.c src/core.c src/arexx_dispatch.c src/scanner_bridge.c src/signature_bridge.c src/result_store.c src/checksum.c src/identify.c include/amiguard_ae.h src/arexx_dispatch.h src/scanner_bridge.h src/signature_bridge.h src/result_store.h src/checksum.h src/identify.h
	@mkdir -p build
	$(HOST_CC) $(HOST_CFLAGS) tests/test_dispatch.c src/core.c src/arexx_dispatch.c src/scanner_bridge.c src/signature_bridge.c src/result_store.c src/checksum.c src/identify.c -o $(TEST_TARGET)

test: $(TEST_TARGET)
	./$(TEST_TARGET)

check: test
	@test -f README.md
	@test -f ROADMAP.md
	@test -f LICENSE
	@test -f docs/AREXX_API.md
	@test -f include/amiguard_ae.h
	@test -f src/main.c
	@test -f src/core.c
	@test -f src/arexx_dispatch.c
	@test -f src/arexx_amiga.c
	@test -f src/scanner_bridge.c
	@test -f src/signature_bridge.c
	@test -f src/result_store.c
	@test -f src/checksum.c
	@test -f src/identify.c
	@test -f examples/ping.rexx
	@grep -q 'AMIGUARD_AE_AREXX_PORT "AMIGUARD"' include/amiguard_ae.h
	@grep -q 'm68k-amigaos-gcc' Makefile
	@grep -q -- '-m68000' Makefile
	@echo "M3.1 host qualification: PASS"

clean:
	$(RM) -r $(TARGET) $(TARGET).amiga build
