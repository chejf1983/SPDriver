del /F /A /Q .\sdk\*.*
xcopy SpectraArsenalForGCS_V5.02\lib\*.* .\sdk
copy SpectraArsenalForGCS_V5.02\Release\*.dll .\sdk
copy SpectraArsenalForGCS_V5.02\Release\*.lib .\sdk
copy SpectraArsenalForGCS_V5.02\SpectraArsenal.h .\sdk


copy .\sdk\*.dll windemon\Release\
xcopy /y /c /h /r .\sdk\*.h windemon\
xcopy /y /c /h /r .\sdk\*.lib windemon\
pause