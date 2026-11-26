CC      = C:/TDM-GCC-64/bin/gcc.exe
IFLAGS  = -IC:/SDL2/include/SDL2 -IC:/SDL2_image/include/SDL2
LFLAGS  = -LC:/SDL2/lib -LC:/SDL2_image/lib -lmingw32 -lSDL2main -lSDL2_image -lSDL2
OFLAGS  = -O2
SRC     = main.c processing.c rotate.c detection.c

dirs:
	@if not exist output mkdir output
	@if not exist output\grid mkdir output\grid
	@if not exist output\wordlist mkdir output\wordlist

main.exe: $(SRC) $(wildcard *.h) | dirs
	$(CC) $(SRC) -o $@ $(IFLAGS) $(LFLAGS)

opti.exe: $(SRC) $(wildcard *.h) | dirs
	$(CC) $(SRC) -o $@ $(IFLAGS) $(LFLAGS) $(OFLAGS)

clean:
	-del /Q output\*.png output\grid\*.png output\wordlist\*.png

terminate: clean
	-del /Q main.exe opti.exe

# First target = launch by a simple "mingw32-make"
# $(wildcard *.h) : recompile if a header changed
# "| dirs" : force dirs to be executed before compilation
# "-" : ignore a failed command (i.e.: no file to delete), otherwise make would terminate
# "/Q" : delete without asking for confirmation
