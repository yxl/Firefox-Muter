set XPI_NAME=muter.xpi 
cd ..
del /f/q %XPI_NAME%
cd extension
..\tools\7za a ..\%XPI_NAME% chrome\
..\tools\7za a ..\%XPI_NAME% components\
..\tools\7za a ..\%XPI_NAME% defaults\
..\tools\7za a ..\%XPI_NAME% plugins\npiemode.dll
..\tools\7za a ..\%XPI_NAME% modules\*.jsm
..\tools\7za a ..\%XPI_NAME% modules\ctypes-binary\*.dll
..\tools\7za a ..\%XPI_NAME% modules\ctypes-binary\*.so
..\tools\7za a ..\%XPI_NAME% chrome.manifest
..\tools\7za a ..\%XPI_NAME% install.rdf