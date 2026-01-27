# Farkle Scoreboard

This project is a fully-featured, physical scoreboard for the dice game Farkle, built using an Arduino Uno R4 WiFi and a variety of custom-wired displays and LEDs.

## A "Vibe Coding" Experiment

I'm using this project as a way of experimenting with some vibe-coding techniques that I've been hearing about, to see what sticks. Rather than starting with low-level implementation, the focus was first on collaboratively designing the user experience, game flow, and overall "feel" of the product with an AI, then having the AI document all of our dicussions in compact, synsinct design documens in markdown. I did this for the initial design of the gameplay, but we'll also rely on creating a new design each time I add new feature to the project. The design documents in the `/design` directory are the output of this process.

---

## How to Play: A Simple User Guide

### 1. Starting a Game
-   Use the **UP** and **DOWN** buttons to scroll through a list of pre-stored player names.
-   Press the **BANK** (Green) button to add the highlighted player to the game.
-   To create a new player, select the **"Add New Player"** option and use the directional buttons to enter a name. Press **BANK** to confirm.
-   When all players have been added, press the **FARKLE** (Red) button to start the game. You can also press **CLEAR** for a "Start Game?" confirmation prompt.

### 2. Playing Your Turn
1.  **Roll your dice** physically next to the scoreboard.
2.  **Add the score** from your roll using the directional buttons on the `ControlPad`:
    -   **DOWN**: +50 points
    -   **LEFT**: +100 points
    -   **RIGHT**: +500 points
    -   **UP**: +1000 points
3.  **Made a mistake?** Press the **CLEAR** button to reset the points for the current roll to zero and re-enter them.
4.  When you are done with your turn, press the **BANK** (Green) button to add the points from your `at-risk` score to your total and end your turn. Watch the scoreboard animate your success!
5.  If you **Farkle**, the scoreboard will let you know, and your turn will end automatically.

### 3. Winning the Game
-   The game doesn't end immediately when someone passes 10,000 points!
-   When the first player crosses this threshold, the high score display will begin to **blink**. This signals the **final round**. Every other player gets one last turn to try and beat the high score.
-   After the final round, the player with the highest score wins, and the scoreboard will play a celebration animation!

### 4. After the Game
-   After a winner is declared, the scoreboard will enter a "Post-Game" mode. It will show a replay of the game's scores on the LED grid and may attempt to upload game stats.
-   To play again, follow the prompts on the text display. You will be guided through a menu to start a new game.
