This repository is based on [Yuki Koyama-san's work](https://github.com/yuki-koyama/sequential-line-search/tree/master) but focus on Python implementation.

# Modifications
- Added the interface for displaying the dynamic process of updating the slider space, the parameter space and the function curve / image. \
in `gui/window.py` using `PyQt5`
- Added a script of Python examples \
in `python-examples/seq-line-search-vis.py` 

**Demos**:
<p align="center">
<img src="docs/demo.gif" width=50% height=50%/>
</p>

<p align="center">
<img src="docs/img_interface.png" width=50%/>
</p>

# Sequential Line Search

![](https://github.com/yuki-koyama/sequential-line-search/workflows/macOS/badge.svg)
![](https://github.com/yuki-koyama/sequential-line-search/workflows/Ubuntu/badge.svg)
![](https://github.com/yuki-koyama/sequential-line-search/workflows/macOS-python/badge.svg)
![](https://github.com/yuki-koyama/sequential-line-search/workflows/Ubuntu-python/badge.svg)
![GitHub](https://img.shields.io/github/license/yuki-koyama/sequential-line-search)

A C++ library for performing the **sequential line search** method (which is a human-in-the-loop variant of **Bayesian optimization**).

![](docs/concept.jpg)

## Publication

Yuki Koyama, Issei Sato, Daisuke Sakamoto, and Takeo Igarashi. 2017. Sequential Line Search for Efficient Visual Design Optimization by Crowds. ACM Trans. Graph. 36, 4, pp.48:1--48:11 (2017). (a.k.a. Proceedings of SIGGRAPH 2017)
DOI: https://doi.org/10.1145/3072959.3073598

![](docs/publication.jpg)

## Installation (Python)
(For the C++ installation please refer to the [original repository](https://github.com/yuki-koyama/sequential-line-search/tree/master).)

This library has a [python binding](./python), named `pySequentialLineSearch`, which is a subset of the C++ library realized by [pybind11](https://github.com/pybind/pybind11).

### Prerequisites
Eigen <http://eigen.tuxfamily.org/>
```
# macOS
brew install cmake eigen

# Linux
sudo apt install cmake libeigen3-dev
```

### Install via PyPI
(Tested on Python `3.6`, `3.7`, and `3.8`.)
```bash
pip install git+https://github.com/yuki-koyama/sequential-line-search
```

## Core code
The core algorithm is implemented in `include/sequential-line-search/*.hpp` and `src/*.cpp`. This repository also contains the following example demos:

- **bayesian_optimization_1d**: A simple demo of the standard Bayesian optimization applied to a one-dimensional test function.
- **sequential_line_search_nd**: A simple demo of the sequential line search method applied to a multi-dimensional test function.
- **bayesian_optimization_1d_gui**: A visual demo of the standard Bayesian optimization applied to a one-dimensional test function.
- **bayesian_optimization_2d_gui**: A visual demo of the standard Bayesian optimization applied to a two-dimensional test function.
- **preferential_bayesian_optimization_1d_gui**: A visual demo of the preferential Bayesian optimization with a simple pairwise comparison query style applied to a one-dimensional test function.
- **sequential_line_search_2d_gui**: A visual interactive demo of the sequential line search method applied to a two-dimensional test function.
- **sequential_line_search_photo**: A visual interactive demo of the sequential line search method where a photograph is enhanced using six-dimensional parameters.

To use the demos, please follow the c++ installation and run for example
```bash
./demos/sequential_line_search_nd/SequentialLineSearchNd
```

## Supported Environments

We have tested the C++ portion of our codebase on macOS using the clang compiler and on Ubuntu 20.04 using the g++ compiler. As for the Python portion, it has been tested on versions 3.7 through 3.11.

## Technical Details

### Gaussian Process Kernel

- ARD Matern 5/2 kernel (default; recommended in [Snoek et al. NIPS 2011])
- ARD squared exponential kernel (used in [Koyama et al. SIGGRAPH 2017])

### Gaussian Process Kernel Hyperparameters

The optimizer API takes a boolean named `use_map_hyperparameters` as input. If this is `true`, the optimizer calculates the kernel hyperparameters for every iteration via the MAP estimation, as described in [Koyama et al. SIGGRAPH 2017]. If this is `false`, the optimizer just uses default values, making the optimization more efficient.

### Finding Maximizer of Acquisition Function

Finding the global maximizer of the acquisition function is a difficult problem since it often has multiple local maxima and is high-dimensional.

This implementation offers two approaches for this problem:

- The first option is to perform DIRECT (a derivative-free global optimization algorithm) and then refine the solution using L-BFGS (a gradient-based local optimization algorithm).
- The second option is to perform L-BFGS multiple times with many different initial solutions and then pick up the best solution.

See `src/acquisition-function.cpp` for details.

### Acquisition Function Choices

The following acquisition functions are supported:

- Expected improvement (EI) [default; used in the original paper]
- Gaussian process upper confidence bound (GP-UCB)

### Search Space

This implementation assumes that the search space is always [0, 1]^D. When you want to handle a different search space, you need to normalize the target space into [0, 1]^D manually.

### Construction of Slider Space

A slider space is constructed by choosing two end points. One of the two end points is always selected by maximizing the acquisition function (i.e., x^{EI} when using EI as the acquisition function). The other end point is selected by one of the following options:

- The point that provides the largest expected value (i.e., x^{+}) (default; used in the original paper)
- The point that is selected in the last subtask (i.e., x^{chosen}) (suggested in [Koyama+, 2020])

## See Also

Sequential Gallery (SIGGRAPH 2020) is a more recent publication on the same topic (i.e., human-in-the-loop design optimization).
- Project page: <https://koyama.xyz/project/sequential_gallery/>
- GitHub: <https://github.com/yuki-koyama/sequential-gallery>

BO as Assistant (UIST 2022) is a research project using this library for a different purpose (i.e., for generating design suggestions during slider manipulation).
- Project page: <https://koyama.xyz/project/bo-as-assistant/>

