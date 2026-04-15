WORK-IN-PROGRESS

# Sims 4 Mod Quarantine

A small Qt desktop app for The Sims 4 that reads exception reports, finds likely-problematic mods, and moves suspected files into a quarantine folder.

It is designed for cases where a Better Exceptions report or another crash/exception file points to a mod, but finding that mod manually is annoying because filenames and folders do not always match exactly.

> Not affiliated with EA / Maxis. Use at your own risk and keep backups of your Mods folder.

## Features
- Select your Sims 4 `Mods` folder
- Select or drag-and-drop an exception file (`.txt` or `.html`)
- Detect exact mod filenames like `.package` and `.ts4script`
- Detect Better Exceptions style folder clues like `.\WickedWhims_v187.16\...`
- Match custom folder names such as `Wicked Whims` against clue names such as `WickedWhims_v187.16`
- Move suspected files into `Mods/Quarantine_Mods`
- Preserve relative folder structure inside quarantine
- Preview changes with a dry run before moving anything
- Restore quarantined files back into the `Mods` folder

## How It Works
The app analyzes the selected exception file and looks for two kinds of clues:

1. Exact filenames
   Example: `SomeMod.package` or `ExampleMod.ts4script`
2. Mod folder hints
   Example: `.\WickedWhims_v187.16\turbolib2\events\interactions.py`

It then scans your `Mods` directory, finds matching files or likely matching mod folders, and moves the suspected files into a quarantine area.

## Quarantine Location
The app creates this folder inside your Mods directory:

```text
.../The Sims 4/Mods/Quarantine_Mods
```

If it detects a mod folder such as:

```text
.../The Sims 4/Mods/Wicked Whims
```

it will move files into:

```text
.../The Sims 4/Mods/Quarantine_Mods/Wicked Whims/...
```

This keeps the original folder structure so restoring files later is easier.

## How To Use
1. Launch the app.
2. Choose your Sims 4 `Mods` folder.
3. Choose an exception file, or drag and drop one onto the window.
4. Leave `Preview only (dry run, do not move files)` checked if you want to inspect the result first.
5. Click `Analyze And Quarantine`.
6. Review the popup summary.
7. If you used dry run first, uncheck preview mode and run again to actually move files.
8. If needed, click `Restore Quarantined Files` to move files back.

## Typical Workflow
1. Run a dry run first.
2. Check the detected mod clues and found locations.
3. Run the real quarantine.
4. Launch the game and test.
5. Restore files later if needed.

## Notes And Limitations
- The tool can only work with clues present in the exception report.
- Some reports contain exact filenames, while others only contain folder or script-path hints.
- If a mod cannot be identified from the report, the app will not guess randomly.
- Files already inside `Quarantine_Mods` are skipped.
- If a destination file already exists during quarantine or restore, that file is skipped.
- Moving files is a real filesystem change, so backups are strongly recommended.

## Project Structure
```text
Sims-4-Exception-Quarantiner/
├─ README.md
└─ SimsModFixer/
   ├─ CMakeLists.txt
   └─ src/
      ├─ main.cpp
      ├─ mainwindow.cpp
      └─ mainwindow.h
```

## Build Requirements
- CMake 3.16 or newer
- Qt 6

## Build On Windows
Requirements:
- Visual Studio 2022 with MSVC
- Qt 6 MSVC build, for example `msvc2022_64`

Configure and build:

```powershell
cmake -S SimsModFixer -B SimsModFixer/build_fresh -DCMAKE_PREFIX_PATH="C:/Qt/6.11.0/msvc2022_64"
cmake --build SimsModFixer/build_fresh --config Debug
```

Run:

```powershell
SimsModFixer/build_fresh/Debug/SimsModFixer.exe
```

## Build On macOS
Requirements:
- Xcode Command Line Tools or Xcode
- Qt 6 for macOS

Configure and build:

```bash
cmake -S SimsModFixer -B SimsModFixer/build_macos -DCMAKE_PREFIX_PATH="$HOME/Qt/6.x.x/macos"
cmake --build SimsModFixer/build_macos --config Debug
```

Run:

```bash
open SimsModFixer/build_macos/Debug/SimsModFixer.app
```

## Pushing To GitHub
If you want to publish the project, a simple flow is:

```powershell
git status
git add README.md SimsModFixer
git commit -m "Improve quarantine workflow and documentation"
git push
```

## Future Ideas
- Export scan results to a text file
- Add a clickable list of detected files before moving
- Support more Sims 4 report formats
- Add packaged releases for Windows and macOS
