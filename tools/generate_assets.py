#!/usr/bin/env python3
"""
generate_assets.py
==================
Master script: regenerates all PNG and C/H assets.

Usage
-----
    python3 tools/generate_assets.py
    make generate

Each generator can also be run individually:
    python3 tools/gen_background.py   # processes all res/backgrounds/*/definition.py
    python3 tools/gen_font.py         # processes all res/fonts/*/definition.py
    python3 tools/gen_sprite.py       # processes all res/sprites/*/definition.py

Requirements:  pip install pillow
"""

import os, sys

TOOLS_DIR = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, TOOLS_DIR)

import gen_background
import gen_font
import gen_sprite


def main():
    print('=== Generating background assets (res/backgrounds/*) ===')
    gen_background.main()
    print()
    print('=== Generating font assets (res/fonts/*) ===')
    gen_font.main()
    print()
    print('=== Generating sprite assets (res/sprites/*) ===')
    gen_sprite.main()
    print()
    print('All assets generated successfully.')


if __name__ == '__main__':
    main()
