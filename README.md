# tictactoe
4th dimensional Tic Tac Toe

### How to play
1. Use keys to make moves (eg. Y = top right corner of the center layer). S=back center, G=center center, K=front center.
2. Hold left_shift/space to place a move in the lower/upper 4th dimension (red/blue). This will also dim moves in other layers.
3. Click and drag to rotate the camera; scroll to zoom in and out. Enter to reset the camera.
4. A win occurs when a three in a row occurs twice in one move. The board will maintain the color of the winning move.
5. 1-4 to enable move recommendations. Left ctrl to highlight recommendations and/or wins.
6. 9 to let the AI move for you. 0 to have the AI play against you.
7. Press escape to quit. More keybindings can be found in `keybinds.cpp` and `keybinds.hpp`.

### Installation
Dependencies: opengl, glew, glfw.  
Clone the repositiory and run `make all` in the directory. This will create an executable named `toe`, which can be run to play the game.  

### Example wins
X wins with final move green center top right (Y)  
![x win 1](https://i.ibb.co/mJCMcBk/xwin1.png)

X wins with final move green center center right (H)  
![x win 2](https://i.ibb.co/W0dq20v/xwin2.png)

X wins with final move blue back top left (SPACE+Q)
![x win 3](https://i.ibb.co/7gPRJX1/xwin3.png)
