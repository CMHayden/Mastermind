# Mastermind
A project using a raspberry pi and some connections to create a version of the Mastermind board game from the 70s. Apart from the electronics, a mixture of C and assembly will be used for the programming side of things.
Mastermind or Master Mind is a code-breaking game for two players. In this project, one player shall be the raspberry pi and the other shall be the user. The raspberry pi will create a random code that the user has to try to break. Attempting to break this code is done in rounds, where the player guesses and the raspberry pi tells them how close they were to the right answer in terms of exact and approximate matches.

Project details:
Systems level application in C and assembler, running on a Raspberry Pi2 with attached devices.
Hardware: Raspberry Pi2 kit.
Software: SD card with Raspbian 7 "Wheezy". Must use standard GNU toolchain (gcc, as, ld) Use GNU debugger (gdb) for debugging.

------------
Task: implement an instance of the mastermind board game using C and ARM assembler as implementation language. Needs to run on a Raspberry Pi2 with two LEDs, a button, an an LCD. These devices should be connected via a breadboard.

------------
Coding:
  No external libraries allowed.
  Encode C colors as numbers from 1 - C.
  Display sequence of pegs as sequence of numbers.
  Testing use C=3 and a sequence length of 3.
  In debug mode, the program should print the secret sequence at the beginning.
  Low level programming must be done in ARM assembler (setting the mode of a pin, writing to an LED or LCD, and reading from a button). Rest can be done in C. (can all be done in C)
  See sample code to help build functionality.

----------
Functionality:
  The application proceeds in rounds of guesses and answers.
  Player enters a sequence of numbers each round.
  A number is entered by pressing the button. Each number is entered as the number of button pressed (2 presses = 2)
  A fixed time-out should be used to separate the input of successive numbers. Use timers (can use C).
  The red LED should blink once to acknowledge input.
  The green LED should blink as many times as the button was pressed.
  Repeat the input and LED echo for each number to be input.
  Once all values have been entered and echo'd, the red LED flashes twice.
  The application has to calculate the number of exact matches (1,2,3 / 2,1,3 -> 3 is exact) and approximate matches ( 1 and 2 in the example before).
  The green LED flashes once for each exact match, then the red LED flashes once, then the green LED flashes once for each approximate match.
  Red LED flashes three times to indicate a new round.
  If the hidden sequence has been guessed, the green LED flashes 3 times while the red LED is on. Otherwise the next turn should start.
  When an LCD is connected: print number of exact and approximate matches as two separate numbers. (not necessary)
  On correct guess, LCD screen prints "SUCCESS" followed by number of attempts required. (not necessary)

  ------------
  Command line usage:
    Provide a command line interface to test its functionality in an automated way.

  -----------
  Submission:
    Submit complete project files containing source code, exe file, and a report in a zip file no later than 15:30 Thursday 29th of March. Submission must be through vision along with a hardcopy of the report with a cover sheet in the drop box.

  -----------
  Report:
    2-5 pages which cover:
      Problem specification.
      Hardware specification and wiring used.
      Short discussion of the code structure, specifying functionality of the main functions.
      List of functions directly accessing the hardware (LEDs, button and LCD) and which parts of the function use assembler and which use C.
      Sample execution of the program in debug mode.
      A summary, covering what was achieved and what not, outstanding features, and what we learnt from the coursework.
