"""
gbc_asset_builder.py
====================
Reusable helper library for generating Game Boy Color (GBC) tile assets.

Provides:
  - 2bpp conversion (pixels_to_2bpp, tiles_to_2bpp_bytes)
  - PNG I/O (make_indexed_png, png_to_tiles)
  - .c/.h writers for backgrounds, fonts, and sprites
    (write_background_files, write_font_files,
     write_sprite_files, write_sprite_files_animated)

Requirements:  pip install pillow
"""

import os

try:
    from PIL import Image
except ImportError:
    raise ImportError("Pillow is required: pip install pillow")


# ---------------------------------------------------------------------------
# GBDK 2bpp conversion helpers
# ---------------------------------------------------------------------------

def pixels_to_2bpp(rows_8x8):
    """Convert an 8x8 pixel tile to 16 bytes of GBDK 2bpp tile data.

    rows_8x8 : list of 8 lists of 8 int colour indices (0-3).
    Bit 7 of each byte = leftmost pixel (col 0).
    """
    result = []
    for row in rows_8x8:
        low_byte = high_byte = 0
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
    """Flatten a list of 8x8 pixel tiles into 2bpp bytes (16 bytes per tile)."""
    out = []
    for tile in tiles:
        out.extend(pixels_to_2bpp(tile))
    return out


# ---------------------------------------------------------------------------
# PNG helpers
# ---------------------------------------------------------------------------

def make_indexed_png(pixel_grid, palette_rgb, output_path):
    """Write an indexed-colour PNG from a 2-D pixel array.

    pixel_grid  : list of rows, each row is a list of int colour indices.
    palette_rgb : list of (R, G, B) tuples (up to 256 entries).
    output_path : destination file path (saved exactly as given).
    """
    height = len(pixel_grid)
    width  = len(pixel_grid[0])
    img = Image.new('P', (width, height))
    flat_pal = []
    for r, g, b in palette_rgb:
        flat_pal.extend([r, g, b])
    flat_pal += [0] * (768 - len(flat_pal))
    img.putpalette(flat_pal)
    pixels = img.load()
    for y, row in enumerate(pixel_grid):
        for x, val in enumerate(row):
            pixels[x, y] = val
    img.save(output_path)


def png_to_tiles(png_path):
    """Load an indexed PNG and extract 8x8 tile pixel data (row-major order).

    Image dimensions must be multiples of 8.
    Returns a list of tiles; each tile is 8 rows of 8 colour-index ints.
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
    """Format ints as comma-separated C hex literals, bytes_per_line per line."""
    lines = []
    for i in range(0, len(data), bytes_per_line):
        chunk = data[i:i + bytes_per_line]
        comma = ',' if i + bytes_per_line < len(data) else ''
        lines.append('    ' + ', '.join(f'0x{b:02X}U' for b in chunk) + comma)
    return '\n'.join(lines)


def _rgb8_literal(r, g, b):
    return f'RGB8({r:3d},{g:3d},{b:3d})'


def _format_palette_array(palette_rows, colors_per_row=4):
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
                            map_width, map_height, out_dir='.', attr_map=None):
    """Write background .c and .h files.

    name           : base name, e.g. 'background'.
    tiles          : list of 8x8 pixel tiles.
    tilemap        : flat list of tile indices (map_width * map_height).
    palette_colors : (r,g,b) tuples, length == n_palettes * 4.
    map_width/height : tilemap dimensions in tiles.
    attr_map       : optional flat list of per-tile palette attribute bytes.
                     When provided, exported as <name>_attr_map[].
    """
    tile_count      = len(tiles)
    palette_count   = len(palette_colors) // 4
    tile_bytes      = tiles_to_2bpp_bytes(tiles)
    total_tile_b    = len(tile_bytes)
    total_map_b     = len(tilemap)
    pal_count_total = len(palette_colors)
    NAME = name.upper()

    c_lines = [
        f'/* Auto-generated by tools/gen_{name}.py - edit that script to change. */',
        f'#include "{name}.h"',
        '',
        f'/* GBC background palettes ({palette_count} palette{"s" if palette_count!=1 else ""} x 4 colors each) */',
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
    if attr_map is not None:
        n_attr = len(attr_map)
        c_lines += [
            '',
            f'/* Per-tile palette attribute map ({map_width}x{map_height} = {n_attr} bytes).',
            f'   Each byte is the GBC VRAM bank-1 attribute: 0x00=sky palette, 0x01=ground palette. */',
            f'const uint8_t {name}_attr_map[{n_attr}] = {{',
            _format_c_bytes(attr_map),
            '};',
        ]

    c_path = os.path.join(out_dir, f'{name}.c')
    with open(c_path, 'w', encoding='utf-8') as f:
        f.write('\n'.join(c_lines) + '\n')
    print(f'Written {c_path}')

    h_guard = f'{NAME}_H'
    h_lines = [
        f'/* Auto-generated by tools/gen_{name}.py - edit that script to change. */',
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
    ]
    if attr_map is not None:
        n_attr = len(attr_map)
        h_lines.append(f'extern const uint8_t {name}_attr_map[{n_attr}];')
    h_lines += ['', '#endif']

    h_path = os.path.join(out_dir, f'{name}.h')
    with open(h_path, 'w', encoding='utf-8') as f:
        f.write('\n'.join(h_lines) + '\n')
    print(f'Written {h_path}')


# ---------------------------------------------------------------------------
# Font file writers
# ---------------------------------------------------------------------------

def write_font_files(name, tiles, palette_colors, extra_defines=None, out_dir='.'):
    """Write font .c and .h files.

    extra_defines : optional list of (define_name, value, comment) tuples
                    appended to the .h file after FONT_TILE_COUNT.
    """
    tile_count      = len(tiles)
    palette_count   = len(palette_colors) // 4
    tile_bytes      = tiles_to_2bpp_bytes(tiles)
    total_tile_b    = len(tile_bytes)
    pal_count_total = len(palette_colors)
    NAME = name.upper()

    c_lines = [
        f'/* Auto-generated by tools/gen_{name}.py - edit that script to change. */',
        f'/* Loaded at GBC background palette slot 2 via set_bkg_palette(2, ...). */',
        f'/* Color 0 = background, Color 1 = text. */',
        f'#include "{name}.h"',
        '',
        f'/* GBC font palette (1 palette x 4 colors) */',
        f'const palette_color_t {name}_palettes[{pal_count_total}] = {{',
        _format_palette_array(palette_colors),
        '};',
        '',
        f'/* Font tile data ({tile_count} tiles, 16 bytes each) */',
        f'const uint8_t {name}_tiles[{total_tile_b}] = {{',
        _format_c_bytes(tile_bytes),
        '};',
    ]
    c_path = os.path.join(out_dir, f'{name}.c')
    with open(c_path, 'w', encoding='utf-8') as f:
        f.write('\n'.join(c_lines) + '\n')
    print(f'Written {c_path}')

    h_guard = f'{NAME}_H'
    h_lines = [
        f'/* Auto-generated by tools/gen_{name}.py - edit that script to change. */',
        f'#ifndef {h_guard}',
        f'#define {h_guard}',
        '',
        '#include <gb/cgb.h>',
        '#include <stdint.h>',
        '',
        f'#define {NAME}_TILE_COUNT    {tile_count}U',
        f'#define {NAME}_PALETTE_COUNT {palette_count}U',
    ]
    if extra_defines:
        h_lines.append('')
        for def_name, def_val, def_comment in extra_defines:
            comment = f'  /* {def_comment} */' if def_comment else ''
            h_lines.append(f'#define {def_name.upper()}  {def_val}U{comment}')
    h_lines += [
        '',
        f'extern const palette_color_t {name}_palettes[{pal_count_total}];',
        f'extern const uint8_t {name}_tiles[{total_tile_b}];',
        '',
        '#endif',
    ]
    h_path = os.path.join(out_dir, f'{name}.h')
    with open(h_path, 'w', encoding='utf-8') as f:
        f.write('\n'.join(h_lines) + '\n')
    print(f'Written {h_path}')


# ---------------------------------------------------------------------------
# Sprite file writers (simple: single-animation, top+bottom frames)
# ---------------------------------------------------------------------------

def write_sprite_files(name, frames_top, frames_bottom, palette_colors, out_dir='.'):
    """Write sprite .c and .h for a single walk-cycle in 8x16 sprite mode."""
    assert len(frames_top) == len(frames_bottom)
    frame_count     = len(frames_top)
    tiles_per_frame = 2
    tile_count      = frame_count * tiles_per_frame
    pal_count_total = len(palette_colors)
    all_tiles = []
    for top, bot in zip(frames_top, frames_bottom):
        all_tiles.append(top)
        all_tiles.append(bot)
    tile_bytes  = tiles_to_2bpp_bytes(all_tiles)
    total_tile_b = len(tile_bytes)
    NAME = name.upper()

    c_lines = [
        f'/* Auto-generated by tools/gen_{name}.py - edit that script to change. */',
        f'#include "{name}.h"',
        '',
        f'/* GBC sprite palette (1 palette x 4 colors). Index 0 = transparent. */',
        f'const palette_color_t {name}_palettes[{pal_count_total}] = {{',
        _format_palette_array(palette_colors),
        '};',
        '',
        f'/* Sprite tiles: {frame_count} frames x {tiles_per_frame} tiles = {tile_count} tiles */',
        f'const uint8_t {name}_tiles[{total_tile_b}] = {{',
        _format_c_bytes(tile_bytes),
        '};',
    ]
    c_path = os.path.join(out_dir, f'{name}.c')
    with open(c_path, 'w', encoding='utf-8') as f:
        f.write('\n'.join(c_lines) + '\n')
    print(f'Written {c_path}')

    h_guard = f'{NAME}_H'
    h_lines = [
        f'/* Auto-generated by tools/gen_{name}.py - edit that script to change. */',
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
        '#endif',
    ]
    h_path = os.path.join(out_dir, f'{name}.h')
    with open(h_path, 'w', encoding='utf-8') as f:
        f.write('\n'.join(h_lines) + '\n')
    print(f'Written {h_path}')


# ---------------------------------------------------------------------------
# Sprite file writers (animated: multiple named animations)
# ---------------------------------------------------------------------------

def write_sprite_files_animated(name, animations, palette_colors,
                                   pixel_chars, out_dir='.', size='8x16'):
    """Write .c and .h for a sprite with multiple named animations.

    name          : base symbol name, e.g. 'player'.
    animations    : dict { anim_name: [frame, ...] }
                    Frame format depends on *size*:
                      '8x16' - (top_rows, bot_rows)          2 tiles/frame
                      '8x8'  - (content_rows,)               2 tiles/frame
                                 stored as [content, blank] so content appears
                                 in the top half of the 8x16 hardware slot;
                                 use OAM Y = GROUND_Y + 8 to land on ground.
                      '16x16'- (l_top, l_bot, r_top, r_bot)  4 tiles/frame
                                 two side-by-side 8x16 OBJ slots.
    palette_colors: list of (r,g,b) tuples (length == 4).
    pixel_chars   : dict mapping character -> colour index (must include '.': 0).
    out_dir       : output directory.
    size          : sprite size string: '8x8', '8x16', or '16x16'.
    """
    def _parse_tile(string_rows):
        tile = []
        for row_str in string_rows:
            row = [pixel_chars.get(c, 0) for c in row_str]
            assert len(row) == 8, f"Row '{row_str}' must be exactly 8 chars"
            tile.append(row)
        assert len(tile) == 8, f"Tile must have exactly 8 rows (got {len(tile)})"
        return tile

    _blank_tile = [[0] * 8 for _ in range(8)]

    # Flatten all animations; track start tile index per animation
    anim_info  = []   # (anim_name, start_tile_idx, frame_count)
    all_tiles  = []

    if size == '16x16':
        tiles_per_frame = 4
        for anim_name, frames in animations.items():
            start_tile = len(all_tiles)
            for l_top, l_bot, r_top, r_bot in frames:
                all_tiles.append(_parse_tile(l_top))
                all_tiles.append(_parse_tile(l_bot))
                all_tiles.append(_parse_tile(r_top))
                all_tiles.append(_parse_tile(r_bot))
            anim_info.append((anim_name, start_tile, len(frames)))
    elif size == '8x8':
        tiles_per_frame = 2
        for anim_name, frames in animations.items():
            start_tile = len(all_tiles)
            for (content_rows,) in frames:
                all_tiles.append(_parse_tile(content_rows))
                all_tiles.append(_blank_tile)
            anim_info.append((anim_name, start_tile, len(frames)))
    else:  # '8x16' - default / backward-compatible
        tiles_per_frame = 2
        for anim_name, frames in animations.items():
            start_tile = len(all_tiles)
            for top_rows, bot_rows in frames:
                all_tiles.append(_parse_tile(top_rows))
                all_tiles.append(_parse_tile(bot_rows))
            anim_info.append((anim_name, start_tile, len(frames)))

    tile_count      = len(all_tiles)
    n_frames_total  = tile_count // tiles_per_frame
    pal_count_total = len(palette_colors)
    tile_bytes      = tiles_to_2bpp_bytes(all_tiles)
    total_tile_b    = len(tile_bytes)
    NAME = name.upper()

    # .c file
    c_lines = [
        f'/* Auto-generated by tools/gen_sprite.py for sprite "{name}". */',
        f'#include "{name}.h"',
        '',
        f'/* GBC sprite palette (1 palette x 4 colors). Index 0 = transparent. */',
        f'const palette_color_t {name}_palettes[{pal_count_total}] = {{',
        _format_palette_array(palette_colors),
        '};',
        '',
        f'/* Sprite tiles: {tile_count} tiles total ({n_frames_total} frames, {tiles_per_frame} tiles/frame) */',
        f'const uint8_t {name}_tiles[{total_tile_b}] = {{',
        _format_c_bytes(tile_bytes),
        '};',
    ]
    c_path = os.path.join(out_dir, f'{name}.c')
    with open(c_path, 'w', encoding='utf-8') as f:
        f.write('\n'.join(c_lines) + '\n')
    print(f'Written {c_path}')

    # .h file
    h_guard = f'{NAME}_H'
    h_lines = [
        f'/* Auto-generated by tools/gen_sprite.py for sprite "{name}". */',
        f'#ifndef {h_guard}',
        f'#define {h_guard}',
        '',
        '#include <gb/cgb.h>',
        '#include <stdint.h>',
        '',
        f'#define {NAME}_TILE_COUNT      {tile_count}U',
        f'#define {NAME}_PALETTE_COUNT    1U',
        f'#define {NAME}_TILES_PER_FRAME  {tiles_per_frame}U',
        '',
    ]
    for anim_name, start_tile, frame_count in anim_info:
        au = anim_name.upper()
        h_lines.append(f'/* Animation: {anim_name} */')
        h_lines.append(f'#define {NAME}_ANIM_{au}_START   {start_tile}U')
        h_lines.append(f'#define {NAME}_ANIM_{au}_FRAMES  {frame_count}U')
        h_lines.append('')
    h_lines += [
        f'extern const palette_color_t {name}_palettes[{pal_count_total}];',
        f'extern const uint8_t {name}_tiles[{total_tile_b}];',
        '',
        '#endif',
    ]
    h_path = os.path.join(out_dir, f'{name}.h')
    with open(h_path, 'w', encoding='utf-8') as f:
        f.write('\n'.join(h_lines) + '\n')
    print(f'Written {h_path}')
