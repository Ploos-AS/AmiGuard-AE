HOST_CC ?= cc
HOST_CFLAGS ?= -std=c89 -Wall -Wextra -Werror -pedantic -Iinclude -Isrc
CRYPTO_CFLAGS ?= -std=gnu89 -Wall -Wextra -Werror -Iinclude -Isrc
AMIGA_CC ?= m68k-amigaos-gcc
AMIGA_CFLAGS ?= -m68000 -Os -Wall -Wextra -Iinclude -Isrc
ED25519_DIR ?= deps/ed25519
ED25519_SRC := $(ED25519_DIR)/src/verify.c $(ED25519_DIR)/src/sha512.c $(ED25519_DIR)/src/ge.c $(ED25519_DIR)/src/fe.c $(ED25519_DIR)/src/sc.c
CI_KEY_ID := ci-test-1
CI_KEY_HEX := 43046bfe4092b3e94994eada15dcc20d8aaa07b658fd3954eb8e0efb8bdca5de
CI_CRYPTO_DEFS := -DAMIGUARD_AE_WITH_ED25519=1 -DAMIGUARD_AE_TRUSTED_KEY_ID=\"$(CI_KEY_ID)\" -DAMIGUARD_AE_TRUSTED_PUBLIC_KEY_HEX=\"$(CI_KEY_HEX)\"

TARGET := AmiGuardAE
CORE_SOURCES := src/core.c src/arexx_dispatch.c src/scanner_bridge.c src/signature_bridge.c src/signature_manifest.c src/signature_auth_ed25519.c src/result_store.c src/checksum.c src/identify.c src/quarantine_model.c
HOST_SOURCES := src/main.c src/arexx_amiga.c $(CORE_SOURCES)
AMIGA_SOURCES := $(HOST_SOURCES)
TEST_TARGET := build/test_dispatch
MANIFEST_TEST_TARGET := build/test_signature_manifest
ED25519_TEST_TARGET := build/test_ed25519_auth
QUARANTINE_TEST_TARGET := build/test_quarantine_model

.PHONY: all host amiga check test crypto-check deps-ed25519 clean

all: host

host: $(TARGET)

$(TARGET): $(HOST_SOURCES) include/amiguard_ae.h src/arexx_dispatch.h src/arexx_amiga.h src/scanner_bridge.h src/signature_bridge.h src/signature_manifest.h src/signature_auth_ed25519.h src/result_store.h src/checksum.h src/identify.h src/quarantine_model.h
	$(HOST_CC) $(HOST_CFLAGS) $(HOST_SOURCES) -o $@

amiga:
	$(AMIGA_CC) $(AMIGA_CFLAGS) $(AMIGA_SOURCES) -o $(TARGET).amiga

$(TEST_TARGET): tests/test_dispatch.c $(CORE_SOURCES) include/amiguard_ae.h src/arexx_dispatch.h src/scanner_bridge.h src/signature_bridge.h src/result_store.h src/checksum.h src/identify.h src/quarantine_model.h
	@mkdir -p build
	$(HOST_CC) $(HOST_CFLAGS) tests/test_dispatch.c $(CORE_SOURCES) -o $(TEST_TARGET)

$(MANIFEST_TEST_TARGET): tests/test_signature_manifest.c src/signature_manifest.c src/signature_manifest.h
	@mkdir -p build
	$(HOST_CC) $(HOST_CFLAGS) tests/test_signature_manifest.c src/signature_manifest.c -o $(MANIFEST_TEST_TARGET)

$(QUARANTINE_TEST_TARGET): tests/test_quarantine_model.c src/quarantine_model.c src/quarantine_model.h src/checksum.c src/checksum.h
	@mkdir -p build
	$(HOST_CC) $(HOST_CFLAGS) tests/test_quarantine_model.c src/quarantine_model.c src/checksum.c -o $(QUARANTINE_TEST_TARGET)

deps-ed25519:
	bash tools/fetch-ed25519.sh $(ED25519_DIR)

$(ED25519_TEST_TARGET): tests/test_ed25519_auth.c src/signature_auth_ed25519.c src/signature_manifest.c src/signature_auth_ed25519.h src/signature_manifest.h
	@test -f $(ED25519_DIR)/src/ed25519.h || { echo "run make deps-ed25519 first"; exit 1; }
	@mkdir -p build
	$(HOST_CC) $(CRYPTO_CFLAGS) -I$(ED25519_DIR)/src $(CI_CRYPTO_DEFS) tests/test_ed25519_auth.c src/signature_auth_ed25519.c src/signature_manifest.c $(ED25519_SRC) -o $(ED25519_TEST_TARGET)

crypto-check: $(ED25519_TEST_TARGET)
	./$(ED25519_TEST_TARGET)

test: $(TEST_TARGET) $(MANIFEST_TEST_TARGET) $(QUARANTINE_TEST_TARGET)
	./$(TEST_TARGET)
	./$(MANIFEST_TEST_TARGET)
	./$(QUARANTINE_TEST_TARGET)

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
	@test -f src/signature_manifest.c
	@test -f src/signature_manifest.h
	@test -f src/signature_auth_ed25519.c
	@test -f src/signature_auth_ed25519.h
	@test -f src/result_store.c
	@test -f src/checksum.c
	@test -f src/identify.c
	@test -f src/quarantine_model.c
	@test -f src/quarantine_model.h
	@test -f examples/ping.rexx
	@grep -q 'AMIGUARD_AE_AREXX_PORT "AMIGUARD"' include/amiguard_ae.h
	@grep -q 'm68k-amigaos-gcc' Makefile
	@grep -q -- '-m68000' Makefile
	@echo "M4.1 host qualification: PASS"

clean:
	$(RM) -r $(TARGET) $(TARGET).amiga build
