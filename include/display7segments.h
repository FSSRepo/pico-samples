
#include "shift_register.h"

#define DIGIT_1         11
#define DIGIT_2         12

class Display7Segments{
public:
Display7Segments();
void setCounter(uint8_t counter);
void update();

private:
ShiftRegister* reg;
uint8_t counter = 0;
};