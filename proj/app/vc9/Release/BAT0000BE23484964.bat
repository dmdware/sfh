@echo off
copy /y "c:\proj\csd\aaa038\proj\app\vc9\Release\app.exe" "C:\proj\csd\aaa038\proj\app\vc9\../../../testfolder/app.exe"
if errorlevel 1 goto VCReportError
goto VCEnd
:VCReportError
echo Project : error PRJ0019: A tool returned an error code from "Performing Post-Build Event..."
exit 1
:VCEnd