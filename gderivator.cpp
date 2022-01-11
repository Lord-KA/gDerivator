#include "gderivator.h"
#include <cstdlib>

int main()
{
    gDerivator der;
    gDerivator_ctor(&der, stderr);

    // gDerivator_lexer(&der, "(123 * x ^ 456 * 8495 + x * 12 * (345 - 567)) / cos(13 * x + 134 - sin 12)");
    // gDerivator_lexer(&der, " 1 * 2 * 3 * x * x * 100 * (11 * 13) + 1");
    char buffer[MAX_LINE_LEN] = "";
    fprintf(stdout, "Provide expression to derivate:\n");
    getline(buffer, MAX_LINE_LEN, stdin);
    gDerivator_lexer(&der, buffer);
    gDerivator_parser(&der);

    gTree_Node *node = NULL;
    gObjPool_get(&der.tree.pool, (der.tree.root), &node);
    #ifdef EXTRA_VERBOSE
        fprintf(stderr, "Real Root = %lu\n", node->child);
    #endif


    gDerivator_optimize(&der, node->child);
    gObjPool_get(&der.tree.pool, (der.tree.root), &node);
    gDerivator_derivate(&der, node->child);

    FILE *out = fopen("before.gv", "w");
    gTree_dumpPoolGraphViz(&der.tree, out);
    fclose(out);

    gObjPool_get(&der.tree.pool, (der.tree.root), &node);
    gDerivator_optimize(&der, node->child);

    gObjPool_get(&der.tree.pool, (der.tree.root), &node);
    gDerivator_optimize(&der, node->child);
    
    out = fopen("after.gv", "w");
    gTree_dumpPoolGraphViz(&der.tree, out);
    fclose(out);

    out = fopen("result.tex", "w");
    gDerivator_dumpLatex(&der, out);
    fclose(out);
    system("pdflatex result.tex && zathura result.pdf");

    gDerivator_dtor(&der);
}
