# tictactoe
Cursed Tic Tac Toe

### How to play
1. Use keys to make moves (eg. Y = top right corner of the center layer).
2. Hold left shift or space to place a move in a different 4th dimension layer. 
3. Click and drag to rotate the camera; scroll to zoom in and out. Enter to reset the camera.
4. A win occurs when a three in a row occurs twice in one move.
5. Once a player has won, all other moves will be dimmed. The winning move retains full brightness, and the moves contributing to the win are dimmed less.
6. Press escape to quit. More keybindings can be found in `keybinds.cpp`

### Installation
Dependencies: opengl, glew, glfw.  
Clone the repositiory and run `make all` in the directory. This should compile the code, creating some object files (ending in `.o`) and an executable named `toe`, which can be run to play the game.
