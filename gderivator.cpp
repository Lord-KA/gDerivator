#include "gderivator.h"

int main()
{
    gDerivator der;
    gDerivator_ctor(&der, stderr);

    gDerivator_lexer(&der, "123 * x");
    gDerivator_parser(&der);
    gDerivator_derivate(&der, 2);
    
    FILE *out = fopen("dump.gv", "w");
    gTree_dumpPoolGraphViz(&der.tree, out);
    fclose(out);

    gDerivator_dtor(&der);
}
