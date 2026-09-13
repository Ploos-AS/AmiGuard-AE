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
rx_host="$(find "$aros_root" -type f -iname 'rx' -print -quit || true)"
rexxmast_host="$(find "$aros_root" -type f -iname 'rexxmast' -print -quit || true)"
rexxlib_host="$(find "$aros_root" -type f -iname 'rexxsyslib.library' -print -quit || true)"
{
  echo "RX=${rx_host:-MISSING}"
  echo "REXXMAST=${rexxmast_host:-MISSING}"
  echo "REXXSYSLIB=${rexxlib_host:-MISSING}"
  echo "NOTE=AROS gate qualifies native 68k dispatcher/core only; production ARexx transport remains a local classic-AmigaOS gate"
} > "$OUT_DIR/arexx-capabilities.txt"

cp "$NATIVE" "$aros_root/AmiGuardAE"
cp "$startup" "$startup.amiguard-ae-original"

cat > "$startup" <<'EOF'
FailAt 21
SYS:C/Echo "M1_GUEST_STARTED=1" >SYS:amiguard-ae-m1-started.txt
SYS:C/Which AmiGuardAE >SYS:amiguard-ae-m1-which.txt
SYS:C/Echo "M1_BEFORE_AMIGUARD_AE=1" >SYS:amiguard-ae-m1-before.txt
SYS:AmiGuardAE >SYS:amiguard-ae-m1-output.txt
SYS:C/Echo $RC >SYS:amiguard-ae-m1-rc.txt
SYS:C/Echo "M1_AFTER_AMIGUARD_AE=1" >SYS:amiguard-ae-m1-after.txt
EOF

rm -f "$aros_root"/amiguard-ae-m1-{started,which,before,output,rc,after}.txt

config="$OUT_DIR/aros-guest.fs-uae"
sed "s|@AROS_ROOT@|$PWD/$aros_root|" ci/fs-uae/aros-guest.fs-uae > "$config"
fs-uae --version > "$OUT_DIR/fs-uae-version.txt" 2>&1 || true

set +e
timeout 45s xvfb-run -a fs-uae "$config" > "$OUT_DIR/fs-uae.log" 2>&1
rc=$?
set -e

started="$aros_root/amiguard-ae-m1-started.txt"
output="$aros_root/amiguard-ae-m1-output.txt"
guest_rc="$aros_root/amiguard-ae-m1-rc.txt"
after="$aros_root/amiguard-ae-m1-after.txt"
status=FAIL
observation=guest_result_missing

if [[ -f "$started" && -f "$after" && -f "$output" ]] \
   && grep -q 'M1 AROS native dispatcher smoke: PASS' "$output" \
   && grep -q 'SMOKE PASS command=PING rc=0 result=PONG' "$output" \
   && grep -q 'SMOKE PASS command=BOGUS rc=10 result=ERROR unknown command' "$output"; then
  status=PASS
  observation=native_68k_dispatcher_core_qualified_in_aros
elif [[ -f "$after" ]]; then
  observation=guest_smoke_returned_without_expected_dispatcher_evidence
elif [[ -f "$aros_root/amiguard-ae-m1-before.txt" ]]; then
  observation=guest_entered_smoke_binary_but_did_not_return
fi

{
  echo "STATUS=$status"
  echo "GATE=M1_AROS_NATIVE_DISPATCHER_SMOKE"
  echo "MODEL=A1200"
  echo "KICKSTART=internal"
  echo "AROS_ROOT=$aros_root"
  echo "FS_UAE_EXIT=$rc"
  echo "OBSERVATION=$observation"
  echo "AROS_REXXMAST=${rexxmast_host:-MISSING}"
  echo "AROS_REXXSYSLIB=${rexxlib_host:-MISSING}"
  echo "AREXX_RUNTIME_QUALIFICATION=LOCAL_CLASSIC_AMIGAOS"
  if [[ -f "$aros_root/amiguard-ae-m1-which.txt" ]]; then
    tr -d '\r' < "$aros_root/amiguard-ae-m1-which.txt" | sed 's/^/GUEST_WHICH=/'
  fi
  if [[ -f "$guest_rc" ]]; then
    tr -d '\r' < "$guest_rc" | sed 's/^/GUEST_RC=/'
  fi
  if [[ -f "$output" ]]; then
    tr -d '\r' < "$output" | sed 's/^/GUEST_OUTPUT=/'
  fi
} | tee "$OUT_DIR/result.txt"

[[ "$status" == PASS ]]
