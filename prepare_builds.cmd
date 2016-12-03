rem %comspec% /k 
@call "c:\Program Files (x86)\Microsoft Visual Studio 11.0\VC\bin\vcvars32.bat"
REM @call ball_debug.cmd
REM @set BUILD_STATUS=%ERRORLEVEL% 
REM @if not %BUILD_STATUS%==0 goto fail
REM @timeout /T 5 /NOBREAK 
@call ball_release.cmd
@set BUILD_STATUS=%ERRORLEVEL% 
@if not %BUILD_STATUS%==0 goto fail
@timeout /T 5 /NOBREAK 
@set @zipper="D:\Program Files\7-Zip\7z.exe"
@del /F /Q d:\Temp\bin_Release.7z
@call %@zipper% a d:\Temp\bin_Release.7z y:\bin_Release\ -xr!*.ilk  
@del /F /Q d:\Temp\bin_Debug.7z
@call %@zipper% a d:\Temp\bin_Debug.7z y:\bin_Debug\ -xr!*.ilk  
@timeout /T 5 /NOBREAK 

@echo user winsor> ftpcmd.dat
@echo ckj;ysq gfhjkm>> ftpcmd.dat
@echo bin>> ftpcmd.dat
@echo cd ~/public_html>> ftpcmd.dat
@echo del bin_Release.7z>> ftpcmd.dat
@echo del bin_Debug.7z>> ftpcmd.dat
@echo put d:\Temp\bin_Release.7z>> ftpcmd.dat
@echo put d:\Temp\bin_Debug.7z>> ftpcmd.dat
@echo by>> ftpcmd.dat
@ftp -n -s:ftpcmd.dat fss.ukrhub.net
@del ftpcmd.dat
@goto end


:fail
exit /b 1

:end
@timeout /T 10 /NOBREAK 
exit /b 0
