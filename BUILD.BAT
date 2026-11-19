rem Microsoft C 6.0a: compiler and generated program are both 16-bit DOS.
rem Configure PATH, INCLUDE, and LIB for the MSC 6.0a DOS installation.
rem /AL = Large model; /G0 = 8086/8088; /Ze enables _far; /Lr selects real-mode CRT.
rem No DOS extender, DPMI host, or 32-bit compiler component is used.
if exist ENC.EXE del ENC.EXE
if exist ENC.MAP del ENC.MAP
if exist ENC.LNK del ENC.LNK
if exist *.OBJ del *.OBJ
echo Compiling with Microsoft C 6.0a (16-bit DOS)...
CL /AL /G0 /Od /Ze /Lr /c /DET_DOS16 /FoMAIN.OBJ MAIN.C
if errorlevel 1 goto failed
CL /AL /G0 /Od /Ze /Lr /c /DET_DOS16 /FoFILEIO.OBJ FILEIO.C
if errorlevel 1 goto failed
CL /AL /G0 /Od /Ze /Lr /c /DET_DOS16 /FoERROR.OBJ ERROR.C
if errorlevel 1 goto failed
CL /AL /G0 /Od /Ze /Lr /c /DET_DOS16 /FoCONVERT.OBJ CONVERT.C
if errorlevel 1 goto failed
CL /AL /G0 /Od /Ze /Lr /c /DET_DOS16 /FoRECOVERY.OBJ RECOVERY.C
if errorlevel 1 goto failed
CL /AL /G0 /Od /Ze /Lr /c /DET_DOS16 /FoREGISTRY.OBJ REGISTRY.C
if errorlevel 1 goto failed
CL /AL /G0 /Od /Ze /Lr /c /DET_DOS16 /FoTEST.OBJ TEST.C
if errorlevel 1 goto failed
CL /AL /G0 /Od /Ze /Lr /c /DET_DOS16 /FoTABLES.OBJ TABLES\TABLES.C
if errorlevel 1 goto failed
CL /AL /G0 /Od /Ze /Lr /c /DET_DOS16 /FoFWD00.OBJ TABLES\FWD00.C
if errorlevel 1 goto failed
CL /AL /G0 /Od /Ze /Lr /c /DET_DOS16 /FoFWD01.OBJ TABLES\FWD01.C
if errorlevel 1 goto failed
CL /AL /G0 /Od /Ze /Lr /c /DET_DOS16 /FoFWD02.OBJ TABLES\FWD02.C
if errorlevel 1 goto failed
CL /AL /G0 /Od /Ze /Lr /c /DET_DOS16 /FoFWD03.OBJ TABLES\FWD03.C
if errorlevel 1 goto failed
CL /AL /G0 /Od /Ze /Lr /c /DET_DOS16 /FoFWD04.OBJ TABLES\FWD04.C
if errorlevel 1 goto failed
CL /AL /G0 /Od /Ze /Lr /c /DET_DOS16 /FoFWD05.OBJ TABLES\FWD05.C
if errorlevel 1 goto failed
CL /AL /G0 /Od /Ze /Lr /c /DET_DOS16 /FoFWD06.OBJ TABLES\FWD06.C
if errorlevel 1 goto failed
CL /AL /G0 /Od /Ze /Lr /c /DET_DOS16 /FoFWD07.OBJ TABLES\FWD07.C
if errorlevel 1 goto failed
CL /AL /G0 /Od /Ze /Lr /c /DET_DOS16 /FoFWD08.OBJ TABLES\FWD08.C
if errorlevel 1 goto failed
CL /AL /G0 /Od /Ze /Lr /c /DET_DOS16 /FoFWD09.OBJ TABLES\FWD09.C
if errorlevel 1 goto failed
CL /AL /G0 /Od /Ze /Lr /c /DET_DOS16 /FoFWD10.OBJ TABLES\FWD10.C
if errorlevel 1 goto failed
CL /AL /G0 /Od /Ze /Lr /c /DET_DOS16 /FoFWD11.OBJ TABLES\FWD11.C
if errorlevel 1 goto failed
CL /AL /G0 /Od /Ze /Lr /c /DET_DOS16 /FoFWD12.OBJ TABLES\FWD12.C
if errorlevel 1 goto failed
CL /AL /G0 /Od /Ze /Lr /c /DET_DOS16 /FoFWD13.OBJ TABLES\FWD13.C
if errorlevel 1 goto failed
CL /AL /G0 /Od /Ze /Lr /c /DET_DOS16 /FoFWD14.OBJ TABLES\FWD14.C
if errorlevel 1 goto failed
CL /AL /G0 /Od /Ze /Lr /c /DET_DOS16 /FoFWD15.OBJ TABLES\FWD15.C
if errorlevel 1 goto failed
CL /AL /G0 /Od /Ze /Lr /c /DET_DOS16 /FoFWD16.OBJ TABLES\FWD16.C
if errorlevel 1 goto failed
CL /AL /G0 /Od /Ze /Lr /c /DET_DOS16 /FoREV00.OBJ TABLES\REV00.C
if errorlevel 1 goto failed
CL /AL /G0 /Od /Ze /Lr /c /DET_DOS16 /FoREV01.OBJ TABLES\REV01.C
if errorlevel 1 goto failed
CL /AL /G0 /Od /Ze /Lr /c /DET_DOS16 /FoREV02.OBJ TABLES\REV02.C
if errorlevel 1 goto failed
CL /AL /G0 /Od /Ze /Lr /c /DET_DOS16 /FoREV03.OBJ TABLES\REV03.C
if errorlevel 1 goto failed
CL /AL /G0 /Od /Ze /Lr /c /DET_DOS16 /FoREV04.OBJ TABLES\REV04.C
if errorlevel 1 goto failed
CL /AL /G0 /Od /Ze /Lr /c /DET_DOS16 /FoREV05.OBJ TABLES\REV05.C
if errorlevel 1 goto failed
CL /AL /G0 /Od /Ze /Lr /c /DET_DOS16 /FoREV06.OBJ TABLES\REV06.C
if errorlevel 1 goto failed
CL /AL /G0 /Od /Ze /Lr /c /DET_DOS16 /FoREV07.OBJ TABLES\REV07.C
if errorlevel 1 goto failed
CL /AL /G0 /Od /Ze /Lr /c /DET_DOS16 /FoREV08.OBJ TABLES\REV08.C
if errorlevel 1 goto failed
CL /AL /G0 /Od /Ze /Lr /c /DET_DOS16 /FoREV09.OBJ TABLES\REV09.C
if errorlevel 1 goto failed
CL /AL /G0 /Od /Ze /Lr /c /DET_DOS16 /FoREV10.OBJ TABLES\REV10.C
if errorlevel 1 goto failed
CL /AL /G0 /Od /Ze /Lr /c /DET_DOS16 /FoREV11.OBJ TABLES\REV11.C
if errorlevel 1 goto failed
CL /AL /G0 /Od /Ze /Lr /c /DET_DOS16 /FoREV12.OBJ TABLES\REV12.C
if errorlevel 1 goto failed
CL /AL /G0 /Od /Ze /Lr /c /DET_DOS16 /FoREV13.OBJ TABLES\REV13.C
if errorlevel 1 goto failed
CL /AL /G0 /Od /Ze /Lr /c /DET_DOS16 /FoREV14.OBJ TABLES\REV14.C
if errorlevel 1 goto failed
CL /AL /G0 /Od /Ze /Lr /c /DET_DOS16 /FoREV15.OBJ TABLES\REV15.C
if errorlevel 1 goto failed
CL /AL /G0 /Od /Ze /Lr /c /DET_DOS16 /FoREV16.OBJ TABLES\REV16.C
if errorlevel 1 goto failed
CL /AL /G0 /Od /Ze /Lr /c /DET_DOS16 /FoFROMU8.OBJ FROM\UTF8.C
if errorlevel 1 goto failed
CL /AL /G0 /Od /Ze /Lr /c /DET_DOS16 /FoFROMU16.OBJ FROM\UTF16.C
if errorlevel 1 goto failed
CL /AL /G0 /Od /Ze /Lr /c /DET_DOS16 /FoFROM949.OBJ FROM\CP949.C
if errorlevel 1 goto failed
CL /AL /G0 /Od /Ze /Lr /c /DET_DOS16 /FoFROM437.OBJ FROM\CP437.C
if errorlevel 1 goto failed
CL /AL /G0 /Od /Ze /Lr /c /DET_DOS16 /FoFROM1252.OBJ FROM\CP1252.C
if errorlevel 1 goto failed
CL /AL /G0 /Od /Ze /Lr /c /DET_DOS16 /FoTOU8.OBJ TO\UTF8.C
if errorlevel 1 goto failed
CL /AL /G0 /Od /Ze /Lr /c /DET_DOS16 /FoTOU16.OBJ TO\UTF16.C
if errorlevel 1 goto failed
CL /AL /G0 /Od /Ze /Lr /c /DET_DOS16 /FoTO949.OBJ TO\CP949.C
if errorlevel 1 goto failed
CL /AL /G0 /Od /Ze /Lr /c /DET_DOS16 /FoTO437.OBJ TO\CP437.C
if errorlevel 1 goto failed
CL /AL /G0 /Od /Ze /Lr /c /DET_DOS16 /FoTO1252.OBJ TO\CP1252.C
if errorlevel 1 goto failed
echo Linking 16-bit real-mode ENC.EXE...
echo MAIN+FILEIO+ERROR+CONVERT+RECOVERY+REGISTRY+TEST+TABLES+FWD00+>ENC.LNK
echo FWD01+FWD02+FWD03+FWD04+FWD05+FWD06+FWD07+FWD08+FWD09+FWD10+FWD11+>>ENC.LNK
echo FWD12+FWD13+FWD14+FWD15+FWD16+REV00+REV01+REV02+REV03+REV04+REV05+>>ENC.LNK
echo REV06+REV07+REV08+REV09+REV10+REV11+REV12+REV13+REV14+REV15+REV16+>>ENC.LNK
echo FROMU8+FROMU16+FROM949+FROM437+FROM1252+TOU8+TOU16+TO949+TO437+>>ENC.LNK
echo TO1252>>ENC.LNK
echo ENC.EXE>>ENC.LNK
echo ENC.MAP>>ENC.LNK
echo.>>ENC.LNK
echo ;>>ENC.LNK
LINK /CO /MAP @ENC.LNK
if errorlevel 1 goto failed
if not exist ENC.EXE goto failed
echo Running self-test in DOS...
ENC.EXE /test
if errorlevel 1 goto failed
echo Microsoft C 6.0a 16-bit DOS build and self-test completed.
goto end
:failed
echo DOS build or self-test failed.
exit 1
:end
if exist ENC.LNK del ENC.LNK
