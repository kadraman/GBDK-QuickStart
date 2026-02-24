#!/usr/bin/env python3
"""
generate_assets.py
==================
Master script: runs all three asset generators in sequence.

Usage
-----
    python3 tools/generate_assets.py

Or via Make:
    make generate

Each generator can also be run individually:
    python3 tools/gen_background.py
    python3 tools/gen_font.py
    python3 tools/gen_sprite.py

Requirements
------------
    pip install pillow
"""

import os
import sys

TOOLS_DIR = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, TOOLS_DIR)

import gen_background
import gen_font
import gen_sprite


def main():
    print("=== Generating background assets ===")
    gen_background.main()
    print()
    print("=== Generating font assets ===")
    gen_font.main()
    print()
    print("=== Generating sprite assets ===")
    gen_sprite.main()
    print()
    print("All assets generated successfully.")


if __name__ == '__main__':
    main()
