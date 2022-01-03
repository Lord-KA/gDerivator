#include "gderivator.h"

int main()
{
    gDerivator der;
    gDerivator_ctor(&der, stderr);

    gDerivator_lexer(&der, "(123 * x ^ 456 * 8495 + x * 12 * (345 - 567)) / cos(13 * x + 134 - sin 12)");
    // gDerivator_lexer(&der, "cos(13 * x + 134 - sin 12)");
    // gDerivator_lexer(&der, "sin (x + 4) / 13");
    gDerivator_parser(&der);
    // for (size_t i = 0; i < 8; ++i)
    //     gDerivator_dumpNode(&der, i);
    gTree_Node *node = NULL;
    gObjPool_get(&der.tree.pool, (der.tree.root), &node);
    fprintf(stderr, "Real Root = %lu\n", node->child);

    FILE *out = fopen("before.gv", "w");
    gTree_dumpPoolGraphViz(&der.tree, out);
    fclose(out);

    gDerivator_derivate(&der, node->child);
    // for (size_t i = 4; i < 15; ++i)
    //     gDerivator_dumpNode(&der, i);
    
    out = fopen("after.gv", "w");
    gTree_dumpPoolGraphViz(&der.tree, out);
    fclose(out);

    gDerivator_dtor(&der);
}
