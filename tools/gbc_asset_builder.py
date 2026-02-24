"""
gbc_asset_builder.py
====================
Reusable helper library for generating Game Boy Color (GBC) tile assets.

It provides functions to:
  - Convert pixel arrays to GBDK 2bpp tile byte format
  - Read 8x8 tiles from an indexed PIL Image
  - Write GBDK-compatible .c and .h source files for backgrounds, fonts,
    and sprites
  - Create indexed-colour PNG files from pixel arrays

Typical usage
-------------
Import this module from a generator script:

    from gbc_asset_builder import (
        pixels_to_2bpp, tiles_to_2bpp_bytes, png_to_tiles,
        make_indexed_png,
        write_background_files, write_font_files, write_sprite_files,
    )

Requirements
------------
    pip install pillow
"""

import os
import textwrap

try:
    from PIL import Image
except ImportError:
    raise ImportError("Pillow is required: pip install pillow")


# ---------------------------------------------------------------------------
# GBDK 2bpp conversion helpers
# ---------------------------------------------------------------------------

def pixels_to_2bpp(rows_8x8):
    """Convert an 8x8 pixel tile to 16 bytes of GBDK 2bpp tile data.

    Parameters
    ----------
    rows_8x8 : list of 8 lists, each containing 8 int pixel values (0-3).
        Pixel (col 0) is the leftmost pixel; colour index 0-3 maps to
        the four palette entries for that tile.

    Returns
    -------
    list of 16 int bytes ready to be written into a C array.

    GBDK 2bpp format
    ~~~~~~~~~~~~~~~~
    Each of the 8 pixel rows produces two bytes:
      - Low-plane byte:  bit N is set when pixel N's colour index has bit 0 set
      - High-plane byte: bit N is set when pixel N's colour index has bit 1 set
    Bit 7 of each byte corresponds to the leftmost (col 0) pixel.
    """
    result = []
    for row in rows_8x8:
        low_byte = 0
        high_byte = 0
        for bit_pos, color in enumerate(row):
            mask = 0x80 >> bit_pos
            if color & 1:
                low_byte  |= mask
            if color & 2:
                high_byte |= mask
        result.append(low_byte)
        result.append(high_byte)
    return result


def tiles_to_2bpp_bytes(tiles):
    """Convert a list of 8x8 pixel tiles to a flat list of 2bpp bytes.

    Parameters
    ----------
    tiles : list of tiles, where each tile is a list of 8 rows of 8 ints.

    Returns
    -------
    list of ints (16 bytes × number of tiles).
    """
    out = []
    for tile in tiles:
        out.extend(pixels_to_2bpp(tile))
    return out


# ---------------------------------------------------------------------------
# PNG helpers
# ---------------------------------------------------------------------------

def make_indexed_png(pixel_grid, palette_rgb, output_path):
    """Create an indexed-colour PNG from a 2-D pixel array.

    Parameters
    ----------
    pixel_grid : list of rows, each row is a list of int colour indices.
        All values must be in [0, len(palette_rgb)-1].
    palette_rgb : list of (R, G, B) tuples (up to 256 entries).
    output_path : str – destination file path.
    """
    height = len(pixel_grid)
    width  = len(pixel_grid[0])
    img = Image.new('P', (width, height))
    flat_pal = []
    for r, g, b in palette_rgb:
        flat_pal.extend([r, g, b])
    # Pad palette to 256 entries
    flat_pal += [0] * (768 - len(flat_pal))
    img.putpalette(flat_pal)
    pixels = img.load()
    for y, row in enumerate(pixel_grid):
        for x, val in enumerate(row):
            pixels[x, y] = val
    img.save(output_path)


def png_to_tiles(png_path):
    """Load an indexed PNG and extract 8x8 tile pixel data.

    Tiles are read in row-major order: left-to-right across the image width,
    then top-to-bottom.  The image dimensions must be multiples of 8.

    Parameters
    ----------
    png_path : str – path to an indexed ('P' mode) PNG.

    Returns
    -------
    list of tiles; each tile is a list of 8 rows of 8 pixel-index ints.
    """
    img = Image.open(png_path)
    if img.mode != 'P':
        img = img.convert('P')
    w, h = img.size
    assert w % 8 == 0 and h % 8 == 0, (
        f"PNG dimensions ({w}x{h}) must be multiples of 8")
    pixels = img.load()
    tiles = []
    for ty in range(h // 8):
        for tx in range(w // 8):
            tile = []
            for row in range(8):
                tile.append([pixels[tx * 8 + col, ty * 8 + row] for col in range(8)])
            tiles.append(tile)
    return tiles


# ---------------------------------------------------------------------------
# C source / header formatting helpers
# ---------------------------------------------------------------------------

def _format_c_bytes(data, bytes_per_line=16):
    """Format a list of ints as comma-separated C hex literals, 16 per line."""
    lines = []
    for i in range(0, len(data), bytes_per_line):
        chunk = data[i:i + bytes_per_line]
        comma = ',' if i + bytes_per_line < len(data) else ''
        lines.append('    ' + ', '.join(f'0x{b:02X}U' for b in chunk) + comma)
    return '\n'.join(lines)


def _rgb8_literal(r, g, b):
    """Return GBDK RGB8() macro call string for an (r,g,b) tuple."""
    return f'RGB8({r:3d},{g:3d},{b:3d})'


def _format_palette_array(palette_rows, colors_per_row=4):
    """Format a flat list of (r,g,b) tuples as a palette_color_t initialiser.

    Parameters
    ----------
    palette_rows : list of (r,g,b) tuples – all palette colours in order.
    colors_per_row : int – how many RGB8() entries to place per source line.
    """
    lines = []
    for i in range(0, len(palette_rows), colors_per_row):
        chunk = palette_rows[i:i + colors_per_row]
        comma = ',' if i + colors_per_row < len(palette_rows) else ''
        lines.append('    ' + ', '.join(_rgb8_literal(*c) for c in chunk) + comma)
    return '\n'.join(lines)


# ---------------------------------------------------------------------------
# Background file writers
# ---------------------------------------------------------------------------

def write_background_files(name, tiles, tilemap, palette_colors,
                            map_width, map_height, out_dir='.'):
    """Write background .c and .h files.

    Parameters
    ----------
    name : str – base name (e.g. 'background'); used for filenames and symbols.
    tiles : list of 8x8 pixel tiles (list of list of list of int).
    tilemap : list of int tile indices, length == map_width * map_height.
    palette_colors : list of (r,g,b) tuples, length == n_palettes * 4.
    map_width, map_height : int – tilemap dimensions in tiles.
    out_dir : str – output directory (default: current directory).
    """
    tile_count   = len(tiles)
    palette_count = len(palette_colors) // 4
    tile_bytes   = tiles_to_2bpp_bytes(tiles)
    total_tile_b = len(tile_bytes)
    total_map_b  = len(tilemap)
    pal_count_total = len(palette_colors)
    NAME = name.upper()

    # --- .c file ---
    c_lines = [
        f'/* Auto-generated by tools/gen_{name}.py – edit that script to change. */',
        f'#include "{name}.h"',
        '',
        f'/* GBC background palettes ({palette_count} palette{"s" if palette_count != 1 else ""} x 4 colors each) */',
        f'const palette_color_t {name}_palettes[{pal_count_total}] = {{',
        _format_palette_array(palette_colors),
        '};',
        '',
        f'/* Background tile data ({tile_count} tiles, 16 bytes each) */',
        f'const uint8_t {name}_tiles[{total_tile_b}] = {{',
        _format_c_bytes(tile_bytes),
        '};',
        '',
        f'/* Background tilemap ({map_width}x{map_height} = {total_map_b} bytes) */',
        f'const uint8_t {name}_map[{total_map_b}] = {{',
        _format_c_bytes(tilemap),
        '};',
    ]
    c_path = os.path.join(out_dir, f'{name}.c')
    with open(c_path, 'w') as f:
        f.write('\n'.join(c_lines) + '\n')
    print(f'Written {c_path}')

    # --- .h file ---
    h_guard = f'{NAME}_H'
    h_lines = [
        f'/* Auto-generated by tools/gen_{name}.py – edit that script to change. */',
        f'#ifndef {h_guard}',
        f'#define {h_guard}',
        '',
        '#include <gb/cgb.h>',
        '#include <stdint.h>',
        '',
        f'#define {NAME}_TILE_COUNT    {tile_count}U',
        f'#define {NAME}_PALETTE_COUNT {palette_count}U',
        f'#define {NAME}_MAP_WIDTH     {map_width}U',
        f'#define {NAME}_MAP_HEIGHT    {map_height}U',
        '',
        f'extern const palette_color_t {name}_palettes[{pal_count_total}];',
        f'extern const uint8_t {name}_tiles[{total_tile_b}];',
        f'extern const uint8_t {name}_map[{total_map_b}];',
        '',
        f'#endif',
    ]
    h_path = os.path.join(out_dir, f'{name}.h')
    with open(h_path, 'w') as f:
        f.write('\n'.join(h_lines) + '\n')
    print(f'Written {h_path}')


# ---------------------------------------------------------------------------
# Font file writers
# ---------------------------------------------------------------------------

def write_font_files(name, tiles, palette_colors, out_dir='.'):
    """Write font .c and .h files.

    Parameters
    ----------
    name : str – base name (e.g. 'font').
    tiles : list of 8x8 pixel tiles (96 tiles for ASCII 32-127).
    palette_colors : list of (r,g,b) tuples, length == 4.
    out_dir : str – output directory.
    """
    tile_count   = len(tiles)
    palette_count = len(palette_colors) // 4
    tile_bytes   = tiles_to_2bpp_bytes(tiles)
    total_tile_b = len(tile_bytes)
    pal_count_total = len(palette_colors)
    NAME = name.upper()

    c_lines = [
        f'/* Auto-generated by tools/gen_{name}.py – edit that script to change. */',
        f'/* Loaded at GBC background palette slot 2 via set_bkg_palette(2, ...). */',
        f'/* Color 0 = background color, Color 1 = text color. */',
        f'#include "{name}.h"',
        '',
        f'/* GBC font palette (1 palette x 4 colors) */',
        f'const palette_color_t {name}_palettes[{pal_count_total}] = {{',
        _format_palette_array(palette_colors),
        '};',
        '',
        f'/* Font tile data ({tile_count} tiles for ASCII 32-127, 16 bytes each) */',
        f'const uint8_t {name}_tiles[{total_tile_b}] = {{',
        _format_c_bytes(tile_bytes),
        '};',
    ]
    c_path = os.path.join(out_dir, f'{name}.c')
    with open(c_path, 'w') as f:
        f.write('\n'.join(c_lines) + '\n')
    print(f'Written {c_path}')

    h_guard = f'{NAME}_H'
    h_lines = [
        f'/* Auto-generated by tools/gen_{name}.py – edit that script to change. */',
        f'#ifndef {h_guard}',
        f'#define {h_guard}',
        '',
        '#include <gb/cgb.h>',
        '#include <stdint.h>',
        '',
        f'#define {NAME}_TILE_COUNT    {tile_count}U',
        f'#define {NAME}_PALETTE_COUNT {palette_count}U',
        '',
        f'extern const palette_color_t {name}_palettes[{pal_count_total}];',
        f'extern const uint8_t {name}_tiles[{total_tile_b}];',
        '',
        f'#endif',
    ]
    h_path = os.path.join(out_dir, f'{name}.h')
    with open(h_path, 'w') as f:
        f.write('\n'.join(h_lines) + '\n')
    print(f'Written {h_path}')


# ---------------------------------------------------------------------------
# Sprite file writers
# ---------------------------------------------------------------------------

def write_sprite_files(name, frames_top, frames_bottom, palette_colors,
                        out_dir='.'):
    """Write sprite .c and .h files for 8x16 sprite mode.

    Each animation frame consists of two 8x8 tiles (top half and bottom half).
    Tiles are interleaved: [frame0_top, frame0_bottom, frame1_top, ...].

    Parameters
    ----------
    name : str – base name (e.g. 'sprite').
    frames_top : list of 8x8 pixel tiles, one per animation frame (top halves).
    frames_bottom : list of 8x8 pixel tiles, one per animation frame (bottom halves).
    palette_colors : list of (r,g,b) tuples (length == 4).
        Color index 0 is treated as transparent on GBC sprites.
    out_dir : str – output directory.
    """
    assert len(frames_top) == len(frames_bottom), \
        "frames_top and frames_bottom must have the same length"
    frame_count = len(frames_top)
    tiles_per_frame = 2
    tile_count = frame_count * tiles_per_frame
    pal_count_total = len(palette_colors)

    # Interleave top and bottom tiles
    all_tiles = []
    for top, bot in zip(frames_top, frames_bottom):
        all_tiles.append(top)
        all_tiles.append(bot)
    tile_bytes = tiles_to_2bpp_bytes(all_tiles)
    total_tile_b = len(tile_bytes)
    NAME = name.upper()

    c_lines = [
        f'/* Auto-generated by tools/gen_{name}.py – edit that script to change. */',
        f'#include "{name}.h"',
        '',
        f'/* GBC sprite palette (1 palette x 4 colors) */',
        f'/* Color index 0 is transparent on GBC sprites. */',
        f'const palette_color_t {name}_palettes[{pal_count_total}] = {{',
        _format_palette_array(palette_colors),
        '};',
        '',
        (f'/* Sprite tile data: {frame_count} animation frames x {tiles_per_frame} tiles per frame'
         f' (8x16 mode),\n'
         f'   {tile_count} tiles total, 16 bytes per tile = {total_tile_b} bytes */'),
        f'const uint8_t {name}_tiles[{total_tile_b}] = {{',
        _format_c_bytes(tile_bytes),
        '};',
    ]
    c_path = os.path.join(out_dir, f'{name}.c')
    with open(c_path, 'w') as f:
        f.write('\n'.join(c_lines) + '\n')
    print(f'Written {c_path}')

    h_guard = f'{NAME}_H'
    h_lines = [
        f'/* Auto-generated by tools/gen_{name}.py – edit that script to change. */',
        f'#ifndef {h_guard}',
        f'#define {h_guard}',
        '',
        '#include <gb/cgb.h>',
        '#include <stdint.h>',
        '',
        f'#define {NAME}_TILE_COUNT      {tile_count}U',
        f'#define {NAME}_PALETTE_COUNT    1U',
        f'#define {NAME}_FRAME_COUNT      {frame_count}U',
        f'#define {NAME}_TILES_PER_FRAME  {tiles_per_frame}U',
        '',
        f'extern const palette_color_t {name}_palettes[{pal_count_total}];',
        f'extern const uint8_t {name}_tiles[{total_tile_b}];',
        '',
        f'#endif',
    ]
    h_path = os.path.join(out_dir, f'{name}.h')
    with open(h_path, 'w') as f:
        f.write('\n'.join(h_lines) + '\n')
    print(f'Written {h_path}')
