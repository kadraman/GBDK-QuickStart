# Copilot Instructions — Reusable SFX Library for GBDK Quickstart (CGB + hUGEDriver)

> **Context**: This repository targets **GBDK-2020 (latest 4.x)**, runs **music concurrently via hUGEDriver/hugetracker**, and targets **Game Boy Color (CGB)** with DMG compatibility where reasonable. Copilot should generate **small, C89-compatible** code that compiles with GBDK-2020, avoids dynamic allocation, and updates audio strictly per **VBlank**.

---

## 1) What Copilot should build

- A reusable **Sound Effects (SFX) library** that can be dropped into any GBDK project.
- **Public C API** in `include/sfx.h` (already spec'd in the Page). Keep the API stable.
- **Runtime** in `src/sfx.c` implementing:
  - `sfx_init()`, `sfx_update()`, `sfx_play()`, `sfx_is_busy()`, `sfx_stop_all()`, `sfx_set_music_mask()`
  - Optional procedural helpers: `sfx_play_jump()`, `sfx_play_laser()`, `sfx_play_explosion()`
- **Data model** in `src/sfx_defs.c` using the scripted-frames format and a couple of presets.
- **Example ROM** under `examples/sfx_quickstart/main.c` that maps buttons to SFX.
- **Docs** under `docs/SFX.md` that explain usage and integration with hUGEDriver.

> Use the API and structures exactly as specified in the Page titled
> **“Copilot Specification: Reusable Sound Effect Library for GBDK Quickstart.”**

---

## 2) Constraints & coding standards

- **Language**: C89/C90. Use `<stdint.h>` or GBDK types (`UBYTE`, `UINT8`). No C99+ features.
- **No dynamic memory**: No `malloc`/`free`. All data is `const` in ROM or static in WRAM.
- **Timing**: All multi-step SFX advance in `sfx_update()` which is called **once per VBlank** from the main loop. No busy-waits.
- **Registers**: Use write patterns for DMG/CGB APU. Always set the **trigger bit (bit7)** for NR1x/NR2x/NR3x/NR4x final control registers when starting a tone/noise.
- **CGB specifics**:
  - Respect NR50/NR51 routing (stereo). Default to `0x77/0xFF` and allow caller overrides.
  - For CH3 (wave), on DMG only volumes are 0, 1/2, 1/4; on CGB full volume is available. The library should stick to DMG-safe volumes by default, with an optional `#define SFX_ENABLE_CGB_FULL_CH3_VOL` to unlock full volume on CGB.
- **Performance**: O(1) per active channel per frame. Keep code-size small.
- **Banking**: Prefer placing scripted SFX tables in bank 0 unless the project uses banking. If banking is introduced, add macros for `BANKREF`/`BANKREF_EXTERN`.

---

## 3) Project layout Copilot should create/update

```
/ (repo root)
├─ include/
│  └─ sfx.h
├─ src/
│  ├─ sfx.c
│  ├─ sfx_defs.c
│  └─ sfx_utils.c         // optional: CH3 wave loader helpers
├─ examples/
│  └─ sfx_quickstart/
│     └─ main.c
├─ docs/
│  └─ SFX.md
├─ copilot-instructions.md
└─ Makefile
```

- The **Makefile** should build the example into `build/sfx_quickstart.gb` using the installed GBDK toolchain.

---

## 4) Public API contract (header)

Copilot must **mirror the header contract** from the Page. Do not change names or signatures. Keep enums and structs as specified. Ensure `extern "C"` guards.

---

## 5) Runtime design Copilot should implement

- Maintain one **slot per hardware channel** (CH1–CH4):
  ```c
  typedef struct {
      const sfx_def_t *def;  // NULL if idle
      uint8_t frame_i;       // current frame index
      uint8_t wait;          // VBlanks remaining before next frame
      sfx_priority_t prio;   // priority of the active SFX
  } sfx_slot_t;
  ```
- `sfx_play(id, prio)` behavior:
  1. Look up `sfx_def_t` by id.
  2. Find the first allowed channel (from `preferred_channel` mask) that is **not reserved** by `music_mask` and either **idle** or running a **lower priority** SFX.
  3. If found, **preempt** the lower-priority SFX (if any), load frame 0 for the new SFX, and return `true`. Otherwise return `false`.
- `sfx_update()` behavior per active slot:
  - Decrement `wait`. When it reaches 0, apply the frame’s register writes in order, then advance to the next frame, loading its `vbls` into `wait`. If past the last frame, mark the slot idle.
- **Register writes**: Implement a tiny `sfx_write(uint8_t reg, uint8_t val)` that writes to `0xFF00 + reg`. Allow scripts to include the trigger bit; otherwise OR bit7 for the final control register when starting a note.

---

## 6) Scripted SFX data model

```c
typedef struct { uint8_t reg; uint8_t val; } sfx_write_t; // (NRxx low address, value)

typedef struct {
    uint8_t vbls;            // VBlanks to wait BEFORE applying these writes
    const sfx_write_t *ops;  // pointer to N writes for this frame
    uint8_t count;           // number of writes
} sfx_frame_t;

typedef struct {
    uint8_t preferred_channel; // bitmask (SFX_CH1..SFX_CH4)
    uint8_t priority_hint;     // default priority / tie-breaker
    const sfx_frame_t *frames; // frames array
    uint8_t frame_count;       // number of frames
} sfx_def_t;
```

**Guidelines**:
- Use **NR10..NR14** for CH1, **NR21..NR24** for CH2, **NR30..NR34** for CH3, **NR41..NR44** for CH4.
- Typical effects:
  - **Jump/Bleep** on CH1 or CH2: set duty (NRx1), envelope (NRx2), freq (NRx3/NRx4|0x80).
  - **Laser** on CH2: rapid retriggers lowering pitch across a few frames.
  - **Explosion/Hit** on CH4: NR41, NR42 envelope decay, NR43 polynomial, NR44 trigger.

---

## 7) hUGEDriver integration policy

- **Music reservation**: `music_mask` reserves channels for music (bits of `SFX_CH1..SFX_CH4`). Default `0x00` (no reservation). In examples with hUGEDriver, reserve **CH1/CH2/CH3** for music and let SFX use **CH4** by default. Provide a key to temporarily claim CH2 for a laser demo.
- **Non-invasive**: The SFX library **does not** poke hUGEDriver state. It only avoids reserved channels.
- **Optional ducking**: Add `#define SFX_ENABLE_DUCKING` which temporarily lowers `NR50` during loud SFX and restores after all SFX stop. Keep it off by default.

Example usage pattern in the example ROM:
```c
// During init, when music starts on CH1-CH3:
sfx_set_music_mask(SFX_CH1 | SFX_CH2 | SFX_CH3);
```

---

## 8) Example ROM behavior Copilot should implement

- **Inputs**:
  - **A**: `sfx_play_jump(2)` (CH1 or CH2)
  - **B**: `sfx_play_explosion(3)` (CH4)
  - **SELECT**: `sfx_play_laser(2)` (prefers CH2; if masked, try CH1)
  - **START**: toggle `music_mask` for CH2 to demonstrate preemption vs. reservation
- **Loop**: `wait_vbl_done(); sfx_update();` once per frame.
- **On-screen text** (optional): Show current mask bits and which channels are active.

---

## 9) Makefile guidance

- Expect `GBDK_HOME` to be set (standard for GBDK-2020). Output to `build/`.
- Provide targets:
  - `make all` (default)
  - `make clean`
  - `make run` (optional if the environment provides an emulator; otherwise omit)

---

## 10) Testing expectations

- Manual test on **SameBoy** (CGB mode) and **BGB** for parity.
- Add `#define SFX_TEST_MODE` that redirects register writes into a small ring buffer (addresses & values) so unit tests can checksum sequences in emulator-driven tests.
- Verify CPU cost is bounded and there are no busy waits.

---

## 11) Copilot prompt snippets (paste into Copilot Chat)

- **Create header**
  > Create `include/sfx.h` implementing the API from the repo’s copilot-instructions and Page. Use C89, include enums/structs, and `extern "C"` guards. No dynamic allocation.

- **Implement runtime**
  > Implement `src/sfx.c` per the instructions: per-channel slots, `sfx_write()`, `sfx_init()`, `sfx_update()`, `sfx_play()` with priority and music mask, `sfx_stop_all()`, `sfx_set_music_mask()`, and the three procedural SFX helpers.

- **Define scripted SFX**
  > Add `src/sfx_defs.c` with at least: jump (CH1), laser (CH2 with 3–4 frame downward pitch steps), explosion (CH4), and UI blip (CH2). Use the `sfx_def_t` / `sfx_frame_t` / `sfx_write_t` format.

- **Example ROM**
  > Write `examples/sfx_quickstart/main.c` that initializes the SFX library, sets `music_mask` to reserve CH1–CH3 for hUGEDriver, and maps buttons to SFX. Call `wait_vbl_done()` then `sfx_update()` each frame.

- **Docs**
  > Create `docs/SFX.md` with quick start, tuning tips for DMG vs CGB (CH3 volume), masking with hUGEDriver, and troubleshooting (NR52, NR50/NR51, triggers).

---

## 12) Definition of Done

- Builds with **latest GBDK-2020 4.x**.
- Example ROM runs; pressing A/B/SELECT plays the corresponding SFX while music (via hUGEDriver) continues without conflicts on reserved channels.
- Code passes a simple scripted test (`SFX_TEST_MODE`) in emulator.
- README/docs explain integration, masking, and limitations.

---

## 13) Guardrails for Copilot

- Do **not** import non-GBDK libraries or use POSIX/host APIs.
- Do **not** implement emulation, file I/O, or timing that depends on anything other than VBlank.
- Keep register writes minimal and grouped per frame.
- Keep symbol names and file paths **exactly** as above so the Makefile can reference them.

