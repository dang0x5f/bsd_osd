# BSD OSD 

## Remarks

on pause. design flaw slowed progress. :(

either due to amateur programmer error or stb file limitations: static is not restricting fuctions to their respective stb file. as a consequence, generic function names (e.g., init() , draw() , etc.) that would be expected in each feature cannot be defined more than once. 

currently, readability is not easy on the eyes with the stb file name prepended/appended to a function name, or any various workarounds naming schemes (buttonlist, applist, xwmain, etc.). i attempted standard function names using function pointers such as "void (\*init)(void)", but not convinced of the partial implementation so far.

best solution going forward seems to be an extraction of common code used across all files, refactoring, and reverting to traditional .h .c program design. 

## OSD Volume Overlay

```sh
osd -v [-/+]
```

![image](img/osd_volume.gif)

## OSD Select Output Device 

```sh
$ osd -m o
```

![image](img/osd_outputdev.gif)
