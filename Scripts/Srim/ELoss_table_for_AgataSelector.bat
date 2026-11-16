@echo off 

set beginA=%1
set endA=%2
set beginZ=%3
set endZ=%4
set maxE=%5
 
for /F "tokens=1,2,3,4" %%i in ('findstr /v /c:"#" target.dat') do (
   call :SetTarget 1 2 3 4 5 %%i %%j %%k %%l
)
exit /b

:SetTarget
set targetA=%6
set targetZ=%7
set targetName=%8
set targetDens=%9
if %targetA% gtr 0 (
   for /l %%x in (%beginA%, 1, %endA%) do (
      for /l %%y in (%beginZ%, 1, %endZ%) do (
         echo Target: %targetA% %targetZ% %targetName% %targetDens% ; Ion: %%x  %%y
         echo ---Stopping/Range Input Data ^(Number-format: Period = Decimal Point^) > SR.IN
         echo ---Output File Name 	>> SR.IN
         echo "Target-%targetA%-%targetZ%_Ion-%%x-%%y.txt" 	>> SR.IN
	     echo ---Ion ^(Z^), Ion Mass ^(u^)  	>> SR.IN
         echo %%y   %%x 	>> SR.IN
         echo ---Target Data: ^(Solid=0,Gas=1^), Density^(g/cm3^), Compound Corr. 	>> SR.IN
         echo 0    %targetDens%    1 	>> SR.IN
         echo ---Number of Target Elements 	>> SR.IN
         echo  1  	>> SR.IN
         echo ---Target Elements: ^(Z^), Target name, Stoich, Target Mass^(u^) 	>> SR.IN
         echo %targetZ%    %targetName%    1    %targetA%	>> SR.IN
         echo ---Output Stopping Units ^(1-8^) 	>> SR.IN
         echo  3 	>> SR.IN
         echo ---Ion Energy : E-Min^(keV^), E-Max^(keV^) 	>> SR.IN
         echo  10    %maxE% 		>> SR.IN
         timeout 0
         start "" SRModule.exe
         timeout 1
      )
   )
)

exit /b 