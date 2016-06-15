msbuild xr_3da/xr_3da.sln /t:xrRender_R1  /p:configuration="debug" /fileLoggerParameters:LogFile=MyLog.log /verbosity:minimal /p:PreferredToolArchitecture=x64 
msbuild xr_3da/xr_3da.sln /t:xrRender_R2  /p:configuration="debug" /fileLoggerParameters:LogFile=MyLog.log /verbosity:minimal /p:PreferredToolArchitecture=x64 
