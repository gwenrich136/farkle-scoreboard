#ifndef PLAYER_LAYOUT_H
#define PLAYER_LAYOUT_H

struct PlayerRows {
    int startRow;
    int numRows;
};

class PlayerLayout {
public:
    static PlayerRows getMapping(int totalPlayers, int playerIdx) {
        int startRow = 0;
        int numRows = 0;
        switch (totalPlayers) {
            case 1: startRow = 2; numRows = 4; break;
            case 2: startRow = (playerIdx == 0) ? 0 : 5; numRows = 3; break;
            case 3: startRow = (playerIdx == 0) ? 0 : (playerIdx == 1 ? 3 : 6); numRows = 2; break;
            case 4: startRow = playerIdx * 2; numRows = 2; break;
            default: startRow = playerIdx; numRows = 1; break;
        }
        return {startRow, numRows};
    }
};

#endif // PLAYER_LAYOUT_H
