# wavBreaker
Break a WAV file into separate mp3 with customized creation date/time and fixed bitrate.

Notes:
- Fixed bitrate is the best for compatibility with old mp3 players.
- The -h parameter change the creation/last acessed time of generated files. This is extremely useful for mp3 players, since they order the files by creation date/time.
- I created this to split long deep house sessions.

Usage:
wavBreaker -i interval_minutes -b bitrate [-h hour_creation] -o filename file.wav

Compiles only on Windows, using mingw-w64 (https://mingw-w64.org).

Dependencies:
- lame.exe (http://www.rarewares.org/mp3-lame-bundle.php)
- ffmpeg.exe (http://ffmpeg.zeranoe.com/builds/)

Before:

![Before](/before.PNG?raw=true)

After:

![After](/after.PNG?raw=true)