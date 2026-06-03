# CTR Overlay Architecture

CTR has **13 overlay binaries** organized into **3 overlay regions** (memory zones
that get swapped at runtime). The main executable (`exe`, 953 functions) stays
resident in RAM at all times. Overlay regions are fixed address ranges that get
overwritten from CD as the game transitions between screens.

## Region 1: End-of-Race UI (`OVR_Region1`) — Overlays 221-225

Loaded by `LOAD_OvrEndRace(param)` depending on game mode. Only one is resident
at a time.

| Overlay | Purpose | Functions |
|---------|---------|-----------|
| **221** | Crystal Challenge end-of-race | 1 (`CC_EndEvent_DrawMenu`) |
| **222** | Adventure Arena end-of-race | 2 (`AA_EndEvent_*`) |
| **223** | Relic Race end-of-race | 3 (`RR_EndEvent_*`) |
| **224** | Time Trial end-of-race | 3 (`TT_EndEvent_*`) |
| **225** | VS/Battle end-of-race | 1 (`VB_EndEvent_DrawMenu`) |

## Region 2: Level-of-Detail Renderer (`OVR_Region2`) — Overlays 226-229

Loaded by `LOAD_OvrLOD(numPlayers)` based on split-screen count (1P/2P/3P/4P).
Contains the quadblock geometry renderer and scratchpad-heavy rendering pipeline.

| Overlay | Purpose | Functions |
|---------|---------|-----------|
| **226** | 1P quadblock renderer | `DrawLevelOvr1P` |
| **227** | 2P quadblock renderer | `DrawLevelOvr2P` |
| **228** | 3P quadblock renderer | `DrawLevelOvr3P` |
| **229** | 4P quadblock renderer | `DrawLevelOvr4P` |

## Region 3: Main Menu + Gameplay (`OVR_Region3`) — Overlays 230-233

The large persistent overlays, each with its own load callback
(`LOAD_Callback_Overlay_230`..`233`).

| Overlay | Purpose | Functions |
|---------|---------|-----------|
| **230** | Main Menu (titles, menus, cheats, track select, video) | 86 |
| **231** | Racing gameplay (weapons, hazards, items, bots, pickups) | 130 |
| **232** | Adventure Hub (warppads, garages, doors, maps, masks) | 37 |
| **233** | Cutscenes, podium, credits, end-of-file stubs | 51 |

## How Overlays Load

All three regions use the same basic mechanism:

1. `LOAD_AppendQueue(0, LT_SETADDR, bigfileIndex, &OVR_RegionN, callback)`
   queues a CD read that streams the overlay binary into the region's fixed
   address range.
2. The callback (`LOAD_Callback_Overlay_Generic` for regions 1 and 2, or the
   dedicated `LOAD_Callback_Overlay_230`..`233` for region 3) fires when the
   read completes and clears `sdata->load_inProgress`.
3. `GameTracker` tracks which overlay is currently resident per region via
   `overlayIndex_EndOfRace`, `overlayIndex_LOD`, and `overlayIndex_Threads`.

Key constraint: **only one overlay per region** can be loaded at a time. When
the game swaps e.g. from 1P to 2P mode, it overwrites overlay 226 with 227 in
the same RAM addresses.

## Native Build Behavior

In ctr-native, the `#ifndef CTR_NATIVE` guards around `LOAD_AppendQueue` calls
in `LOAD_OvrEndRace` and `LOAD_OvrLOD` skip runtime overlay streaming. Native
has a host-file `Cd*` facade for asset loads, but overlay C code is compiled
directly into the native binary, so every overlay function is always present
regardless of which region it logically belongs to.
