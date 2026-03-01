#include "uci.h"
#include "attacks.h"
#include "rays.h"
#include "zobrist.h"

int main() {
    initRays();
    initAttackTables();
    initZobrist();
    
    UCILoop();
    
    return 0;
}
