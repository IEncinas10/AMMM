# AMMM

This document contains the necessary instructions to reproduce our AMMM project results. We have implemented it in C++ and use some libraries for printing and reading input files. Of couse, we assume a C++ compiler and CPLEX are already installed.


## Dependencies

- Boost libraries 
- Sort of recent C++ impl (>= C++17 [?])
- [fmt](https://github.com/fmtlib/fmt)

### Install boost libraries 

```
sudo apt install libboost-all-dev
```
### Install fmt
https://packages.debian.org/source/sid/fmtlib

## Compilation

Every program has its corresponding makefile in order to facilitate the building process. The compilation times are not great because of the usage of the libraries we use to facilitate the user interface / file input reading (C++ templates are slow to compile)

## Instance generation


We can generate instances with an arbitrary number of players. We can also generate the range of instances
with sizes [3, n] if the option *--every_instance* is used.
```
$ ./generator --help
AMMM Course Project 2022
Instance generator
Usage:
  ./generator [OPTION...]

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
Just out of curiosity, we provide three ILP models:
- redundant.mod: ILP with redundant constraints 
- clean.mod: ILP with what we believe is the minimum amount of constraints
- rest.mod: ILP that just gets the optimal resting days(*). Actual pairings can be done as we do in our metaheuristic code. 

```
oplrun clean.mod instance_filepath
```

(*) This ILP was developed to confirm our suspicion that the costly thing for CPLEX is getting a valid pairing, and not optimizing the objective function. For more information see the report itself.


## (Meta)Heuristics

To execute our solver we have to compile solver.cpp and specify the instance we want to solve, whether we want local search or not and the value of alpha for GRASP.

```
AMMM Course Project 2022
Solver
Usage:
  ./solver [OPTION...]

  -i, --instance arg  Path to the instance to solve
  -l, --localsearch   Enable local search. It's implied whenever we do GRASP
      --no-calendar   Avoid printing full calendar and point matrix
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
Of course, results are obtained compiling with optimizations and adding -DNDEBUG. This removes the asserts we do in order to check that our solution is correct. That is:

- We check every player plays vs every other player
- Check 50% black, 50% white
- Check total number of matches


This slows down the program a lot, and are there just to check the validity of our solution just in case.

### Tuning alpha

The tuner will select the smallest alpha that gives the best error possible. Tries alphas [0, 1] with step 0,01. Instances tells the biggest instance
we want to solve. Instances have to be stored in the predefined folder.

```
AMMM Course Project 2022
Alpha tuner
Usage:
  ./tuner [OPTION...]

      --instances arg  Range from {3..arg} with step 2
  -h, --help           Print help
```

For example: 
```
./tuner --instances 11
```

Would try instances 3, 5, 7, 9 and 11.

## Reproducing results

We are providing the instances we've used for our report, so running the ILP, Greedy and Local Search should give the same answer(\*). GRASP might vary as we're using random numbers, and although we're specifying the seed for the random number generator we have no guarantee that it gives the same result under different libstdc++ implementations.

(\*) Maybe the std::sort is implemented differently so they might vary slightly (rest vector, tournament schedule) but the objective function value should be exactly the same.

Also, we have experienced minor differences in the objective functions for some *alphas* because of how floating points are stored/represented internally.

