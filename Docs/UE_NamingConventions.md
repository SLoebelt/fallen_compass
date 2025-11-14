# Unreal Engine Naming Conventions (Blueprints-Only)

This guide standardizes names for assets and Blueprint symbols in this project. It follows common UE practices, tuned for Blueprint-only workflows. Keep names short, consistent, and searchable.

## General Rules

- Use PascalCase after a short, type-specific prefix (e.g., `BP_InteractableDoor`).
- No spaces or special chars; use `_` only in prefixes and clear separators.
- One concept per asset; don’t reuse names across types (avoid collisions).
- Prefer descriptive over clever: “Door” > “Obj01”.
- Keep related assets together in folders that match their type.
- Naming: All custom classes use the **FC** prefix (e.g. `UFCGameInstance`, `AFCGameMode`, `AFCFirstPersonCharacter`).

## Folder Structure (Content/)

Hybrid approach: top-level by feature/domain, then inside each folder organize by asset type. Keep shared and third-party content isolated. C++ files go to `/Source/FC`.

Top-level features

```
Content/FC
 ├─ Core/
 │   ├─ Blueprints/
 │   ├─ Data/           # DataTables, Curves, DataAssets, Input
 │   ├─ Persistence/    # Save/Load assets (SG_, BPI_Saveable, etc.)
 │   └─ UI/
 ├─ Player/
 │   ├─ Blueprints/
 │   ├─ Animations/
 │   ├─ Data/
 │   └─ UI/
 ├─ InventorySystem/
 │   ├─ Blueprints/
 │   ├─ Data/
 │   └─ UI/
 ├─ InteractionSystem/
 │   ├─ Blueprints/ # Strategies, Interfaces etc.
 │   ├─ Data/
 │   └─ UI/
 ├─ AI/
 │   ├─ Blueprints/
 │   ├─ StateTrees/     # ST_ assets
 │   ├─ BehaviorTrees/  # BT_/BB_/BTT_/BTD_/BTS_
 │   └─ Data/
 ├─ UI/
 │   ├─ Widgets/
 │   └─ Data/
 ├─ World/
 │   ├─ Blueprints/     # Environment, Galley, Boarding, Events, etc.
 │   ├─ Levels/
 │   ├─ SmartObjects/
 │   ├─ Materials/
 │   ├─ Meshes/
 │   ├─ Animations/
 │   └─ FX/
 ├─ Audio/
 │   ├─ Cues/
 │   └─ Waves/
 ├─ ThirdParty/         # Vendor/imported packs kept separate
 └─ Dev/                # Temporary/WIP (clean before shipping)
```

Notes

- Input (Enhanced Input) can live under `Content/Input/`.
- Shared meshes/materials used across features can live under `Content/World/...` or `Content/Core/Data` as appropriate.
- Keep third-party content under `Content/ThirdParty/...` to avoid mixing with project assets.

## Asset Prefixes (Blueprint-centric)

Blueprint Classes

- BP\_ Blueprint Actor/Actor-based class (e.g., `BP_ServingTrolley`)
- BPC\_ Blueprint Component (Actor Component) (e.g., `BPC_Inventory`)
- BPFL\_ Blueprint Function Library (e.g., `BPFL_SaveHelpers`)
- BPI\_ Blueprint Interface (e.g., `BPI_Saveable`)
- WBP\_ Widget Blueprint (UMG) (e.g., `WBP_HUD`)

Animation

- ABP\_ Animation Blueprint (`ABP_Player`)
- AM\_ Anim Montage (`AM_Collect`)
- AN\_ Anim Sequence (`AN_Walk_Fwd`)
- BS1D* / BS2D* Blend Spaces (`BS1D_LocomotionSpeed`, `BS2D_Locomotion`)
- AO\_ Aim Offset (`AO_Locomotion`)
- SK\_ Skeletal Mesh (`SK_Mannequin`)
- SKEL\_ Skeleton (`SKEL_Mannequin`)
- PHYS\_ Physics Asset (`PHYS_Mannequin`)

Materials & Textures

- M\_ Material (`M_CabinPanel`)
- MI\_ Material Instance (`MI_CabinPanel_Glossy`)
- MF\_ Material Function (`MF_Triplanar`)
- MPC\_ Material Parameter Collection (`MPC_Global`)
- T\_ Texture + map suffixes (`T_Panel_BaseColor`, `T_Panel_N`, `T_Panel_R`, `T_Panel_AO`, `T_Panel_M`)
  - Packed: `_ORM` (Occlusion/Roughness/Metallic) or `_MRA` (Metallic/Roughness/AO)

Meshes

- SM\_ Static Mesh (`SM_Trolley`)
- UCX*/UBX*/USP*/UCP* Collision meshes for SM (DCC naming)

Data & Config

- DT\_ Data Table (`DT_Items`)
- CT\_ Curve Table (`CT_Gameplay`)
- CF*/CV*/CLC\_ Curves (`CF_Recoil`, `CV_Wind`, `CLC_DamageColor`)
- DA\_ Data Asset (`DA_ItemDefs`)
- PM\_ Physical Material (`PM_Metal`)
- INI*/SET* Project Setting assets if needed

Input (Enhanced Input)

- IA\_ Input Action (`IA_Interact`)
- IMC\_ Input Mapping Context (`IMC_Player`)

FX & Cinematics

- NS\_ Niagara System (`NS_Sparks`)
- NE\_ Niagara Emitter (`NE_Sparks_A`)
- NM\_ Niagara Module/Function (`NM_Turbulence`)
- LS\_ Level Sequence (`LS_Intro`)

Audio

- SW\_ Sound Wave (`SW_Click`)
- SC\_ Sound Cue (`SC_UI_Click`)
- MS\_ MetaSound (`MS_Footsteps`)

AI & Logic

- BT\_ Behavior Tree (`BT_Passenger`)
- BB\_ Blackboard (`BB_Passenger`)
- EQS\_ Environment Query (`EQS_SeatSearch`)
- BTT*/BTD*/BTS\_ BT Task/Decorator/Service (`BTT_RequestCoffee`)
- ST\_ StateTree (`ST_Passenger`)

Levels & World

- L\_ Level/Map (`L_AircraftMain`)
- LI\_ Level Instance (`LI_Aisle02`)
- DL\_ Data Layer (`DL_Cabin_Passengers`)

UI & Localization

- WBP\_ Widget Blueprint (`WBP_Inventory`)
- FNT\_ Font (`FNT_Roboto_14`)
- STL\_ Style/DataAsset for UI (`DA_UIStyle` or `STL_Buttons`)

## Blueprint Symbols (names inside Blueprints)

Variables

- Use PascalCase for most variables (`InventorySlots`, `ActiveSlotIndex`).
- Keep `b` prefix for booleans for clarity (`bIsOpen`, `bIsBrewing`).
- Arrays, Maps, Sets: pluralize and name by content (`Passengers`, `SeatClaimsById`).
- ExposeOnSpawn variables: suffix `OnSpawn` when useful.

Functions (Blueprint Functions)

- Verb first, PascalCase (`SaveSession`, `ApplySaveData`, `FindClosestSeat`).
- Pure vs. Impure: include a verb even for pure functions.
- Avoid On/Handle for general functions; reserve `On...` for events/delegates.

Events & Dispatchers

- Event Dispatchers: `OnSomethingHappened` (`OnPassengerCheckedIn`).
- Override events keep engine name; custom events follow dispatcher style.

Enums & Structs

- Enum asset prefix `E_` and type-style name (`E_InteractionType`).
- Enum entries in PascalCase (`Press`, `Hold`, `Tap`), optionally prefixed with the type for clarity.
- Struct asset names may use `F_`/`F` to mirror UE style (`FActorSaveData`, `FInventorySlotSave`).
- Struct fields in PascalCase (`SaveID`, `Transform`, `StackCount`).

Interfaces

- Asset prefix `BPI_` (`BPI_Saveable`).
- Function names state intent, not event framing (`GetSaveID`, `CaptureSaveData`, `ApplySaveData`).

Components

- Asset prefix `BPC_` for reusable component Blueprints.
- Component instances on actors: PascalCase instance names (`Inventory`, `Interaction`).

## Suffixes & Patterns

- LODs/Variants: `Name_LOD1`, `Name_HQ`/`_LQ` if needed.
- Platform/Language variants: `..._Mobile`, `..._DE` (rare; prefer DataAssets/loc).
- Temporary/WIP: prefix with `WIP_` or put in `/Dev` folder; remove before shipping.

## Examples

- `BP_ServingTrolley`, `BPC_Inventory`, `BPI_Saveable`
- `SG_Profile`, `SG_Session`, `FActorSaveData`, `E_InteractionType`
- `IA_Interact`, `IMC_Player`
- `ABP_Player`, `AM_Collect`, `BS2D_Locomotion`, `AO_Locomotion`
- `SM_Trolley`, `SK_Mannequin`, `PHYS_Mannequin`
- `M_CabinPanel`, `MI_CabinPanel_Glossy`, `T_Panel_BaseColor`, `T_Panel_ORM`
- `DT_Items`, `DA_SaveDefaults` (if added later)
- `BT_Passenger`, `BB_Passenger`, `EQS_SeatSearch`, `ST_Passenger`
- `WBP_HUD`, `SC_UI_Click`, `NS_Sparks`, `LS_Intro`, `L_AircraftMain`

## Do/Don’t Quicklist

- Do keep prefixes consistent; they’re your fastest search filter.
- Do use PascalCase and clear verbs/nouns.
- Do prefix booleans with `b`.
- Don’t mix spaces or case styles.
- Don’t overload `BP_` for everything—use `BPC_`, `BPI_`, `BPFL_` where appropriate.
- Don’t rely on folder names to infer type—prefix assets too.
