# AMMM

- [https://github.com/goktugc7/AMMM](https://github.com/goktugc7/AMMM)
- [https://github.com/marcelcases/miri-ammm-labs](https://github.com/marcelcases/miri-ammm-labs)
- [https://github.com/eloygil/AMMM-Project](https://github.com/eloygil/AMMM-Project)
- [https://github.com/jeromepasvantis/AMMM](https://github.com/jeromepasvantis/AMMM)

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
  ./generator [OPTION...]

  -n, --numplayers arg  Number of players. has to be odd (default: 3)
  -s, --seed arg        Seed for the random engine (default: 1)
      --every_instance  Generate every instance up to [numplayers]
      --output_dir arg  Output directory for generated instance(s) (default: 
                        ../instances/)
  -h, --help            Print help

```

The good thing about this is that the instances are reproducible, as we specify the seed. Creating them is as simple as:

```
./generator -n 31 -s 1 --every_instance
```

With that command, we create instances from 3 to 31 (3, 5, 7, ..., 31) at directory ../instances using seed 1.

## ILP

### Redundant vs non-redundant

[time_per_instance.pdf](https://github.com/IEncinas10/AMMM/files/9900567/time_per_instance.pdf)
