CC=g++
CFLAGS=-std=c++11 -Wall -O3
LDFLAGS=-static-libgcc -static-libstdc++ -Wl,-Bstatic -lstdc++ -lpthread -Wl,-Bdynamic

FILES= wavBreaker.cpp

OUTPUT= out/wavBreaker

all:
	if not exist out mkdir out
	copy lame.exe out\lame.exe
	copy ffmpeg.exe out\ffmpeg.exe
	$(CC) $(CFLAGS) $(FILES) -o $(OUTPUT) $(LDFLAGS)

clean:
    del out\*.* /s /q
	rmdir out