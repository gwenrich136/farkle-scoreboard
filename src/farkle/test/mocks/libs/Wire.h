#ifndef TwoWire_h
#define TwoWire_h

#include <inttypes.h>

class TwoWire {
public:
    void begin();
};

extern TwoWire Wire;
extern int mockWireBeginCount;

#endif
