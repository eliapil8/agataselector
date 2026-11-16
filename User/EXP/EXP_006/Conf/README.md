## Contents

| Folder      | Content description                                                       |                       
|-------------|---------------------------------------------------------------------------|
| CUT         | Cuts (TGraphs) needed for the analysis. Each detector has its own folder. |
| EnergyLoss  | Energy loss tables generated in SRIM                                      |
| EXP         | Selector configuration files of old experiments                           |
| LUT         | Lookup table configuration files for various experiments                  |
| NuclearData | Nuclear masses files                                                      |
| Optimizer   | Files to perform the optimization                                         |

## About configuration files

The default configuration file can be generated with

```
./RunSelector --print_conf my_conf.conf
```

Please note that old configuration files are no longer compatible with the updated version of the selector.
