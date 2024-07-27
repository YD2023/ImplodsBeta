# Implods Beta 1.0.2

Bitboard chess engine that tests at a rating of 1700-1800 at a depth of 5 while utilizing a nega-max algorithm with alpha-beta pruning and a quiescence search for efficiency.

## Description

This project is a chess engine implemented in C++, designed to play and analyze chess games. The engine uses advanced techniques for move generation, evaluation, and search algorithms to determine the best moves in a given position. It can interact with any chess GUI interface that uses the Universal Chess Interface (UCI) protocol.

## Features
* Bitboard Representation: Utilizes 64-bit integers to represent the state of the board, allowing highly efficient computation and simultaneous manipulation using bitwise operations
* Negamax Algorithm with Alpha-Beta Pruning: Applys variation of minimax for zero-sum games like chess by using a single-evaluation function
* Quiescence Search: Uses selective deepening to focus on capture moves and checks and extends the search at unstable positions (where there are captures and checks) to avoid the horizon effect.
* UCI Protocol support: Implements standard UCI loop to make the engine compatible with all UCI GUIs.
* Debugging tools: Provides functions to test performance, print readable boards, and move lists for debugging purposes

## Getting Started

### Technologies Used

* C++: Primary programming language for back-end
* Libraries: Algorithms library for optimized search and sort, standard C++ libraries for input/output and string manipulation
* OS: Developed and tested on Windows 10, but should be compatible with other operating systems with minor modifications
* Arena Chess GUI: Graphical interface used to optimize search and evaluation functions
* git Version Control: Personal use for backing up files

### Installing

* Download the source code: Clone the repository from GitHub
```
git clone https://github.com/your-repository/implods-beta.git
cd implods-beta
```
* Compile the program
```
g++ -o implods_beta main.cpp
```
* Run the program in terminal or upload the executable to a UCI based chess GUI

## Version History

* 1.0.2: Web Front-end coming soon!
* 1.0.2
   * Implemented move scoring and ordering to improve efficiency
*  1.0.1
    * Initial Release


## Acknowledgments
Thank you to the Chess Programming Wiki for providing invaluable guidance to players and programmers as well as countless open-source chess engine implementations for inspiration
