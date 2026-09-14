#!/usr/bin/env bash
set -euo pipefail
OUT_DIR="${1:-build/fs-uae/aros-guest}"
SYSTEM_DIR="build/fs-uae/aros-system"
NATIVE="build/fs-uae/native/AmiGuardAE-aros-smoke"
mkdir -p "$OUT_DIR"
[[ -f "$NATIVE" ]] || { echo "ERROR: AROS smoke binary missing" >&2; exit 1; }
iso="$(bash ci/fs-uae/fetch-aros-system.sh "$SYSTEM_DIR" | tail -n 1)"
root_extract="$OUT_DIR/system-root"; rm -rf "$root_extract"; mkdir -p "$root_extract"; 7z x -y -o"$root_extract" "$iso" >/dev/null
startup="$(find "$root_extract" -type f -ipath '*/s/startup-sequence' -print -quit)"; [[ -n "$startup" ]] || exit 1
aros_root="$(dirname "$(dirname "$startup")")"
rexxmast_host="$(find "$aros_root" -type f -iname 'rexxmast' -print -quit || true)"; rexxlib_host="$(find "$aros_root" -type f -iname 'rexxsyslib.library' -print -quit || true)"
printf 'REXXMAST=%s\nREXXSYSLIB=%s\nNOTE=AROS qualifies native 68k runtime signature update core; production ARexx transport remains deferred local classic AmigaOS\n' "${rexxmast_host:-MISSING}" "${rexxlib_host:-MISSING}" > "$OUT_DIR/arexx-capabilities.txt"
cp "$NATIVE" "$aros_root/AmiGuardAE"; cp "$startup" "$startup.amiguard-ae-original"
cat > "$startup" <<'EOF'
FailAt 21
SYS:C/Echo "M3_5_GUEST_STARTED=1" >SYS:amiguard-ae-m3-5-started.txt
SYS:AmiGuardAE >SYS:amiguard-ae-m3-5-output.txt
SYS:C/Echo $RC >SYS:amiguard-ae-m3-5-rc.txt
SYS:C/Echo "M3_5_AFTER_AMIGUARD_AE=1" >SYS:amiguard-ae-m3-5-after.txt
EOF
rm -f "$aros_root"/amiguard-ae-m3-5-{started,output,rc,after}.txt
config="$OUT_DIR/aros-guest.fs-uae"; sed "s|@AROS_ROOT@|$PWD/$aros_root|" ci/fs-uae/aros-guest.fs-uae > "$config"; fs-uae --version > "$OUT_DIR/fs-uae-version.txt" 2>&1 || true
set +e; timeout 45s xvfb-run -a fs-uae "$config" > "$OUT_DIR/fs-uae.log" 2>&1; rc=$?; set -e
started="$aros_root/amiguard-ae-m3-5-started.txt"; output="$aros_root/amiguard-ae-m3-5-output.txt"; guest_rc="$aros_root/amiguard-ae-m3-5-rc.txt"; after="$aros_root/amiguard-ae-m3-5-after.txt"
status=FAIL; observation=guest_result_missing
if [[ -f "$started" && -f "$after" && -f "$output" && -f "$guest_rc" ]] && grep -q '^0' "$guest_rc" && grep -q 'M3.5 AROS runtime signature database update smoke: PASS' "$output" && grep -q 'SMOKE PASS STATUS => READY M3.5 scanner=connected' "$output" && grep -q 'SMOKE PASS SIGNATURE.STATUS => READY COUNT=4 UPDATE=AVAILABLE' "$output" && grep -q 'SMOKE PASS SIGNATURE.UPDATE RAM:amiguard-ae-runtime.sigdb => UPDATED runtime signature database loaded' "$output" && grep -q 'SMOKE PASS SIGNATURE.COUNT => 3' "$output"; then status=PASS; observation=native_68k_runtime_signature_update_qualified_in_aros; elif [[ -f "$after" ]]; then observation=guest_smoke_returned_without_expected_m3_5_evidence; fi
{
echo "STATUS=$status"; echo "GATE=M3_5_AROS_RUNTIME_SIGNATURE_UPDATE"; echo "MODEL=A1200"; echo "KICKSTART=internal"; echo "FS_UAE_EXIT=$rc"; echo "OBSERVATION=$observation"; echo "AROS_REXXMAST=${rexxmast_host:-MISSING}"; echo "AROS_REXXSYSLIB=${rexxlib_host:-MISSING}"; echo "AREXX_RUNTIME_QUALIFICATION=DEFERRED_LOCAL_CLASSIC_AMIGAOS"; [[ -f "$guest_rc" ]] && tr -d '\r' < "$guest_rc" | sed 's/^/GUEST_RC=/' || true; [[ -f "$output" ]] && tr -d '\r' < "$output" | sed 's/^/GUEST_OUTPUT=/' || true;
} | tee "$OUT_DIR/result.txt"
[[ "$status" == PASS ]]
