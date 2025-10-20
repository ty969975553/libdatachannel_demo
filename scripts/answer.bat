
pushd %~dp0%   
cd ..\out\build\x64-Debug
echo "%CD%"
native_app --role answer --signal-dir ./signals
popd