# PrimaryMonitorSwapper
A simple Windows command line utility to quickly swap which monitor is the primary one.
Mostly made for applications like [Source Engine](https://developer.valvesoftware.com/wiki/Source) games that dislike being on any monitor other than the primary one.

## Install Instructions
If running on 64 bit Windows x86 architecture, you can download the latest [release](https://github.com/Jetinat0r/PrimaryMonitorSwapper/releases/latest)

## Build Instructions
- Ensure Visual Studio & C++ is installed
- Open the project in Visual Studio (2022+) and build in `Release` mode

The project is 1 cpp file, so it probably isn't difficult to build without VS, if desired

## Usage
- `pm <monitorId>` to set the monitor corresponding to monitorId as the primary monitor
- `pm -l` to list available monitors and their Ids
- `pm -n` to list the number of available monitors (does not necessarily correspond to a valid id range)
- `pm -N` to list the number of monitors (available or not)

## Suggestion
Add the `bin` folder containing the executable to your `PATH` so it can be invoked from anywhere.

## Notes
- `pm.exe` is copied to `pm` on build for convenience with Unix shells (like Git Bash)
