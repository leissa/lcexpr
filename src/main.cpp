#include <iostream>

#include "world.h"

int main() {
    World w;

    auto sel = w.select(w.eq(w.zero(), w.one()), w.add(w.one(), w.one()), w.add(w.one(), w.one()));
    std::cout << w.gid << std::endl;
    sel->dump();
}
