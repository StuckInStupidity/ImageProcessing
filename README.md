# Word Search Solver

Given a picture of a word search puzzle, the program locates each letter belonging to the grid 
and to the word list, ready to be fed to a character recognition step.

> [!NOTE]
> **Target platform:** Windows
> 
> Written in C with SDL2.

## How it works

1. **Greyscale** conversion with contrast stretching
2. **Median filter** (optional, 3×3) to remove noise
3. **Binarisation** with Otsu's automatic threshold
4. **Automatic rotation**: estimates the skew angle needed to straighten the image
5. **Detection** of the grid and of the word list
6. **Letter extraction** in a way that reconstructs original letter placement
7. **Normalization** of each letter to a 28×28 image

## Requirements

- Windows
- TDM-GCC-64 installed in `C:\TDM-GCC-64`
- SDL2 in `C:\SDL2`
- SDL2_image in `C:\SDL2_image`

## Build

From the project folder:

```
C:\TDM-GCC-64\bin\mingw32-make.exe main.exe
C:\TDM-GCC-64\bin\mingw32-make.exe opti.exe
```

This creates the `output/` folders if needed.
`opti.exe` is built with flag -O2, `main.exe` without.
Add as many C flags as needed in a new variable.

## Usage

```
.\main.exe .\input\image.png
.\main.exe .\input\image.png with
```

| Argument | Description |
|----------|-------------|
| `image.png` | Image of the word search puzzle |
| `with` | Optional. Applies a median filter |

## Output

Everything is written to the `output/` folder:

| File | Content |
|------|---------|
| `greyscaled.png` | Image after greyscale conversion |
| `filtered.png` | Image after the median filter (only with `with`) |
| `binarised.png` | Black and white image |
| `rotated.png` | Straightened image (only if a rotation was applied) |
| `grid.png` | Detected letter grid |
| `wordlist.png` | Detected word list |
| `grid/letter_N.png` | Letters of the grid, 28×28 |
| `wordlist/letter_N.png` | Letters of the word list, 28×28 |

## Terminal output

The program also prints information in the terminal, in this order:

| Buffer | Description |
|------|---------|
| `Usage reminder` | printed at the launch |
| `threshold detected` | the threshold found by the binarisation |
| `angle detected` | the skew angle in degrees found for the rotation |
| `nb_rows` | the number of rows of letters |
| `nb_letters` | the number of letters, per row |

Example:

```
Usage : main.exe image.png predica
	predica : with -> apply median filter
	predica is OPTIONAL

threshold detected : 142
angle detected : -1.500000

nb_rows : 3
nb lettres : 4
nb lettres : 4
nb lettres : 4

nb_rows : 2
nb lettres : 5
nb lettres : 3
```

The first `nb_rows` block describes the grid, the second one the word list.

> [!NOTE]
> Errors are printed on stderr : `Error : args` if the number of arguments is wrong
> 
> Or the SDL error message if SDL_image fails to initialize or the image cannot be loaded

## Cleaning

```
C:\TDM-GCC-64\bin\mingw32-make.exe clean
```

Deletes the generated images in `output/`. `main.exe` is kept, so you can run it on another image without recompiling.

```
C:\TDM-GCC-64\bin\mingw32-make.exe terminate
```

Deletes the generated images **and** `main.exe`. The next build recompiles everything.

## Project structure

| File | Role |
|------|------|
| `main.c` | Entry point and processing pipeline |
| `processing.c/.h` | Greyscale, median filter, binarisation |
| `rotate.c/.h` | Automatic and manual rotation |
| `detection.c/.h` | Grid and word list detection, letter extraction |
| `misc.h` | Shared definitions |
| `Makefile` | Build and cleaning rules |
