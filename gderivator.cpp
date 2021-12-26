#include "gderivator.h"

int main()
{
    gDerivator der;
    gDerivator_ctor(&der, stderr);

    gDerivator_lexer(&der, "sin(1234 + 3) + 0x10 - 0.013 + x / cos(x - 12/13)");
    gDerivator_parser(&der);
    
    FILE *out = fopen("dump.gv", "w");
    gTree_dumpPoolGraphViz(&der.tree, out);
    fclose(out);

    gDerivator_dtor(&der);
}
