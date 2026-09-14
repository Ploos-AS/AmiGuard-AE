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
printf 'REXXMAST=%s\nREXXSYSLIB=%s\nNOTE=AROS qualifies native 68k signature status core; production ARexx transport remains deferred local classic AmigaOS\n' "${rexxmast_host:-MISSING}" "${rexxlib_host:-MISSING}" > "$OUT_DIR/arexx-capabilities.txt"
cp "$NATIVE" "$aros_root/AmiGuardAE"; cp "$startup" "$startup.amiguard-ae-original"
cat > "$startup" <<'EOF'
FailAt 21
SYS:C/Echo "M3_4_GUEST_STARTED=1" >SYS:amiguard-ae-m3-4-started.txt
SYS:AmiGuardAE >SYS:amiguard-ae-m3-4-output.txt
SYS:C/Echo $RC >SYS:amiguard-ae-m3-4-rc.txt
SYS:C/Echo "M3_4_AFTER_AMIGUARD_AE=1" >SYS:amiguard-ae-m3-4-after.txt
EOF
rm -f "$aros_root"/amiguard-ae-m3-4-{started,output,rc,after}.txt
config="$OUT_DIR/aros-guest.fs-uae"; sed "s|@AROS_ROOT@|$PWD/$aros_root|" ci/fs-uae/aros-guest.fs-uae > "$config"; fs-uae --version > "$OUT_DIR/fs-uae-version.txt" 2>&1 || true
set +e; timeout 45s xvfb-run -a fs-uae "$config" > "$OUT_DIR/fs-uae.log" 2>&1; rc=$?; set -e
started="$aros_root/amiguard-ae-m3-4-started.txt"; output="$aros_root/amiguard-ae-m3-4-output.txt"; guest_rc="$aros_root/amiguard-ae-m3-4-rc.txt"; after="$aros_root/amiguard-ae-m3-4-after.txt"
status=FAIL; observation=guest_result_missing
if [[ -f "$started" && -f "$after" && -f "$output" && -f "$guest_rc" ]] && grep -q '^0' "$guest_rc" && grep -q 'M3.4 AROS signature status smoke: PASS' "$output" && grep -q 'SMOKE PASS STATUS => READY M3.4 scanner=connected' "$output" && grep -q 'SMOKE PASS SIGNATURE.STATUS => READY COUNT=4 UPDATE=UNAVAILABLE' "$output" && grep -q 'SMOKE PASS SIGNATURE.STATUS now => ERROR SIGNATURE.STATUS takes no arguments' "$output"; then status=PASS; observation=native_68k_signature_status_api_qualified_in_aros; elif [[ -f "$after" ]]; then observation=guest_smoke_returned_without_expected_m3_4_evidence; fi
{
echo "STATUS=$status"; echo "GATE=M3_4_AROS_SIGNATURE_STATUS_SMOKE"; echo "MODEL=A1200"; echo "KICKSTART=internal"; echo "FS_UAE_EXIT=$rc"; echo "OBSERVATION=$observation"; echo "AROS_REXXMAST=${rexxmast_host:-MISSING}"; echo "AROS_REXXSYSLIB=${rexxlib_host:-MISSING}"; echo "AREXX_RUNTIME_QUALIFICATION=DEFERRED_LOCAL_CLASSIC_AMIGAOS"; [[ -f "$guest_rc" ]] && tr -d '\r' < "$guest_rc" | sed 's/^/GUEST_RC=/' || true; [[ -f "$output" ]] && tr -d '\r' < "$output" | sed 's/^/GUEST_OUTPUT=/' || true;
} | tee "$OUT_DIR/result.txt"
[[ "$status" == PASS ]]
