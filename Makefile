CC ?= cc
CFLAGS ?= -std=c89 -Wall -Wextra -Werror -pedantic -Iinclude

TARGET := AmiGuardAE
SOURCES := src/main.c src/core.c

.PHONY: all clean check

all: $(TARGET)

$(TARGET): $(SOURCES) include/amiguard_ae.h
	$(CC) $(CFLAGS) $(SOURCES) -o $@

check:
	@test -f README.md
	@test -f ROADMAP.md
	@test -f LICENSE
	@test -f docs/AREXX_API.md
	@test -f include/amiguard_ae.h
	@test -f src/main.c
	@test -f src/core.c
	@test -f examples/ping.rexx
	@grep -q 'AMIGUARD_AE_AREXX_PORT "AMIGUARD"' include/amiguard_ae.h
	@grep -q 'AmigaOS 2.04+' ROADMAP.md
	@grep -q '68000' ROADMAP.md
	@echo "M0 structural qualification: PASS"

clean:
	rm -f $(TARGET)
