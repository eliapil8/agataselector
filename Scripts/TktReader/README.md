Load script in ROOT
```
.L tktToRoot.cxx
```
and get either single histogram, e.g.: 
```
auto *histo = getHistogram("Prod__4-38-32768-UI__Ampli.spec", {1,1})
```
or convert entire `.spec` file into a ROOT file with possible recalibration using offset and gain using
```
saveTktToRoot("Prod__4-38-32768-UI__Ampli.spec",0,0.2)
```