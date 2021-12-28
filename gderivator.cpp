#include "gderivator.h"

int main()
{
    gDerivator der;
    gDerivator_ctor(&der, stderr);

    gDerivator_lexer(&der, "123 * 456 * 789 / sin (13/14 + x * cos 3 * sin (13 *4))");
    gDerivator_parser(&der);
    
    FILE *out = fopen("dump.gv", "w");
    gTree_dumpPoolGraphViz(&der.tree, out);
    fclose(out);

    gDerivator_dtor(&der);
}
