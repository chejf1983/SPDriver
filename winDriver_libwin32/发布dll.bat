del /F /A /Q .\libx32\lib\*.*
xcopy SpectraArsenalForGCS_V5.02\lib\*.* .\libx32\lib
copy SpectraArsenalForGCS_V5.02\Release\SpectraArsenal.dll .\libx32\lib
copy SpectraArsenalForGCS_V5.02\Release\SpectraArsenal.lib .\libx32\lib
copy SpectraArsenalForGCS_V5.02\SpectraArsenal.h .\libx32

copy .\libx32\lib\*.dll windemon\Release\
xcopy /y /c /h /r .\libx32\lib\*.h windemon\
xcopy /y /c /h /r .\libx32\lib\*.lib windemon\

del /F /A /Q .\libx64\lib\*.*
xcopy SpectraArsenal64\lib\*.* .\libx64\lib
copy SpectraArsenal64\x64\Release\SpectraArsenal.dll .\libx64\lib
copy SpectraArsenal64\x64\Release\SpectraArsenal.lib .\libx64\lib
copy SpectraArsenal64\SpectraArsenal\SpectraArsenal.h .\libx64

copy .\libx64\lib\*.dll 64demon1\64demon1\
xcopy /y /c /h /r .\libx64\lib\*.h 64demon1\64demon1\
xcopy /y /c /h /r .\libx64\lib\*.lib 64demon1\64demon1\
pause