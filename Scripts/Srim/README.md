# Energy loss table production using SRIM

```ELoss_table_for_AgataSelector.bat``` is windows script, which uses module of ```SRIM``` called ```SRModule.exe``` to produce tables of energy loss tables used by AgataSelector

### Running the script

Locate your installation of ```SRIM``` and its subdirectory ```SR Module```, which you can copy wherever you want to run it

Make sure that files ```SNUC03.dat``` and ```SCOEF03.dat``` are present and named accordingly

Copy files ```ELoss_table_for_AgataSelector.bat``` and ```target.dat``` into your ```SR Module``` folder

Edit ```target.dat``` file with parameters of your target in the form:
A   Z   name   Density(g/cm3)

Open ```cmd``` (press ```win+R```, type ```cmd``` and press enter) and navigate to your ```SR Module``` folder by ```cd /d X:\path\to\your\folder```

Run command ```ELoss_table_for_AgataSelector.bat Amin Amax Zmin Zmax Emax```,
where from Amin to Amax is mass number range of impinging ions,
where from Zmin to Zmax is proton number range of impinging ions
and Emax is maximum energy in keV of impinging ion to be calculated.

The output are .txt files named as Target-A-Z_Ion-A-Z.txt for each ion - target combination. This files needs to be transferred to ```agataselector/User/EXP/Your_EXP_NAME/Conf/EnergyLoss``` folder and you are good to go