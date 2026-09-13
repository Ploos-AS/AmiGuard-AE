# AmiGuard AE

**AmiGuard ARexx Edition** — an automation-oriented companion to AmiGuard for classic Amiga systems.

AmiGuard AE is intended to expose AmiGuard-compatible antivirus and signature functionality through a stable ARexx interface. ARexx is an interface layer rather than the scanner engine: scanner functionality must remain separated from RexxMast and reusable by other front ends.

## M0 baseline

- Target: **AmigaOS 2.04+**
- Baseline CPU: **68000**
- ARexx public port: **`AMIGUARD`**
- License: **MIT**
- Copyright: **Ploos AS**
- Scanner/backend and ARexx transport are separate architectural layers.
- Signature/scanning semantics should remain compatible with AmiGuard wherever practical.

M0 establishes the repository and API contract. It deliberately does **not** claim a functioning Amiga ARexx port or antivirus engine yet; those begin in M1/M2 and require runtime qualification.

## Layout

```text
include/             public/core interfaces
src/                 implementation skeleton
docs/AREXX_API.md    ARexx contract
examples/            ARexx examples
ROADMAP.md            milestone plan
```

## Host structural check

```sh
make clean
make check
make
./AmiGuardAE
```

The host build is only a structural smoke test. The native Amiga build will use the Bebbo `m68k-amigaos-gcc` toolchain beginning with M1.

## Planned ARexx surface

Core discovery/liveness starts with `PING`, `VERSION`, `STATUS` and `HELP`. Later milestones add scanning, signatures, structured results, quarantine/configuration and event-driven integration.

See [`docs/AREXX_API.md`](docs/AREXX_API.md) for the initial API contract and [`ROADMAP.md`](ROADMAP.md) for milestone gates.

## Ecosystem direction

AmiGuard AE is designed as automation glue for workflows such as AmiForensics/AmiSandbox analysis, signature-workstation pipelines, AmiGet package checks, and BBS upload scanning without coupling those consumers directly to scanner internals.

## Status

**M0 — Foundation established.**

Next: **M1 — native 68000 skeleton + live `AMIGUARD` ARexx port + initial FS-UAE qualification.**
