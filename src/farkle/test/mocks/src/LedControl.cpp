#include <map>

std::map<int, std::map<int, char>> mockLedState;
std::map<int, int> mockLedIntensity;

int mockSetIntensityCount = 0;
int mockClearDisplayCount = 0;
int mockSetCharCount = 0;
