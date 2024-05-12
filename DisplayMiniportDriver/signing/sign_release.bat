copy /b /y ..\x64\Release\DisplayMiniportDriver\DisplayMiniportDriver.sys build
copy /b /y ..\x64\Release\DisplayMiniportDriver\DisplayMiniportDriver.inf build
copy /b /y ..\x64\Release\DisplayMiniportDriver\DisplayMiniportDriver.cat build
copy /b /y ..\x64\Release\DisplayMiniportDriver.pdb build
"C:\Program Files (x86)\Windows Kits\10\bin\10.0.22000.0\x64\signtool.exe" sign /a /i "GlobalSign" /tr http://timestamp.globalsign.com/tsa/r6advanced1 /fd SHA256 /td SHA256 build\DisplayMiniportDriver.sys
"C:\Windows\System32\makecab.exe" /f DisplayMiniportDriver.ddf
"C:\Program Files (x86)\Windows Kits\10\bin\10.0.22000.0\x64\signtool.exe" sign /a /i "GlobalSign" /tr http://timestamp.globalsign.com/tsa/r6advanced1 /fd SHA256 /td SHA256 disk1\DisplayMiniportDriver.cab
pause
