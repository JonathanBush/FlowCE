# FlowCE

In FlowCE, the goal is to connect all of the nodes of like color, while filling all of the available empty spaces. This is accomplished by selecting a node and dragging the pipe to the other node of the same color along the desired path.

### Installation

To use FlowCE, you will need to install the CE C Libraries on your calculator. If you do not already have them installed, they can downloaded from tiny.cc/clibs. Please transfer all four .8xv to your calculator. After sending the C Libraries, transfer FlowCE.8cg to your calculator using TI Connect CE. This group contains all of the files necessary to play FlowCE.

### Use

Run FlowCE using Asm(prgmFLOWCE) for OS 5.2 and below. Users with later OS versions may simply run prgmFLOWCE directly from the home screen.

Upon starting FlowCE, you will be presented with the pack selection screen, which allows you to select which level pack you would like to play. Use the arrow keys to move the cursor, and the [2nd] or [enter] key to make a selection. Press [clear] to exit FlowCE.

After selecting the pack, the level selection screen will be displayed. This screen shows the available levels in the selected pack and their respective completion statuses. Use the arrow keys to move the corsor, and the [2nd] or [enter] key to make a selection. Press [clear] to go back to the pack selections screen or to exit FlowCE.

Once you have selected a level, the game board will be displayed. Use the arrow keys to move the cursor, and [2nd] or [enter] to make a selection. Press the clear key to restart the current level or exit the game. If you are able to complete the level with the fewest possible number of moves, a star will be displayed on the level seection screen for that level. If you make more than the minimum number of moves, a check mark will be displayed. After completing the level, a menu with options will be displayed.

### Controls

    arrows  - Move
    [2nd]   - Select/Release
    [clear] - Options menu (Try Again, Select Level, Quit)
    [y=]    - Go to the previous level
    [graph] - Skip to the next level


### Building

You may also compile the latest version of FlowCE by cloning the repository at https://github.com/JonathanBush/FlowCE.git and building it with the CE C SDK.
