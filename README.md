# Implods Beta 1.0.3

Bitboard chess engine that tests at a rating of 1700-1800 at a depth of 5 while utilizing a nega-max algorithm with alpha-beta pruning and a quiescence search for efficiency. Using javascript and HTML, there is now a complete front end for any user to play against the engine with ease.

## Description

This project is a chess engine implemented in C++, designed to play and analyze chess games. The engine uses advanced techniques for move generation, evaluation, and search algorithms to determine the best moves in a given position. Using HTML and Javascript, there is now a complete front end for any user to play against the engine! It can also interact with any chess GUI interface that uses the Universal Chess Interface (UCI) protocol.

## Features
* Bitboard Representation: Utilizes 64-bit integers to represent the state of the board, allowing highly efficient computation and simultaneous manipulation using bitwise operations
* Negamax Algorithm with Alpha-Beta Pruning: Applys variation of minimax for zero-sum games like chess by using a single-evaluation function
* Quiescence Search: Uses selective deepening to focus on capture moves and checks and extends the search at unstable positions (where there are captures and checks) to avoid the horizon effect.
* Custom Web GUI: Built User interface on HTML and Javascript to allow players to test the engine themselves with interactive buttons and move listing. 
* UCI Protocol support: Implements standard UCI loop to make the engine compatible with all UCI GUIs.
* Debugging tools: Provides functions to test performance, print readable boards, and move lists for debugging purposes

## Video Demonstration
Here is a demonstration of gameplay on the custom web front-end. I am continuing to work on the design as it is currently a bit rudimentary and hoping to integrate react for a more modern UI. Click on the thumbnail below! : 

[![Watch the video](https://img.youtube.com/vi/CduDCnl6Pus/maxresdefault.jpg)](https://www.youtube.com/watch?v=CduDCnl6Pus)




### Technologies Used

* C++: Primary programming language for back-end
* HTML/CSS: Basic libraries used. Used Jquery to connect web requests to python custom API
* Javascript: Used chess-board and chess.js libraries for seamless front end board UI and interactive play
* Libraries: Algorithms library for optimized search and sort, standard C++ libraries for input/output and string manipulation
* OS: Developed and tested on Windows 10, but should be compatible with other operating systems with minor modifications
* Arena Chess GUI: Graphical interface used to optimize search and evaluation functions
* git Version Control: Personal use for backing up files
* Lichess Bot API: To test against other engines and players

### Installing

* Download the source code: Clone the repository from GitHub
```
git clone https://github.com/your-repository/implods-beta.git
cd implods-beta
```
* Compile the program
```
g++ -o implods_beta Implods_Beta21.exe
```
* Run the app.py in terminal and go to localhost:5000
* I am deploying the server to a web address soon

## Version History

* 1.0.4: Integrate live web address to allow any user to play without repository cloning
* 1.0.3: Complete functional UI for chess engine testing and user interaction using HTML/CSS and Javascript!
* 1.0.2
   * Implemented move scoring and ordering to improve efficiency
*  1.0.1
    * Initial Release


## Acknowledgments
Thank you to the Chess Programming Wiki for providing invaluable guidance to players and programmers as well as countless open-source chess engine implementations for inspiration. Thank you to the creators of chess.js as well as chess-board.js for providing libraries to allow seamless front-end GUI creation.
