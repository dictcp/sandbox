# Dependency upgrade security report

- Checked on 2026-10-27 using Trivy 0.76.1 and its freshly downloaded vulnerability database.
- Built both branches with `docker build --pull`.
- Base image: Debian 12 slim -> Debian 13 slim (current stable).
- udocker 1.3.17 is already the latest upstream release.
- Same-major track: `debian:12-slim` floats to the newest Debian 12 patch; no stale application pin to update and no empty PR is opened.
- Smoke check: `docker run --rm --network none --entrypoint /home/udocker/udocker/udocker dictcp-upgrade/sandbox:latest version` passes and reports 1.3.17.

## Security findings

- Baseline image: 885 package findings (14 critical, 285 high, 382 medium, 189 low, 15 unknown).
- Updated image: 830 package findings (12 critical, 275 high, 355 medium, 179 low, 9 unknown).
- Counts include duplicate vulnerabilities across bundled executables; they are scanner findings, not confirmed reachable exploits.
- 212 package/CVE pairs present before are absent after the update.
- Remaining critical CVEs: CVE-2023-24538, CVE-2023-24540, CVE-2024-24790, CVE-2025-68121. These are reported in old Go standard libraries embedded in udocker's downloaded execution helpers; upgrading Debian does not replace those binaries. Upstream udocker has no newer release to select. This PR does not claim to eliminate all security issues.
- Source: https://www.debian.org/releases/ and https://github.com/indigo-dc/udocker/releases/tag/1.3.17.
- Nested container execution depends on host capabilities and was not exercised by the version smoke check.
