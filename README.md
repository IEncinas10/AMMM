# AMMM

## Dependencies

- Boost libraries 
- Sort of recent C++ impl
- Cmake
- [fmt](https://github.com/fmtlib/fmt)

### Install boost libraries 

```
sudo apt install libboost-all-dev
```
### Install fmt
https://packages.debian.org/source/sid/fmtlib

## Instance generation

We have to compile **generator.cpp** linking with libfmt. Generally, -lfmt 
```
$ ./generator --help
AMMM Course Project 2022
Instance generator
Usage:
  ./a.out [OPTION...]

  -n, --numplayers arg          Number of players. has to be odd (default: 3)
  -s, --seed arg                Seed for the random engine (default: 1)
      --max_points_per_day arg  Maximum points that can be assigned to a given day 
                                (default: 100)
      --every_instance          Generate every instance up to [numplayers]
      --output_dir arg          Output directory for generated instance(s) (default: 
                                ../instances/)
  -h, --help                    Print help
```

The good thing about this is that the instances are reproducible(under the same libc++ implementation), as we specify the seed. Creating them is as simple as:

```
./generator -n 31 -s 1 --every_instance
```

With that command, we create instances from 3 to 31 (3, 5, 7, ..., 31) at directory ../instances using seed 1.

## ILP

### Redundant vs non-redundant

[time_per_instance.pdf](https://github.com/IEncinas10/AMMM/files/9900567/time_per_instance.pdf)

## Slides

```
xelatex -shell-escape main.tex
```
