#ifndef GDERIVATOR_H
#define GDERIVATOR_H

#define STACK_TYPE size_t
#define ELEM_PRINTF_FORM "%lu"
#define CHEAP_DEBUG

#include "gstack.h"

enum gDerivator_Node_mode {
    gDerivator_Node_mode_none,
    gDerivator_Node_mode_sum,
    gDerivator_Node_mode_sub,
    gDerivator_Node_mode_mul,
    gDerivator_Node_mode_div,
    gDerivator_Node_mode_exp,
    gDerivator_Node_mode_opBrack,
    gDerivator_Node_mode_clBrack,
    gDerivator_Node_mode_var,
    gDerivator_Node_mode_func,
    gDerivator_Node_mode_num,
    gDerivator_Node_mode_unknown,
    gDerivator_Node_mode_CNT,
};

static const char gDerivator_Node_modeView[gDerivator_Node_mode_CNT][10] = {
        "NONE",
        "+",
        "-",
        "*",
        "/",
        "^",
        "(",
        ")",
        "x",
        "FUNC",
        "NUM",
        "UNKNOWN",
    };

static const char gDerivator_Node_modeViewLatex[gDerivator_Node_mode_CNT][10] = {
        "NONE",
        "+",
        "-",
        "\\cdot",
        "\\frac",
        "^",
        "(",
        ")",
        "x",
        "FUNC",
        "NUM",
        "UNKNOWN",
    };

enum gDerivator_Node_func {
    gDerivator_Node_func_sin,
    gDerivator_Node_func_cos,
    gDerivator_Node_func_ln,
    gDerivator_Node_func_log,
    gDerivator_Node_func_CNT,
};

static const char gDerivator_Node_funcView[gDerivator_Node_func_CNT][10] = {
        "sin",
        "cos",
        "ln",
        "log",
    };

struct gDerivator_Node 
{
    gDerivator_Node_mode mode;
    gDerivator_Node_func func;
    double value;
    size_t position;
} typedef gDerivator_Node;

static const char GDERIVATOR_DELIMS_LIST[] = " +-*/^()";

typedef gDerivator_Node GTREE_TYPE;

#include "gtree.h"

static const size_t MAX_LINE_LEN = 1000;
static const size_t GDERIVATOR_MAX_LIT_LEN = 100;
static const size_t GDERIVATOR_LEX_LIM = 1000;

static const double GDERIVATOR_EPS = 1e-3;

enum gDerivator_status {
    gDerivator_status_OK,  
    gDerivator_status_BadStructPtr,
    gDerivator_status_TreeErr,
    gDerivator_status_ObjPoolErr,
    gDerivator_status_FileErr,
    gDerivator_status_BadPtr,
    gDerivator_status_BadId,
    gDerivator_status_BadInput,
    gDerivator_status_EmptyLexer,
    gDerivator_status_EmptyTree,
    gDerivator_status_ParsingErr_UnknownLex,
    gDerivator_status_ParsingErr_NoBrack,
    gDerivator_status_DecompositionErr,
    gDerivator_status_CNT,
};

static const char gDerivator_statusMsg[gDerivator_status_CNT + 1][MAX_LINE_LEN] = {
        "OK",
        "Bad structure pointer provided",
        "Error in gTree",
        "Error in gObjPool",
        "Error in file IO",
        "Bad pointer provided",
        "Bad node id provided",
        "WARNING: bad input provided",
        "WARNING: lexemes stack is empty, have you run the lexer?",
        "WARNING: expression tree is empty, have you run the parser?",
        "Parsing error: unknown lexemes sequence",
        "Parsing error: no closing bracket",
        "Some error during tree decomposition",
        "UNKNOWN ERROR CODE!",
    };

#ifndef NLOGS
#define GDERIVATOR_ASSERT_LOG(expr, errCode) ({                                         \
        if (errCode >= gDerivator_status_CNT || errCode < 0)  {                          \
            ASSERT_LOG(false, gDerivator_status_CNT,                                      \
                    gDerivator_statusMsg[gDerivator_status_CNT], context->logStream);      \
        }                                                                                   \
        ASSERT_LOG(expr, errCode, gDerivator_statusMsg[errCode], context->logStream);        \
    })
#else
#define GDERIVATOR_ASSERT_LOG(expr, errCode) ASSERT_LOG(expr, errCode, gDerivator_statusMsg[errCode], NULL)
#endif

#define GDERIVATOR_CHECK_SELF_PTR(ptr) ASSERT_LOG(gPtrValid(ptr), gDerivator_status_BadStructPtr,     \
                                                 gDerivator_statusMsg[gDerivator_status_BadStructPtr], \
                                                 stderr)
#define GDERIVATOR_IS_OK(expr) ({                                                 \
        gDerivator_status macroStatus = (expr);                                    \
        GDERIVATOR_ASSERT_LOG(macroStatus == gDerivator_status_OK, macroStatus);    \
    })

#define GDERIVATOR_NODE_BY_ID(macroId) ({                                                               \
    assert(macroId != -1);                                                                               \
    gTree_Node *macroNode = NULL;                                                                         \
    GDERIVATOR_ASSERT_LOG(gObjPool_get(&context->tree.pool, (macroId), &macroNode) == gObjPool_status_OK,  \
                            gDerivator_status_ObjPoolErr);                                                  \
    assert(gPtrValid(macroNode));                                                                            \
    macroNode;                                                                                                \
})                                                                   


#define GDERIVATOR_POOL_ALLOC() ({                                                                 \
    size_t macroId = -1;                                                                            \
    gTree_Node *macroNode = NULL;                                                                    \
    GDERIVATOR_ASSERT_LOG(gObjPool_alloc(&context->tree.pool, &macroId) == gObjPool_status_OK,        \
                            gDerivator_status_ObjPoolErr);                                             \
    GDERIVATOR_ASSERT_LOG(gObjPool_get(&context->tree.pool, macroId, &macroNode) == gObjPool_status_OK, \
                            gDerivator_status_ObjPoolErr);                                               \
      macroNode->sibling = -1;                                                                            \
      macroNode->parent  = -1;                                                                             \
      macroNode->child   = -1;                                                                              \
      macroId;                                                                                               \
})

#define GDERIVATOR_POOL_FREE(id) ({                                                           \
    GDERIVATOR_ASSERT_LOG(gObjPool_free(&context->tree.pool, id) == gObjPool_status_OK,        \
                            gDerivator_status_ObjPoolErr);                                      \
})


#define GDERIVATOR_TREE_CHECK(expr) ({                                             \
    GDERIVATOR_ASSERT_LOG((expr) == gTree_status_OK, gDerivator_status_TreeErr);    \
})


#define GDERIVATOR_ID_CHECK(id) GDERIVATOR_ASSERT_LOG(gObjPool_idValid(&context->tree.pool, id), gDerivator_status_BadId)


struct gDerivator {
    gTree tree;
    FILE *logStream;
    GENERIC(stack) LexemeIds = {};
    const char *buffer;
} typedef gDerivator;


gDerivator_status gDerivator_dumpNode(gDerivator *context, size_t id);


gDerivator_status gDerivator_ctor(gDerivator *context, FILE *newLogStream);

gDerivator_status gDerivator_dtor(gDerivator *context);


gDerivator_status gDerivator_lexer(gDerivator *context, const char *buffer);


gDerivator_status gDerivator_parser(gDerivator *context);

static gDerivator_status gDerivator_parser_expr (gDerivator *context, const size_t start, const size_t end, const size_t subRoot);

static gDerivator_status gDerivator_parser_prior(gDerivator *context, const size_t start, const size_t end, const size_t subRoot);

static gDerivator_status gDerivator_parser_expn (gDerivator *context, const size_t start, const size_t end, const size_t subRoot);

static gDerivator_status gDerivator_parser_term (gDerivator *context, const size_t start, const size_t end, const size_t subRoot);


gDerivator_status gDerivator_derivate(gDerivator *context, const size_t rootId);


gDerivator_status gDerivator_optimize(gDerivator *context, const size_t rootId);


static gDerivator_status gDerivator_dumpSubtreeLatex(const gDerivator *context, const size_t rootId, FILE *out);

gDerivator_status gDerivator_dumpLatex(const gDerivator *context, FILE *out);

#endif
