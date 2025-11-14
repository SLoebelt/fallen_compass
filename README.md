# Fallen Compass

Fallen Compass is an expedition-focused single-player game prototype built with Unreal Engine 5.7.

Short description

- You lead and outfit expeditions and guide a 3D top-down overworld convoy across uncharted regions. Gameplay mixes meta-level planning, resource management, and tactical, grid-based combat.

Engine

- Unreal Engine: 5.7

Branching strategy (recommended)

- `master` - stable/main branch for released or milestone-ready code. Release branch after pase 2 (Demo Release).
- `develop` - used for development after Demo is released.
- `feature/<name>` - short-lived feature branches for specific tasks or features (e.g. `feature/office-camera`).
- `hotfix/<name>` - emergency fixes applied to `master` and merged back into active branches.
- `release<version>` - short lived release branch to build and deploy release version.

Basic workflow

1. Create a branch: `git checkout -b feature/<name>`
2. Work, commit frequently with clear messages (e.g. `feat: add office table interaction - task ref`).
3. Merge to `master`/`develop` once reviewed and CI/QA checks pass.

Notes

- See `/Docs/Fallen_Compass_GDD_v0.2.md` for design context and `/Docs/UE_NamingConventions.md` for code & asset naming rules.
