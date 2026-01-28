#include "uci.h"
#include "attacks.h"
#include "rays.h"

int main() {
    initRays();
    initAttackTables();
    
    UCILoop();
    
    return 0;
}
