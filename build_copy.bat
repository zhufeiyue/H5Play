call uglifyjs viewboxplayerui.js viewboxplayer.js -m -c -o viewboxplayerc.js

call build.bat

del /F viewboxplayerc.js

copy /y main.js  D:\Downloads\nginx-1.12.0\html\videoplayer

copy /y videoflvdash.html D:\Downloads\nginx-1.12.0\html\videoplayer