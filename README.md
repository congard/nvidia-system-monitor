# nvidia-system-monitor
<center>
    <img src="icon.png" alt="icon" width="256" height="256"/>
</center>

Task Manager for Linux for Nvidia graphics cards

I wrote this program quickly enough, because it was urgently needed. All other functions (temperature, language, etc.) will be added in next versions

# Dependencies
wxWidgets is required for work. You must install (if using ArchLinux) the `wxgtk` and `wxgtk3` packages

# Building
Execute:
 1. `mkdir bin`
 2. `chmod +x make.sh`
 3. `./make.sh`
 4. `./bin/nvsm` (or if you want to launch on discrete GPU launch with `optirun`)

# Installing
Currently the installation script is not written yet. But soon it will be written, and it will also be possible to install this package from AUR

# Config
Here example of simple config located in `~/.config/nvidia-system-monitor/config`:
```
# time in ms
updateDelay 500
graphLength 120000

#           gpu id  red  green  blue
gpuColor    0       0    0      255
gpuColor    1       0    255    0
gpuColor    2       255  0      0
```

# Screenshots
[Open SCREENSHOTS.md](SCREENSHOTS.md)

# Donate
[Open DONATE.md](DONATE.md)