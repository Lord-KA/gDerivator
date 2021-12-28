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
    size_t position;
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
    gDerivator_status_EmptyLexer,
    gDerivator_status_ParsingErr_UnknownLex,
    gDerivator_status_ParsingErr_NoBrack,
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
        "WARNING: lexemes stack is empty, have you run lexer?",
        "Parsing error: unknown lexemes sequence",
        "Parsing error: no closing bracket",
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

#define GDERIVATOR_POOL_FREE(id) ({                                                           \
    GDERIVATOR_ASSERT_LOG(gObjPool_free(&context->tree.pool, id) == gObjPool_status_OK,        \
                            gDerivator_status_ObjPoolErr);                                      \
})


#define GDERIVATOR_TREE_CHECK(expr) ({                                             \
    GDERIVATOR_ASSERT_LOG((expr) == gTree_status_OK, gDerivator_status_TreeErr);    \
})

struct gDerivator {
    gTree tree;
    FILE *logStream;
    GENERIC(stack) LexemeIds = {};
    const char *buffer;
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
    GDERIVATOR_ASSERT_LOG(gPtrValid(buffer), gDerivator_status_BadPtr);

    context->buffer = buffer;
    char *cur = (char*)buffer;
    size_t id = -1;
    gDerivator_Node *node = NULL;
    GENERIC(stack_clear)(&context->LexemeIds);

    while (*cur != '\0' && *cur != '\n') {
        #ifdef EXTRA_VERBOSE
            fprintf(stderr, "cur = %s\n", cur);
        #endif  
        if (isspace(*cur)) {
            ++cur;
            continue;
        }
        id   = GDERIVATOR_POOL_ALLOC();
        node = &(GDERIVATOR_NODE_BY_ID(id)->data);
        GENERIC(stack_push)(&context->LexemeIds, id);
        node->position = cur - buffer;

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
            #ifdef EXTRA_VERBOSE
                fprintf(stderr, "Found lit!\n");
                fprintf(stderr, " cur = #%s#\n", cur);
            #endif
            continue;
        }

        bool isFunc = false;                                                                              //TODO switch to Trim alg
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
            #ifdef EXTRA_VERBOSE
                fprintf(stderr, "Found func!");
                fprintf(stderr, " cur = #%s#\n", cur);
            #endif
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

    
    #ifdef EXTRA_VERBOSE
        fprintf(stderr, "LexemeIds = {");
        for (size_t i = 0; i < context->LexemeIds.len; ++i) 
            fprintf(stderr, "%lu, ", context->LexemeIds.data[i]);
        fprintf(stderr, "}\n");
    #endif
    
    return gDerivator_status_OK;
}


gDerivator_status gDerivator_parser_expr (gDerivator *context, size_t start, size_t end, size_t subRoot);        //TODO

gDerivator_status gDerivator_parser_prior(gDerivator *context, size_t start, size_t end, size_t subRoot);

gDerivator_status gDerivator_parser_term (gDerivator *context, size_t start, size_t end, size_t subRoot);




gDerivator_status gDerivator_parser(gDerivator *context)
{ 
    GDERIVATOR_CHECK_SELF_PTR(context);
    size_t len = context->LexemeIds.len;

    GDERIVATOR_ASSERT_LOG(len != 0 && len < 10000, gDerivator_status_EmptyLexer);      //TODO put constant into `static const`

    return gDerivator_parser_expr(context, 0, len, context->tree.root);
}


gDerivator_status gDerivator_parser_expr(gDerivator *context, size_t start, size_t end, size_t subRoot)
{
    GDERIVATOR_CHECK_SELF_PTR(context);
    assert(start <  context->LexemeIds.len);
    assert(end   <= context->LexemeIds.len);

    gDerivator_status status = gDerivator_status_OK;

    size_t subStart = start, subEnd = -1;
    size_t *data = context->LexemeIds.data;
    long brackCnt = 0;
    for (subEnd = start; subEnd < end; ++subEnd) {
        gDerivator_Node *node = &(GDERIVATOR_NODE_BY_ID(data[subEnd])->data);
        if (node->mode == gDerivator_Node_mode_opBrack) 
            ++brackCnt;
        if (node->mode == gDerivator_Node_mode_clBrack) 
            --brackCnt;
        if ((node->mode == gDerivator_Node_mode_sum || node->mode == gDerivator_Node_mode_sub) && brackCnt == 0) {
            GDERIVATOR_TREE_CHECK(gTree_addExistChild(&context->tree, subRoot, data[subEnd]));
            status = gDerivator_parser_term(context, subStart, subEnd, data[subEnd]);
            GDERIVATOR_ASSERT_LOG(status == gDerivator_status_OK, status);
            subStart = subEnd + 1;
            status = gDerivator_parser_expr(context, subStart, end, data[subEnd]);
            return status;
        }
    }
    status = gDerivator_parser_term(context, start, end, subRoot);
    return status;
}

gDerivator_status gDerivator_parser_term(gDerivator *context, size_t start, size_t end, size_t subRoot)
{
    GDERIVATOR_CHECK_SELF_PTR(context);
    assert(start <  context->LexemeIds.len);
    assert(end   <= context->LexemeIds.len);

    gDerivator_status status = gDerivator_status_OK;

    size_t subStart = start, subEnd = -1;
    size_t *data = context->LexemeIds.data;
    long brackCnt = 0;
    for (subEnd = start; subEnd < end; ++subEnd) {
        gDerivator_Node *node = &(GDERIVATOR_NODE_BY_ID(data[subEnd])->data);
        if (node->mode == gDerivator_Node_mode_opBrack) 
            ++brackCnt;
        if (node->mode == gDerivator_Node_mode_clBrack) 
            --brackCnt;
        if ((node->mode == gDerivator_Node_mode_mul || node->mode == gDerivator_Node_mode_div) && brackCnt == 0) {
            GDERIVATOR_TREE_CHECK(gTree_addExistChild(&context->tree, subRoot, data[subEnd]));
            status = gDerivator_parser_prior(context, subStart, subEnd, data[subEnd]);
            GDERIVATOR_ASSERT_LOG(status == gDerivator_status_OK, status);
            subStart = subEnd + 1;
            status = gDerivator_parser_expr(context, subStart, end, data[subEnd]);
            return status;
        }
    }
    status = gDerivator_parser_prior(context, start, end, subRoot);
    return status;
}

gDerivator_status gDerivator_parser_prior(gDerivator *context, size_t start, size_t end, size_t subRoot)
{
    GDERIVATOR_CHECK_SELF_PTR(context);
    assert(start <  context->LexemeIds.len);
    assert(end   <= context->LexemeIds.len);

    gDerivator_status status = gDerivator_status_OK;

    size_t *data = context->LexemeIds.data;
    if (GDERIVATOR_NODE_BY_ID(data[start])->data.mode == gDerivator_Node_mode_opBrack) {            
        GDERIVATOR_ASSERT_LOG(GDERIVATOR_NODE_BY_ID(data[end - 1])->data.mode == gDerivator_Node_mode_clBrack,
                                    gDerivator_status_ParsingErr_NoBrack);
        status = gDerivator_parser_expr(context, start + 1, end - 1, subRoot);
        GDERIVATOR_POOL_FREE(data[start]);
        GDERIVATOR_POOL_FREE(data[end - 1]);

    } else if (end - start == 1) {
        GDERIVATOR_TREE_CHECK(gTree_addExistChild(&context->tree, subRoot, data[start]));

    } else if (GDERIVATOR_NODE_BY_ID(data[start])->data.mode == gDerivator_Node_mode_func) {
        GDERIVATOR_TREE_CHECK(gTree_addExistChild(&context->tree, subRoot, data[start]));
        status = gDerivator_parser_prior(context, start + 1, end, data[start]);

    } else {
        GDERIVATOR_ASSERT_LOG(false, gDerivator_status_ParsingErr_UnknownLex);
    }
    return status;
}

