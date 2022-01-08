#include "gderivator.h"

int main()
{
    gDerivator der;
    gDerivator_ctor(&der, stderr);

    // gDerivator_lexer(&der, "(123 * x ^ 456 * 8495 + x * 12 * (345 - 567)) / cos(13 * x + 134 - sin 12)");
    // gDerivator_lexer(&der, " 1 * 2 * 3 * x * x * 100 * (11 * 13) + 1");
    gDerivator_lexer(&der, "x + sin(11 * 13 * x) / 10");
    gDerivator_parser(&der);
    // for (size_t i = 0; i < 8; ++i)
    //     gDerivator_dumpNode(&der, i);
    gTree_Node *node = NULL;
    gObjPool_get(&der.tree.pool, (der.tree.root), &node);
    fprintf(stderr, "Real Root = %lu\n", node->child);

    FILE *out = fopen("before.gv", "w");
    gTree_dumpPoolGraphViz(&der.tree, out);
    fclose(out);

    gDerivator_optimize(&der, node->child);
    gObjPool_get(&der.tree.pool, (der.tree.root), &node);
    gDerivator_derivate(&der, node->child);
    gObjPool_get(&der.tree.pool, (der.tree.root), &node);
    gDerivator_optimize(&der, node->child);
    
    out = fopen("after.gv", "w");
    gTree_dumpPoolGraphViz(&der.tree, out);
    fclose(out);

    out = fopen("result.tex", "w");
    gDerivator_dumpLatex(&der, out);
    fclose(out);

    gDerivator_dtor(&der);
}
