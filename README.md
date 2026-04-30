# PrimaryMonitorSwapper
A simple command line utility to quickly swap which monitor is the primary one.
Mostly made for applications like Source games that dislike being on any monitor other than the primary one.

## Build Instructions
- Ensure Visual C++ is installed
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
- `pm.exe` is copied to `pm` on build for convenience with linux-like shells (like Git Bash)