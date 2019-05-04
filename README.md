# Evolutionary algorithm for adaptive variable-reflectivity beam-splitter estimation

We implement evolutionary algorithm to the problem of adaptive variable-reflectivity beam-splitter estimation, which is an example of quantum control problems. The aim of this project is to create a library containing modules that streamlines the construction of an optimization algorithm for quantum control problems. Access to modules of optimization algorithms provides the building blocks that users can use to tweak the algorithm to their needs.

Features:

 * Library in C++
 * Support MPI
 * Support VSL and GPU for random number generation
 * Include modules for particle swarm optimization (PSO) and differential evolution (DE)
 * Include uniform and clustered method of initializing solution candidates
 * Include access to user specified accept-reject criteria
 * Include preliminary support for multi-objective calculation
 * Support Autotools

## Copyright and license
This is a free software made available under the [GNU GENERAL PUBLIC LICENSE](http://www.gnu.org/licenses/gpl-3.0.html), which means you can share, modify, and redistribute this software. While we endeavor to make this software as useful and as error-free as possible, we cannot make any such guarantee, and the software is hence released **without any warranty**.

## Compilation and installation

The project contains the support for compilation using Autotools, and has been tested using GNU Compile Chain (GCC) and Intel Compilers. The Intel VSL library and CUDA are automatically detected. An MPI implementation is required to compile and run the code.

If you cloned the Git repository, first run `autogen.sh` in order to create missing files and generate the executable configure from configure.ac. 

Follow the standard POSIX procedure:

    $ ./configure [options]
    $ make
    $ make install

To use the Intel compilers, set the following environment variables:

    export CC=/path/of/intel/compiler/icc
    export CXX=/path/of/intel/compiler/icpc
    export OMPI_CC=/path/of/intel/compiler/icc
    export OMPI_CXX=/path/of/intel/compiler/icpc

In order to use icc and icpc compilers, you have to set these variables
so the mpic++ will invoke icpc instead of the default compiler.

Options for configure

    --prefix=PATH           Set directory prefix for installation
    --with-mpi=MPIROOT      Use MPI root directory.
    --with-mpi-compilers=DIR or --with-mpi-compilers=yes
                              use MPI compiler (mpicxx) found in directory DIR, or
                              in your PATH if =yes
    --with-mpi-libs="LIBS"  MPI libraries [default "-lmpi"]
    --with-mpi-incdir=DIR   MPI include directory [default MPIROOT/include]
    --with-mpi-libdir=DIR   MPI library directory [default MPIROOT/lib]

The above flags allow the identification of the correct MPI library the user wishes to use. The flags are especially useful if MPI is installed in a non-standard location, or when multiple MPI libraries are available.

    --with-cuda=/path/to/cuda           Set path for CUDA

The configure script looks for CUDA in /usr/local/cuda. If your installation is not there, then specify the path with this parameter. If you do not want CUDA enabled, set the parameter to ```--without-cuda```.


    --with-vsl=PATH    prefix where Intel MKL/VSL is installed

Specify the path to the VSL installation with this parameter.

## Usage

The program is designed to work on HPC clusters and it requires MPI to run. The basic use is as follows:

    $ [mpirun -np NPROC] phase_estimation [config_file]


### Input and configuration

Arguments:

    config_file              Configuration file name

If it is run without a configuration file, some default values are taken for all parameters; the exact settings are identical to the one in the provided `default.cfg` file. The configuration file is a plain text file with the name of the parameter on the left, followed by an equation sign surrounded by a space on either side, and a value on the right-hand side. For example, the contents of `default.cfg` are as follows:

    pop_size = 20
    N_begin = 4
    N_cut = 5
    N_end = 10
    iter = 100
    iter_begin = 300
    repeat = 10
    output_filename = output.dat
    time_filename = time.dat

If you supply a configuration file, but do not set a specific value to every possible option, the default values are again the ones described in `default.cfg`.

The meaning of the individual parameters:

  - `pop_size`: population size.
  
  - `N_begin`: the starting number of particles.
  
  - `N_cut`: the number of particles where the program use cluster initialization around previous solution.
  
  - `N_end`: the final number of particles.

  - `iter`: number of iterations when cluster initialization is used.
  
  - `iter_begin`: number of iteration when uniformly random initialization is used.
  
  - `repeat`: number of time the candidates are compute before the best candidate is selected after the optimization
  
  - `optimization`: choose the heuristic optimization algorithm: de (differential evolution) or pso (particle swarm optimization)
  
  - `output_filename`: the name of the file to write the results to.
  
  - `time_filename`: the name of the file to write the time taken to run the program for each number of variables.

  - `random_seed`: fix a random seed. If it is not specified, the random number generator is initialized with the system time

  - `data_end`: set where accept-reject criterion based on error from expected solution is used

  - `prev_dev`: the boundary for the cluster initialization -- for variables that are initialized from previous solution 

  - `new_dev`: the boundary for the cluster initialization -- for new variable

  - `t_goal`: parameter corresponding to the error which the algorithm will accept to solution


### Output

The program outputs two files, one containing the policy and fitness value (output.dat) and the other containing the CPU time used in finding the policy (time.dat). The numbers are updated for every _N_ and can be used to track the progress of the optimization.

Example of output:

output.dat

```
#N 	 Sharpness 	 Policy
4	0.854507	4.91056	5.44221	5.73905	5.87313	
5	0.888902	4.92427	5.46181	5.76412	5.88404	5.95493	
6	0.90593	4.9164	5.46353	5.75503	5.89696	5.95538	6.05514	
7	0.920337	4.91471	5.48735	5.72474	5.8765	5.94925	6.06185	6.14271	
8	0.932388	4.88864	5.45983	5.72587	5.88542	5.94729	6.06073	6.12173	6.17251	
9	0.941697	4.87251	5.45756	5.74001	5.90138	5.95748	6.04226	6.13899	6.16975	6.07994	
10	0.941703	4.86374	5.42822	5.74102	5.87777	5.96852	6.04537	6.13999	6.13744	6.0704	6.25615	

```

time.dat

```
#N 	 Time
4	1
5	0
6	1
7	1
8	0
9	0
10	0

```

## Expanding the library

The intention of this project is to create a library that can be used for solving multiple quantum control problems. The code is therefore designed to ease the process of including new problems and algorithms and make the selection of problems and algorithms as error-free as possible. The following document is a guide on how users can write and include their own problems and optimization algorithms to the existing library, and what is needed to customize and compile the code.

Readers are assumed to be familiar with population-based optimization algorithm, C++, object-oriented programming, class hierarchy and inheritance, and polymorphism.

### User-specified components

The orange boxes correspond to the components in which the users specify before compiling the program. `Phase` class contains the modules for the adaptive variable beam-splitter reflectivity estimation problem, which can be replaced with other problems. To select a problem of choice, replace `Phase()` by the constructor of the class in `main()` in the following line.
```
problem = new Phase(numvar, gaussian_rng, uniform_rng);
```

The Phase class is accessed through the `Problem` class. The pointer is given to the `OptAlg` class to be used for computing the fitness values and accept-reject criteria.

### Optimization algorithms

The choice of optimization algorithm is specified in the configuration file. Otherwise, it can also be coded in `main()` in the following line if necessary.

```
opt = new DE(problem, gaussian_rng, pop_size);
```

### MPI

The MPI library is required for the program to run, as the program is designed to spread the solution candidate evenly on a group of processors. The processors communicate in the following situations.

* Constructing a new set of candidates from existing population
* Finding the best candidate in the population or subset of the population
* Selecting the best candidate as a solution

### Add a new problem

A new problem should be written as a class derived from `Problem` class. There are five functions that the users must include in the new problem.

* `fitness()` is a function intended to be a wrapper for changing conditions in which the fitness function is evaluated.
* `avg_fitness()` is the function for calculating the fitness value.
* `T_condition()` is a function for calculating additional conditions for when the optimization algorithm is set to accept solution after T iteration.
* `error_condition()` is a function for calculating additional conditions for when optimization algorithm is set to accept solution from error bound.
* `boundary()` is used to keep the solution candidate within the boundary of the search space.

This class does not use any MPI functionalities.

### Add new algorithm

New algorithms can be added to the library of optimization algorithms by creating a derived class from the `OptAlg` class. The functions are designed based on swarm intelligence algorithms and evolutionary algorithms which share the same backbone functions for initializing the population, for selecting the final solution candidate, and so forth. Aspects that are specific to the algorithm, such as how the new candidates are generated and selected, are declared as virtual function in `OptAlg` to allow the functions to be called from the derived class.

The functions, including virtual functions, are listed in OptAlg class document.

### Constructing optimization algorithm

The library provides the module for users to contract the optimization algorithm in `main()`. The basic structure of the algorithm is given in main.cpp, which compile to the following structure.

* Initialize MPI and setting is compute to spread the number of candidates evenly on the processors
* Initialize problem and select the optimization algorithm
* Population is initialized using a user specified method
* The fitness values are computed and the population is prepared for the optimization
* The iterative optimization commences until the solution satisfied the specified criterion
* The program writes the fitness value, the solution, and the computational time as .dat files
* Program terminates

Most of these functionalities are in `OptAlg` class.

For adaptive variable beam-splitter estimation, the program runs many consecutive optimization problems with different number of variables _N_ and the accept-reject criteria changes for different sets of _N_, which is possible by changing conditions given to the optimization algorithm in `main()`. 

### Changing the compilation setting

When a new problem and/or algorithm is included to the library, the following line in `src\Makefile.in`,

`OBJS=main.o candidate.o phase_loss_opt.o io.o problem.o mpi_optalg.o mpi_pso.o mpi_de.o candidate.o rng.o aux_functions.o`,

should be updated to include the new class.

## Acknowledgement

This software was initially developed by [Pantita Palittapongarnpim](https://github.com/PanPalitta) and [Peter Wittek](https://github.com/peterwittek) with the financial support of [NSERC](http://www.nserc-crsng.gc.ca/index_eng.asp) and [AITF](http://www.albertatechfutures.ca/). The current version is a modification by [Hamza Qureshi](https://github.com/thehamzaq).

The computational work was enabled by support from [WestGrid](https://www.westgrid.ca/) and [Calcul Quebec](http://www.calculquebec.ca/en/) through [Compute Canada](https://www.computecanada.ca/).

## References

1. Pantita Palittapongarnpim, Peter Wittek and Barry C. Sanders. Controlling adaptive quantum phase estimation with scalable reinforcement learning. In *Proc. 24th European Symposium on Artificial Neural Networks, Computational Intelligence and Machine Learning* (ESANN 2016): 327--332, Apr 2016.

2. Pantita Palittapongarnpim, Peter Wittek and Barry C. Sanders. Single-shot adaptive measurement for quantum-enhanced metrology. In *Proc. of SPIE Quantum Communications and Quantum Imaging XIV*, **9980** :99800H, Sep 2016. [DOI](https://doi.org/10.1117/12.2237355) [arXiv:1608.06238](https://arxiv.org/abs/1608.06238)
  
3. Pantita Palittapongarnpim, Peter Wittek, Ehsan Zahedinejad, Shakib Vedaie and Barry C. Sanders. Learning in quantum control: High-dimensional global optimization for noisy quantum dynamics, *Neurocomputing* **268**: 116--126, Apr 2017. [DOI](https://doi.org/10.1016/j.neucom.2016.12.087) [arXiv:1607.03428](https://arxiv.org/abs/1607.03428)

4. Pantita Palittapongarnpim, Peter Wittek and Barry C. Sanders. Robustness of learning-assisted adaptive quantum-enhanced metrology in the presence of noise. In *Proc. 2017 IEEE International Conference on Systems, Man and Cybernetics* (2017 SMC): 294--299, Dec 2017. [DOI](https://doi.org/10.1109/SMC.2017.8122618)
 
5. Pantita Palittapongranpim and Barry C. Sanders. Robustness of Adaptive Quantum-Enhanced Phase Estimation. 2018. [arXiv:1809.05525](https://arxiv.org/abs/1809.05525)
