# RP6502 Snake Game using the RP6502 CC65 VSCode

This is an example project using the Picocomputer 6502 cc65 VSCode scaffolding for the game: Snake.

Credit for the original source code to: https://github.com/brentward/snake

"Snake" illustrates fundemental graphics available on the picocomputer's video system
using "mode-3" bitmapped graphics as well as basic keyboard input control.
The source offers an opportunity to learn by its example.
The source has been updated to use the latest cc65 picocomputer build-system scaffolding; 
the original's SNES-controller inputs have been commented-out; new updated methods for various 
input controls such as the PS4 Gamepad are now available on newer pico-RIA firmware (ver-0.7).  

Snake code here uses only keyboard input: use the W S A D keys to control the snake's movements.

Tip: if the snake moves too fast for you, use the pico's "SET PHI2" command to slow down the 6502.


### Linux Tools Install:
 * [VSCode](https://code.visualstudio.com/). This has its own installer.
 * A source install of [CC65](https://cc65.github.io/getting-started.html).
 * The following tools installed from your package managers:
    * `sudo apt-get install cmake python3 pip git build-essential`
    * `pip install pyserial`

### Windows Tools Install:
 * [VSCode](https://code.visualstudio.com/). This has its own installer.
 * A source install of [CC65](https://cc65.github.io/getting-started.html).
   Do not skip the step about adding bin to your path.
 * Install python by typing `python3` which will launch the Microsoft Store
   where you start the install. If python runs, this has already been done,
   exit python with Ctrl-Z plus Return.
 * Install the python serial library with `pip install pyserial`.
 * `winget install -e --id Kitware.CMake`.
 * `winget install -e --id GnuWin32.Make`.
    Add "C:\Program Files (x86)\GnuWin32\bin" to your path.
 * `winget install -e --id Git.Git`.

### Getting Started:
Go to the [GitHub template](https://github.com/picocomputer/vscode-cc65) and
select "Use this template" then "Create a new repository". GitHub will make a
clean project for you to start with. Then you can download the repository
and open the files.

```
$ git clone [path_to_github]
$ cd [to_where_it_cloned]
$ code .
```

Install the extensions and choose the default or obvious choice if VSCode
prompts you.

You can build with F7. Running a program is done with "Run Build Task..."
CTRL-SHIFT-B. If the default communications device doesn't work, edit ".rp6502"
in the project root folder. This file will be created the first time you
"Run Build Task..." and will be ignored by git.

Edit CMakeLists.txt to add new source and asset files. It's
pretty normal C/ASM development from here on.
