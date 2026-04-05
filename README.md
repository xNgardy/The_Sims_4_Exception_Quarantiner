WORK-IN-PROGRESS

# Sims 4 Mod Quarantine

A small Windows desktop tool that helps you isolate (quarantine) likely-problematic Sims 4 mods by reading an exception report and moving matching `.package` / `.ts4script` files out of your Mods folder.

> Not affiliated with EA / Maxis. Use at your own risk — always keep backups.

## What it does
- You select:
  1) Your Sims 4 **Mods** folder
  2) An **exception report** file (`.txt` or `.html`)
- The app scans the exception report for filenames ending in:
  - `.package`
  - `.ts4script`
- It searches your Mods folder (including subfolders) for those exact filenames and **moves** matches into a quarantine folder.

## Where quarantined files go
The tool creates a folder named `Quarantine_Mods` **next to** your Mods folder (one directory above it), then moves detected files there.

Example:
- Mods folder: `.../The Sims 4/Mods`
- Quarantine folder: `.../The Sims 4/Quarantine_Mods`

## How to use
1. Run the app.
2. Click “Browse Mods Folder” and select your Sims 4 Mods directory.
3. Click “Browse Exception File” and select the exception report (`.txt` or `.html`).
4. Click “RUN QUARANTINE”.
5. Launch the game and see if the issue is resolved; re-add mods from quarantine gradually if needed.

## Notes / limitations
- The tool matches *filenames* mentioned in the exception report. If the report doesn’t contain a clear filename, nothing may be quarantined.
- If multiple mods share the same filename in different folders, only one can be moved into the single quarantine folder (Windows filename collisions).
- Moving files is a real change to your Mods folder. Consider backing up before running.

## Build (Windows)
### Requirements
- Visual Studio 2022 (MSVC)
- CMake 3.16+
- Qt 6 (MSVC build, e.g. `msvc2022_64`)

### Configure + build (CMake)
From the project root:

```powershell
cmake -S . -B build -DCMAKE_PREFIX_PATH="C:/Qt/6.11.0/msvc2022_64"
cmake --build build --config Debug
