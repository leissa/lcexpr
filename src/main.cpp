#include <iostream>

#include "world.h"

int main() {
    World w;

    auto x = w.id('x');
    auto y = w.id('y');
    auto sel = w.select(w.eq(w.lit(0), w.lit(1)), w.add(x, y), w.add(w.lit(1), w.lit(1)));
    std::cout << w.gid << std::endl;
    sel->dump();
}
