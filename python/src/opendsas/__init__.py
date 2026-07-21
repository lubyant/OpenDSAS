"""Thin launcher that execs the static ``dsas`` binary bundled in this wheel."""

import os
import subprocess
import sys
from pathlib import Path

__version__ = "0.0.0"


def _binary_path() -> Path:
    bin_dir = Path(__file__).parent / "bin"
    candidate = bin_dir / ("dsas.exe" if os.name == "nt" else "dsas")
    if not candidate.exists():
        raise FileNotFoundError(
            f"bundled dsas binary not found at {candidate} — this wheel appears "
            "to be corrupt, or was installed for the wrong platform/architecture"
        )
    return candidate


def main() -> None:
    binary = _binary_path()
    if os.name != "nt":
        os.chmod(binary, 0o755)
        os.execv(str(binary), [str(binary), *sys.argv[1:]])
    else:
        raise SystemExit(subprocess.run([str(binary), *sys.argv[1:]]).returncode)


if __name__ == "__main__":
    main()
