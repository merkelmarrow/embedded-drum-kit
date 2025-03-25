#!/usr/bin/env python3
import argparse
import subprocess
import numpy as np

def load_pcm_from_mp3(path: str) -> np.ndarray:
    cmd = [
        "ffmpeg", "-hide_banner", "-loglevel", "error",
        "-i", path,
        "-f", "s16le",     # 16-bit PCM
        "-acodec", "pcm_s16le",
        "-ar", "44100",    # sample rate
        "-ac", "1",        # mono
        "-"
    ]
    proc = subprocess.run(cmd, stdout=subprocess.PIPE, check=True)
    return np.frombuffer(proc.stdout, dtype=np.int16).astype(np.int32)

def convert_to_12bit(samples: np.ndarray) -> np.ndarray:
    # Map [-32768, +32767] to  [0, 4095], centre = 2048
    return np.clip(((samples / 32767.0) * 2047).astype(int) + 2048, 0, 4095)

def write_cpp_header(arr: np.ndarray, outfile: str, var: str):
    with open(outfile, "w") as f:
        f.write("#pragma once\n#include <cstdint>\n\n")
        f.write(f"const uint16_t {var}[] = {{\n")
        for i, val in enumerate(arr):
            if i % 12 == 0:
                f.write("    ")
            f.write(f"{val}")
            f.write(", " if i < len(arr)-1 else "\n")
            if i % 12 == 11:
                f.write("\n")
        f.write("};\n")
        f.write(f"const unsigned {var}_length = {len(arr)};\n")

def main():
    p = argparse.ArgumentParser()
    p.add_argument("input", help="Path to MP3")
    p.add_argument("output", help="Output .h file")
    p.add_argument("--var", default="audio_data", help="C++ variable name")
    args = p.parse_args()

    pcm = load_pcm_from_mp3(args.input)
    arr12 = convert_to_12bit(pcm)
    write_cpp_header(arr12, args.output, args.var)

if __name__ == "__main__":
    main()
