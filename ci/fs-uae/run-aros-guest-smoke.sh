#!/usr/bin/env bash
set -euo pipefail

OUT_DIR="${1:-build/fs-uae/aros-guest}"
SYSTEM_DIR="build/fs-uae/aros-system"
NATIVE="build/fs-uae/native/AmiGuardAE-aros-smoke"
mkdir -p "$OUT_DIR"

if [[ ! -f "$NATIVE" ]]; then
  echo "ERROR: AROS smoke binary missing; run native build gate first" >&2
  exit 1
fi

iso="$(bash ci/fs-uae/fetch-aros-system.sh "$SYSTEM_DIR" | tail -n 1)"
root_extract="$OUT_DIR/system-root"
rm -rf "$root_extract"
mkdir -p "$root_extract"
7z x -y -o"$root_extract" "$iso" >/dev/null

startup="$(find "$root_extract" -type f -ipath '*/s/startup-sequence' -print -quit)"
if [[ -z "$startup" ]]; then
  echo "ERROR: AROS system ISO does not contain S/Startup-Sequence" >&2
  find "$root_extract" -maxdepth 3 -type f | sort > "$OUT_DIR/system-files.txt"
  exit 1
fi

aros_root="$(dirname "$(dirname "$startup")")"
rexxmast_host="$(find "$aros_root" -type f -iname 'rexxmast' -print -quit || true)"
rexxlib_host="$(find "$aros_root" -type f -iname 'rexxsyslib.library' -print -quit || true)"
{
  echo "REXXMAST=${rexxmast_host:-MISSING}"
  echo "REXXSYSLIB=${rexxlib_host:-MISSING}"
  echo "NOTE=AROS gate qualifies native 68k scanner/result/checksum/identify/scan core only; production ARexx transport remains a deferred local classic-AmigaOS gate"
} > "$OUT_DIR/arexx-capabilities.txt"

cp "$NATIVE" "$aros_root/AmiGuardAE"
cp "$startup" "$startup.amiguard-ae-original"

cat > "$startup" <<'EOF'
FailAt 21
SYS:C/Echo "M2_6_GUEST_STARTED=1" >SYS:amiguard-ae-m2-6-started.txt
SYS:C/Which AmiGuardAE >SYS:amiguard-ae-m2-6-which.txt
SYS:C/Echo "M2_6_BEFORE_AMIGUARD_AE=1" >SYS:amiguard-ae-m2-6-before.txt
SYS:AmiGuardAE >SYS:amiguard-ae-m2-6-output.txt
SYS:C/Echo $RC >SYS:amiguard-ae-m2-6-rc.txt
SYS:C/Echo "M2_6_AFTER_AMIGUARD_AE=1" >SYS:amiguard-ae-m2-6-after.txt
EOF

rm -f "$aros_root"/amiguard-ae-m2-6-{started,which,before,output,rc,after}.txt

config="$OUT_DIR/aros-guest.fs-uae"
sed "s|@AROS_ROOT@|$PWD/$aros_root|" ci/fs-uae/aros-guest.fs-uae > "$config"
fs-uae --version > "$OUT_DIR/fs-uae-version.txt" 2>&1 || true

set +e
timeout 45s xvfb-run -a fs-uae "$config" > "$OUT_DIR/fs-uae.log" 2>&1
rc=$?
set -e

started="$aros_root/amiguard-ae-m2-6-started.txt"
which_file="$aros_root/amiguard-ae-m2-6-which.txt"
output="$aros_root/amiguard-ae-m2-6-output.txt"
guest_rc="$aros_root/amiguard-ae-m2-6-rc.txt"
after="$aros_root/amiguard-ae-m2-6-after.txt"
before="$aros_root/amiguard-ae-m2-6-before.txt"
status=FAIL
observation=guest_result_missing

if [[ -f "$started" && -f "$after" && -f "$output" && -f "$guest_rc" ]] \
   && grep -q '^0' "$guest_rc" \
   && grep -q 'M2.6 AROS scan smoke: PASS' "$output" \
   && grep -q 'SMOKE PASS PING => PONG' "$output" \
   && grep -q 'SMOKE PASS STATUS => READY M2.6 scanner=connected' "$output" \
   && grep -q 'SMOKE PASS SCAN => ERROR SCAN requires path' "$output" \
   && grep -q 'SMOKE PASS SCAN RAM:amiguard-ae-crc.bin => CLEAN ' "$output" \
   && grep -q 'SMOKE PASS RESULT.STATUS => CLEAN' "$output" \
   && grep -q 'SMOKE PASS RESULT.PATH => RAM:amiguard-ae-crc.bin' "$output" \
   && grep -q 'SMOKE PASS CHECKSUM RAM:amiguard-ae-crc.bin => CRC32 CBF43926' "$output" \
   && grep -q 'SMOKE PASS IDENTIFY RAM:amiguard-ae-hunk.bin => AMIGA-HUNK HUNK_HEADER' "$output" \
   && grep -q 'SMOKE PASS RESULT.CLEAR => OK' "$output" \
   && grep -q 'SMOKE PASS BOGUS => ERROR unknown command' "$output"; then
  status=PASS
  observation=native_68k_scan_api_qualified_in_aros
elif [[ -f "$after" ]]; then
  observation=guest_smoke_returned_without_expected_m2_6_evidence
elif [[ -f "$before" ]]; then
  observation=guest_entered_smoke_binary_but_did_not_return
fi

{
  echo "STATUS=$status"
  echo "GATE=M2_6_AROS_SCAN_SMOKE"
  echo "MODEL=A1200"
  echo "KICKSTART=internal"
  echo "AROS_ROOT=$aros_root"
  echo "FS_UAE_EXIT=$rc"
  echo "OBSERVATION=$observation"
  echo "AROS_REXXMAST=${rexxmast_host:-MISSING}"
  echo "AROS_REXXSYSLIB=${rexxlib_host:-MISSING}"
  echo "AREXX_RUNTIME_QUALIFICATION=DEFERRED_LOCAL_CLASSIC_AMIGAOS"
  if [[ -f "$which_file" ]]; then tr -d '\r' < "$which_file" | sed 's/^/GUEST_WHICH=/'; fi
  if [[ -f "$guest_rc" ]]; then tr -d '\r' < "$guest_rc" | sed 's/^/GUEST_RC=/'; fi
  if [[ -f "$output" ]]; then tr -d '\r' < "$output" | sed 's/^/GUEST_OUTPUT=/'; fi
} | tee "$OUT_DIR/result.txt"

[[ "$status" == PASS ]]
