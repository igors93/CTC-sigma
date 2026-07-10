from __future__ import annotations

import os
import sys
from pathlib import Path

PROJECT_ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(Path(__file__).resolve().parent))


def pytest_configure(config):
    default_library = PROJECT_ROOT / "build" / "libctc_sigma.so"
    os.environ.setdefault("CTC_SIGMA_LIBRARY", str(default_library))
