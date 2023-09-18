#include <iostream>

#include "world.h"

int main() {
    World w;

#if 0
    auto x = w.id('x');
    auto y = w.id('y');
    auto sel = w.select(w.eq(w.lit(0), w.lit(1)), w.add(x, y), w.add(w.lit(1), w.lit(1)));
    //w.lit(1)->expose();
    sel->dot();
    auto add = w.add(x, y);
#endif
    auto x = w.id('x');
    auto y = w.id('y');
    auto add = w.add(x, y);
    add->dot();
}
