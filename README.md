# AMMM

## Dependencies

- Boost libraries 
- Sort of recent C++ impl
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

## (Meta)Heuristics

To execute our solver we have to compile solver.cpp and specify the instance we want to solve, whether we want local search or not and the value of alpha for GRASP.

```
AMMM Course Project 2022
Solver
Usage:
  ./solver [OPTION...]

  -i, --instance arg  Path to the instance to solve
  -l, --localsearch   Enable local search. It's implied whenever we do GRASP
  -a, --alpha arg     Set alpha for GRASP [0-1]. If alpha is different to (0) GRASP 
                      algorithm will be set (default: 0)
  -h, --help          Print help
```

- Greedy: 
```
./solver -i instance_filepath 
```
- Greedy + LS: 
```
./solver -i instance_filepath --localsearch
```
- GRASP: 
```
./solver -i instance_filepath --alpha ALPHA_VALUE
```

### Details
Of course, results are obtaining compiling with optimizations and adding -DNDEBUG. This removes the asserts we do in order to check that our solution is correct. That is:

- We check every player plays vs every other player
- Check 50% black, 50% white
- Check total number of matches


This slows down the program a lot, and are there just to check the validity of our solution just in case.

## Reproducing results

We are providing the instances we've used for our report, so running the ILP, Greedy and Local Search should give the same answer(\*). GRASP might vary as we're using random numbers, and although we're specifying the seed for the random number generator we have no guarantee that it gives the same result under different libstdc++ implementations.

(\*) Maybe the std::sort is implemented differently so they might vary slightly (rest vector, tournament schedule) but the objective function value should be exactly the same.

## Slides

```
xelatex -shell-escape main.tex
```
