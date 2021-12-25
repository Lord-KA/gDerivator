#pragma once

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
    gDerivator_Node_func_CNT,
};

static const char gDerivator_Node_funcView[gDerivator_Node_func_CNT][10] = {
        "sin",
        "cos"
    };

struct gDerivator_Node 
{
    gDerivator_Node_mode mode;
    gDerivator_Node_func func;
    double value;
} typedef gDerivator_Node;

static const char GDERIVATOR_DELIMS_LIST[] = "[]+-*/() ";

typedef gDerivator_Node GTREE_TYPE;

#include "gtree.h"

static const size_t MAX_LINE_LEN = 1000;
static const size_t GDERIVATOR_MAX_LIT_LEN = 100;

bool gTree_storeData(gDerivator_Node data, size_t level, FILE *out) //TODO
{
    return 0;
}

bool gTree_restoreData(gDerivator_Node *data, FILE *in) //TODO
{
    return 0;
}

bool gTree_printData(gDerivator_Node data, FILE *out)
{
    assert(gPtrValid(out));
    if (data.mode >= gDerivator_Node_mode_CNT || data.mode < 0)
        data.mode = gDerivator_Node_mode_unknown;
    fprintf(out, "{mode | %d (\'%s\')}", data.mode, gDerivator_Node_modeView[data.mode]);

    if (data.mode == gDerivator_Node_mode_num)
        fprintf(out, "| {%lf} ", data.value);
    else if (data.mode == gDerivator_Node_mode_func)
        fprintf(out, "| {\'%s\'} ", gDerivator_Node_funcView[data.func]);

    return 0;
}

enum gDerivator_status {
    gDerivator_status_OK,  
    gDerivator_status_BadStructPtr,
    gDerivator_status_TreeErr,
    gDerivator_status_ObjPoolErr,
    gDerivator_status_FileErr,
    gDerivator_status_BadPtr,
    gDerivator_status_BadId,
    gDerivator_status_BadInput,
    gDerivator_status_CNT,
};

static const char gDerivator_statusMsg[gDerivator_status_CNT][MAX_LINE_LEN] = {
        "OK",
        "Bad structure pointer provided",
        "Error in gTree",
        "Error in gObjPool",
        "Error in file IO",
        "Bad pointer provided",
        "Bad node id provided",
        "WARNING: bad input provided",
    };

#ifndef NLOGS
#define GDERIVATOR_ASSERT_LOG(expr, errCode) ASSERT_LOG(expr, errCode, gDerivator_statusMsg[errCode], context->logStream)
#else
#define GDERIVATOR_ASSERT_LOG(expr, errCode) ASSERT_LOG(expr, errCode, gDerivator_statusMsg[errCode], NULL)
#endif

#define GDERIVATOR_CHECK_SELF_PTR(ptr) ASSERT_LOG(gPtrValid(ptr), gDerivator_status_BadStructPtr,     \
                                                 gDerivator_statusMsg[gDerivator_status_BadStructPtr], \
                                                 stderr)


#define GDERIVATOR_NODE_BY_ID(id) ({                                                          \
    assert(id != -1);                                                                          \
    gTree_Node *node = NULL;                                                                    \
    GDERIVATOR_ASSERT_LOG(gObjPool_get(&context->tree.pool, (id), &node) == gObjPool_status_OK,  \
                            gDerivator_status_ObjPoolErr);                                        \
    assert(gPtrValid(node));                                                                       \
    node;                                                                                           \
})                                                                   


#define GDERIVATOR_POOL_ALLOC() ({                                                            \
    size_t id = -1;                                                                            \
    gTree_Node *node = NULL;                                                                    \
    GDERIVATOR_ASSERT_LOG(gObjPool_alloc(&context->tree.pool, &id) == gObjPool_status_OK,        \
                            gDerivator_status_ObjPoolErr);                                        \
    id;                                                                                            \
})

struct gDerivator {
    gTree tree;
    FILE *logStream;
    GENERIC(stack) LexemeIds = {};
} typedef gDerivator;


gDerivator_status gDerivator_ctor(gDerivator *context, FILE *newLogStream)
{
    if (!gPtrValid(context)) {                                          
        FILE *out;                                                   
        if (!gPtrValid(newLogStream)) 
            out = stderr;                                            
        else                                                         
            out = newLogStream;                                      
        fprintf(out, "ERROR: bad structure ptr provided to derivator ctor!\n");
        return gDerivator_status_BadStructPtr;                         
    }
    
    context->logStream = stderr;
    if (gPtrValid(newLogStream))
        context->logStream = newLogStream;

    gTree_status status = gTree_ctor(&context->tree, context->logStream);
    GDERIVATOR_ASSERT_LOG(status == gTree_status_OK, gDerivator_status_TreeErr);

    GENERIC(stack_ctor)(&context->LexemeIds);

    return gDerivator_status_OK;
}

gDerivator_status gDerivator_dtor(gDerivator *context)
{
    GDERIVATOR_CHECK_SELF_PTR(context);

    gTree_status status = gTree_dtor(&context->tree);
    GDERIVATOR_ASSERT_LOG(status == gTree_status_OK, gDerivator_status_TreeErr);

    GENERIC(stack_dtor)(&context->LexemeIds);

    return gDerivator_status_OK;
}

gDerivator_status gDerivator_lexer(gDerivator *context, const char *buffer)
{
    GDERIVATOR_CHECK_SELF_PTR(context);

    char *cur = (char*)buffer;
    size_t id = -1;
    gDerivator_Node *node = NULL;
    GENERIC(stack_clear)(&context->LexemeIds);

    while (*cur != '\0' && *cur != '\n') {      //TODO add gArray so objPool could be used repeatedly
        fprintf(stderr, "cur = %s\n", cur);
        if (isspace(*cur)) {
            ++cur;
            continue;
        }
        id   = GDERIVATOR_POOL_ALLOC();
        node = &(GDERIVATOR_NODE_BY_ID(id)->data);
        GENERIC(stack_push)(&context->LexemeIds, id);

        bool foundLit = false;
        for (size_t i = gDerivator_Node_mode_sum; i <= gDerivator_Node_mode_var; ++i) {                 //TODO make vars universal
            if (*gDerivator_Node_modeView[i] == *cur) {
                node->mode = (gDerivator_Node_mode)i;
                foundLit = true;
                ++cur;
                break;
            }
        }
        if (foundLit) {
            fprintf(stderr, "Found lit!\n");
            fprintf(stderr, " cur = #%s#\n", cur);
            continue;
        }

        bool isFunc = false;
        for (size_t i = 0; i < gDerivator_Node_func_CNT; ++i) {
            if (strncmp(cur, gDerivator_Node_funcView[i], strlen(gDerivator_Node_funcView[i])) == 0) {  //TODO check if '\n' is a valid problem
                cur += strlen(gDerivator_Node_funcView[i]);
                node->mode = gDerivator_Node_mode_func;
                node->func = (gDerivator_Node_func)i;
                isFunc = true;
                break;
            }
        }
        if (isFunc && *cur != '\0' && *cur != '\n' && !strnConsistsChrs(cur, GDERIVATOR_DELIMS_LIST, 1, strlen(GDERIVATOR_DELIMS_LIST))) {
            node->mode = gDerivator_Node_mode_unknown;
            fprintf(stderr, "ERROR: Unknown word has been found in func!\n");
            ++cur;
        }
        if (isFunc) {
            fprintf(stderr, "Found func!");
            fprintf(stderr, " cur = #%s#\n", cur);
            continue;
        }

        char *litEnd = cur;
        while (!strnConsistsChrs(litEnd, GDERIVATOR_DELIMS_LIST, 1, strlen(GDERIVATOR_DELIMS_LIST)) && *litEnd != '\0' && *litEnd != '\n') 
            ++litEnd;
        char literal[GDERIVATOR_MAX_LIT_LEN] = "";
        strncpy(literal, cur, litEnd - cur);
        if (isInteger(literal)) {
            node->mode = gDerivator_Node_mode_num;
            node->value = (double)(strtol(literal, NULL, 0));
            cur = litEnd;
            continue;
        } else if (isDouble(literal)) {
            node->mode = gDerivator_Node_mode_num;
            node->value = strtod(literal, NULL);
            cur = litEnd;
            continue;
        }
        node->mode = gDerivator_Node_mode_unknown;
        fprintf(stderr, "ERROR: Unknown word has been found!\n");
        *cur;
        ++cur;
    }

    fprintf(stderr, "LexemeIds = {");
    for (size_t i = 0; i < context->LexemeIds.len; ++i) 
        fprintf(stderr, "%lu, ", context->LexemeIds.data[i]);
    fprintf(stderr, "}\n");
    
    return gDerivator_status_OK;
}

