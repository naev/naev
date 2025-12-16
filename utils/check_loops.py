#!/usr/bin/env python3
"""
Quick loop edge inspector.

Decodes audio files with ffmpeg, then reports start/end sample values, RMS of
the first/last window, distance to the last zero crossing, and how biased the
edges are from zero. Helpful for spotting loop seams that may click when wrapped.
"""

from __future__ import annotations

import argparse
import math
import subprocess
from array import array
from pathlib import Path
from typing import Iterable


def decode_to_f32le(path: Path, sample_rate: int) -> array:
    """Decode ``path`` to mono f32le at the given sample rate using ffmpeg."""
    cmd = [
        "ffmpeg",
        "-v",
        "error",
        "-i",
        str(path),
        "-f",
        "f32le",
        "-ac",
        "1",
        "-ar",
        str(sample_rate),
        "-",
    ]
    proc = subprocess.Popen(cmd, stdout=subprocess.PIPE)
    raw = proc.stdout.read() if proc.stdout else b""
    proc.wait()
    if proc.returncode != 0:
        raise RuntimeError(f"ffmpeg failed on {path}")
    samples = array("f")
    samples.frombytes(raw)
    return samples


def rms(samples: array, count: int) -> float:
    """Root mean square of the first ``count`` samples (clamped to length)."""
    if not samples:
        return 0.0
    count = min(len(samples), count)
    acc = sum(s * s for s in samples[:count])
    return math.sqrt(acc / count)


def last_zero_crossing(samples: array) -> int | None:
    """Index (from start) of the last zero crossing; None if none found."""
    last = None
    for i in range(len(samples) - 1):
        if (samples[i] >= 0) != (samples[i + 1] >= 0):
            last = i
    return last


def inspect(path: Path, sample_rate: int, rms_window_ms: float) -> str:
    samples = decode_to_f32le(path, sample_rate)
    if not samples:
        return f"{path.name}: no samples"

    window = int(sample_rate * (rms_window_ms / 1000.0))
    first = samples[0]
    last = samples[-1]
    duration = len(samples) / sample_rate
    start_rms = rms(samples, window)
    end_rms = rms(samples[::-1], window)  # reverse for trailing window
    # Edge bias: how far the averages are from zero where a clean loop wants them.
    start_mean = (
        sum(samples[:window]) / window if window and len(samples) >= window else 0.0
    )
    end_mean = (
        sum(samples[-window:]) / window if window and len(samples) >= window else 0.0
    )
    zc = last_zero_crossing(samples)
    zc_from_end = len(samples) - zc - 1 if zc is not None else None
    zc_ms = (zc_from_end / sample_rate * 1000.0) if zc_from_end is not None else None

    parts = [
        f"{path.name}",
        f"dur: {duration:.3f}s",
        f"first/last: {first:+.6f} / {last:+.6f} (Δ {last - first:+.6f})",
        f"rms start/end ({rms_window_ms:.1f} ms): {start_rms:.6f} / {end_rms:.6f}",
        f"mean start/end: {start_mean:+.6f} / {end_mean:+.6f} (Δ {end_mean - start_mean:+.6f})",
    ]
    if zc_from_end is not None:
        parts.append(f"last zero-cross: {zc_from_end} samples ({zc_ms:.2f} ms) from end")
    else:
        parts.append("last zero-cross: none")
    return " | ".join(parts)


def main(argv: Iterable[str] | None = None) -> None:
    parser = argparse.ArgumentParser(
        description="Inspect loop endpoints for potential wrap pops."
    )
    parser.add_argument("files", nargs="+", type=Path, help="audio files to inspect")
    parser.add_argument(
        "--rate",
        type=int,
        default=48_000,
        help="decode sample rate (default: 48000)",
    )
    parser.add_argument(
        "--rms-window",
        type=float,
        default=10.0,
        help="RMS window in milliseconds for start/end (default: 10 ms)",
    )
    args = parser.parse_args(list(argv) if argv is not None else None)

    for f in args.files:
        try:
            print(inspect(f, args.rate, args.rms_window))
        except Exception as e:  # pragma: no cover - best-effort tool
            print(f"{f.name}: error {e}")

if __name__ == "__main__":
    main()
