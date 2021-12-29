# 0 "./gderivator.cpp"
# 0 "<built-in>"
# 0 "<command-line>"
# 1 "/usr/include/stdc-predef.h" 1 3 4
# 0 "<command-line>" 2
# 1 "./gderivator.cpp"
# 1 "./gderivator.h" 1
       







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



static const size_t MAX_LINE_LEN = 1000;
static const size_t GDERIVATOR_MAX_LIT_LEN = 100;
static const size_t GDERIVATOR_LEX_LIM = 1000;

bool gTree_storeData(gDerivator_Node data, size_t level, FILE *out)
{
    return 0;
}

bool gTree_restoreData(gDerivator_Node *data, FILE *in)
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
    gDerivator_status_EmptyTree,
    gDerivator_status_ParsingErr_UnknownLex,
    gDerivator_status_ParsingErr_NoBrack,
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
        "UNKNOWN ERROR CODE!",
    };
# 180 "./gderivator.h"
struct gDerivator {
    gTree tree;
    FILE *logStream;
    GENERIC(stack) LexemeIds = {};
    const char *buffer;
} typedef gDerivator;


static gDerivator_status gDerivator_ctor(gDerivator *context, FILE *newLogStream)
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
    ({ if (gDerivator_status_TreeErr >= gDerivator_status_CNT || gDerivator_status_TreeErr < 0) { fprintf(context->logStream, "UNKNOWN STATUS CODE = %d\n", gDerivator_status_TreeErr); ASSERT_LOG(false, gDerivator_status_CNT, gDerivator_statusMsg[gDerivator_status_CNT], context->logStream); } ASSERT_LOG(status == gTree_status_OK, gDerivator_status_TreeErr, gDerivator_statusMsg[gDerivator_status_TreeErr], context->logStream); });

    GENERIC(stack_ctor)(&context->LexemeIds);

    return gDerivator_status_OK;
}

static gDerivator_status gDerivator_dtor(gDerivator *context)
{
    ASSERT_LOG(gPtrValid(context), gDerivator_status_BadStructPtr, gDerivator_statusMsg[gDerivator_status_BadStructPtr], stderr);

    gTree_status status = gTree_dtor(&context->tree);
    ({ if (gDerivator_status_TreeErr >= gDerivator_status_CNT || gDerivator_status_TreeErr < 0) { fprintf(context->logStream, "UNKNOWN STATUS CODE = %d\n", gDerivator_status_TreeErr); ASSERT_LOG(false, gDerivator_status_CNT, gDerivator_statusMsg[gDerivator_status_CNT], context->logStream); } ASSERT_LOG(status == gTree_status_OK, gDerivator_status_TreeErr, gDerivator_statusMsg[gDerivator_status_TreeErr], context->logStream); });

    GENERIC(stack_dtor)(&context->LexemeIds);

    return gDerivator_status_OK;
}

static gDerivator_status gDerivator_lexer(gDerivator *context, const char *buffer)
{
    ASSERT_LOG(gPtrValid(context), gDerivator_status_BadStructPtr, gDerivator_statusMsg[gDerivator_status_BadStructPtr], stderr);
    ({ if (gDerivator_status_BadPtr >= gDerivator_status_CNT || gDerivator_status_BadPtr < 0) { fprintf(context->logStream, "UNKNOWN STATUS CODE = %d\n", gDerivator_status_BadPtr); ASSERT_LOG(false, gDerivator_status_CNT, gDerivator_statusMsg[gDerivator_status_CNT], context->logStream); } ASSERT_LOG(gPtrValid(buffer), gDerivator_status_BadPtr, gDerivator_statusMsg[gDerivator_status_BadPtr], context->logStream); });

    context->buffer = buffer;
    char *cur = (char*)buffer;
    size_t id = -1;
    gDerivator_Node *node = NULL;
    GENERIC(stack_clear)(&context->LexemeIds);

    while (*cur != '\0' && *cur != '\n') {



        if (isspace(*cur)) {
            ++cur;
            continue;
        }
        id = ({ size_t id = -1; gTree_Node *node = NULL; ({ if (gDerivator_status_ObjPoolErr >= gDerivator_status_CNT || gDerivator_status_ObjPoolErr < 0) { fprintf(context->logStream, "UNKNOWN STATUS CODE = %d\n", gDerivator_status_ObjPoolErr); ASSERT_LOG(false, gDerivator_status_CNT, gDerivator_statusMsg[gDerivator_status_CNT], context->logStream); } ASSERT_LOG(gObjPool_alloc(&context->tree.pool, &id) == gObjPool_status_OK, gDerivator_status_ObjPoolErr, gDerivator_statusMsg[gDerivator_status_ObjPoolErr], context->logStream); }); id; });
        node = &(({ assert(id != -1); gTree_Node *node = NULL; ({ if (gDerivator_status_ObjPoolErr >= gDerivator_status_CNT || gDerivator_status_ObjPoolErr < 0) { fprintf(context->logStream, "UNKNOWN STATUS CODE = %d\n", gDerivator_status_ObjPoolErr); ASSERT_LOG(false, gDerivator_status_CNT, gDerivator_statusMsg[gDerivator_status_CNT], context->logStream); } ASSERT_LOG(gObjPool_get(&context->tree.pool, (id), &node) == gObjPool_status_OK, gDerivator_status_ObjPoolErr, gDerivator_statusMsg[gDerivator_status_ObjPoolErr], context->logStream); }); assert(gPtrValid(node)); node; })->data);
        GENERIC(stack_push)(&context->LexemeIds, id);
        node->position = cur - buffer;

        bool foundLit = false;
        for (size_t i = gDerivator_Node_mode_sum; i <= gDerivator_Node_mode_var; ++i) {
            if (*gDerivator_Node_modeView[i] == *cur) {
                node->mode = (gDerivator_Node_mode)i;
                foundLit = true;
                ++cur;
                break;
            }
        }
        if (foundLit) {




            continue;
        }

        bool isFunc = false;
        for (size_t i = 0; i < gDerivator_Node_func_CNT; ++i) {
            if (strncmp(cur, gDerivator_Node_funcView[i], strlen(gDerivator_Node_funcView[i])) == 0) {
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
# 318 "./gderivator.h"
    return gDerivator_status_OK;
}


static gDerivator_status gDerivator_parser_expr (gDerivator *context, const size_t start, const size_t end, const size_t subRoot);

static gDerivator_status gDerivator_parser_prior(gDerivator *context, const size_t start, const size_t end, const size_t subRoot);

static gDerivator_status gDerivator_parser_term (gDerivator *context, const size_t start, const size_t end, const size_t subRoot);


static gDerivator_status gDerivator_parser(gDerivator *context)
{
    ASSERT_LOG(gPtrValid(context), gDerivator_status_BadStructPtr, gDerivator_statusMsg[gDerivator_status_BadStructPtr], stderr);
    size_t len = context->LexemeIds.len;

    ({ if (gDerivator_status_EmptyLexer >= gDerivator_status_CNT || gDerivator_status_EmptyLexer < 0) { fprintf(context->logStream, "UNKNOWN STATUS CODE = %d\n", gDerivator_status_EmptyLexer); ASSERT_LOG(false, gDerivator_status_CNT, gDerivator_statusMsg[gDerivator_status_CNT], context->logStream); } ASSERT_LOG(len != 0 && len < GDERIVATOR_LEX_LIM, gDerivator_status_EmptyLexer, gDerivator_statusMsg[gDerivator_status_EmptyLexer], context->logStream); });

    return gDerivator_parser_expr(context, 0, len, context->tree.root);
}


static gDerivator_status gDerivator_parser_expr(gDerivator *context,
                                                const size_t start,
                                                const size_t end,
                                                const size_t subRoot)
{
    ASSERT_LOG(gPtrValid(context), gDerivator_status_BadStructPtr, gDerivator_statusMsg[gDerivator_status_BadStructPtr], stderr);
    assert(start < context->LexemeIds.len);
    assert(end <= context->LexemeIds.len);
    assert(start <= end);

    gDerivator_status status = gDerivator_status_OK;

    size_t subStart = -1, subEnd = -1;
    size_t *data = context->LexemeIds.data;
    long brackCnt = 0;

    size_t firstPos = -1;
    for (size_t i = start; i < end; ++i) {
        gDerivator_Node *node = &(({ assert(data[i] != -1); gTree_Node *node = NULL; ({ if (gDerivator_status_ObjPoolErr >= gDerivator_status_CNT || gDerivator_status_ObjPoolErr < 0) { fprintf(context->logStream, "UNKNOWN STATUS CODE = %d\n", gDerivator_status_ObjPoolErr); ASSERT_LOG(false, gDerivator_status_CNT, gDerivator_statusMsg[gDerivator_status_CNT], context->logStream); } ASSERT_LOG(gObjPool_get(&context->tree.pool, (data[i]), &node) == gObjPool_status_OK, gDerivator_status_ObjPoolErr, gDerivator_statusMsg[gDerivator_status_ObjPoolErr], context->logStream); }); assert(gPtrValid(node)); node; })->data);
        if (node->mode == gDerivator_Node_mode_opBrack)
            ++brackCnt;
        if (node->mode == gDerivator_Node_mode_clBrack)
            --brackCnt;

        if ((node->mode == gDerivator_Node_mode_sum || node->mode == gDerivator_Node_mode_sub) && brackCnt == 0) {
            if (firstPos == -1)
                firstPos = i;
            else
                firstPos = -2;
        }
    }




    if (firstPos == -1) {
        return gDerivator_parser_term(context, start, end, subRoot);
    } else if (firstPos != -2) {
        ({ ({ if (gDerivator_status_TreeErr >= gDerivator_status_CNT || gDerivator_status_TreeErr < 0) { fprintf(context->logStream, "UNKNOWN STATUS CODE = %d\n", gDerivator_status_TreeErr); ASSERT_LOG(false, gDerivator_status_CNT, gDerivator_statusMsg[gDerivator_status_CNT], context->logStream); } ASSERT_LOG((gTree_addExistChild(&context->tree, subRoot, data[firstPos])) == gTree_status_OK, gDerivator_status_TreeErr, gDerivator_statusMsg[gDerivator_status_TreeErr], context->logStream); }); });

        status = gDerivator_parser_term(context, start, firstPos, data[firstPos]);
        ({ gDerivator_status status = (status); fprintf(stderr, "STATUS IN IS_OK = %d\n", status); ({ if (status >= gDerivator_status_CNT || status < 0) { fprintf(context->logStream, "UNKNOWN STATUS CODE = %d\n", status); ASSERT_LOG(false, gDerivator_status_CNT, gDerivator_statusMsg[gDerivator_status_CNT], context->logStream); } ASSERT_LOG(status == gDerivator_status_OK, status, gDerivator_statusMsg[status], context->logStream); }); });
        status = gDerivator_parser_term(context, firstPos + 1, end, data[firstPos]);
        ({ gDerivator_status status = (status); fprintf(stderr, "STATUS IN IS_OK = %d\n", status); ({ if (status >= gDerivator_status_CNT || status < 0) { fprintf(context->logStream, "UNKNOWN STATUS CODE = %d\n", status); ASSERT_LOG(false, gDerivator_status_CNT, gDerivator_statusMsg[gDerivator_status_CNT], context->logStream); } ASSERT_LOG(status == gDerivator_status_OK, status, gDerivator_statusMsg[status], context->logStream); }); });
        return status;
    }

    size_t sumRoot = -1;
    bool isSum = true;
    ({ ({ if (gDerivator_status_TreeErr >= gDerivator_status_CNT || gDerivator_status_TreeErr < 0) { fprintf(context->logStream, "UNKNOWN STATUS CODE = %d\n", gDerivator_status_TreeErr); ASSERT_LOG(false, gDerivator_status_CNT, gDerivator_statusMsg[gDerivator_status_CNT], context->logStream); } ASSERT_LOG((gTree_addChild(&context->tree, subRoot, &sumRoot, gDerivator_Node{gDerivator_Node_mode_sum, gDerivator_Node_func_CNT, 0, 0})) == gTree_status_OK, gDerivator_status_TreeErr, gDerivator_statusMsg[gDerivator_status_TreeErr], context->logStream); }); })
                                                                                           ;
    brackCnt = 0;
    for (subStart = subEnd = start; subEnd < end; ++subEnd) {
        gDerivator_Node *node = &(({ assert(data[subEnd] != -1); gTree_Node *node = NULL; ({ if (gDerivator_status_ObjPoolErr >= gDerivator_status_CNT || gDerivator_status_ObjPoolErr < 0) { fprintf(context->logStream, "UNKNOWN STATUS CODE = %d\n", gDerivator_status_ObjPoolErr); ASSERT_LOG(false, gDerivator_status_CNT, gDerivator_statusMsg[gDerivator_status_CNT], context->logStream); } ASSERT_LOG(gObjPool_get(&context->tree.pool, (data[subEnd]), &node) == gObjPool_status_OK, gDerivator_status_ObjPoolErr, gDerivator_statusMsg[gDerivator_status_ObjPoolErr], context->logStream); }); assert(gPtrValid(node)); node; })->data);
        if (node->mode == gDerivator_Node_mode_opBrack)
            ++brackCnt;
        if (node->mode == gDerivator_Node_mode_clBrack)
            --brackCnt;

        if ((node->mode == gDerivator_Node_mode_sum || node->mode == gDerivator_Node_mode_sub) && brackCnt == 0) {



            if (isSum) {
                status = gDerivator_parser_term(context, subStart, subEnd, sumRoot);
            } else {
                size_t mulRoot = -1;
                ({ ({ if (gDerivator_status_TreeErr >= gDerivator_status_CNT || gDerivator_status_TreeErr < 0) { fprintf(context->logStream, "UNKNOWN STATUS CODE = %d\n", gDerivator_status_TreeErr); ASSERT_LOG(false, gDerivator_status_CNT, gDerivator_statusMsg[gDerivator_status_CNT], context->logStream); } ASSERT_LOG((gTree_addChild(&context->tree, sumRoot, &mulRoot, gDerivator_Node{gDerivator_Node_mode_mul, gDerivator_Node_func_CNT, 0, 0})) == gTree_status_OK, gDerivator_status_TreeErr, gDerivator_statusMsg[gDerivator_status_TreeErr], context->logStream); }); })
                                                                                                   ;

                ({ ({ if (gDerivator_status_TreeErr >= gDerivator_status_CNT || gDerivator_status_TreeErr < 0) { fprintf(context->logStream, "UNKNOWN STATUS CODE = %d\n", gDerivator_status_TreeErr); ASSERT_LOG(false, gDerivator_status_CNT, gDerivator_statusMsg[gDerivator_status_CNT], context->logStream); } ASSERT_LOG((gTree_addChild(&context->tree, mulRoot, NULL, gDerivator_Node{gDerivator_Node_mode_num, gDerivator_Node_func_CNT, -1, 0})) == gTree_status_OK, gDerivator_status_TreeErr, gDerivator_statusMsg[gDerivator_status_TreeErr], context->logStream); }); })
                                                                                                    ;

                status = gDerivator_parser_term(context, subStart, subEnd, mulRoot);
            }
            ({ gDerivator_status status = (status); fprintf(stderr, "STATUS IN IS_OK = %d\n", status); ({ if (status >= gDerivator_status_CNT || status < 0) { fprintf(context->logStream, "UNKNOWN STATUS CODE = %d\n", status); ASSERT_LOG(false, gDerivator_status_CNT, gDerivator_statusMsg[gDerivator_status_CNT], context->logStream); } ASSERT_LOG(status == gDerivator_status_OK, status, gDerivator_statusMsg[status], context->logStream); }); });
            isSum = (node->mode == gDerivator_Node_mode_sum);
            ({ ({ if (gDerivator_status_ObjPoolErr >= gDerivator_status_CNT || gDerivator_status_ObjPoolErr < 0) { fprintf(context->logStream, "UNKNOWN STATUS CODE = %d\n", gDerivator_status_ObjPoolErr); ASSERT_LOG(false, gDerivator_status_CNT, gDerivator_statusMsg[gDerivator_status_CNT], context->logStream); } ASSERT_LOG(gObjPool_free(&context->tree.pool, data[subEnd]) == gObjPool_status_OK, gDerivator_status_ObjPoolErr, gDerivator_statusMsg[gDerivator_status_ObjPoolErr], context->logStream); }); });
            ++subEnd;
            subStart = subEnd;
        }
    }
    if (isSum) {
        status = gDerivator_parser_term(context, subStart, subEnd, sumRoot);
    } else {
        size_t mulRoot = -1;
        ({ ({ if (gDerivator_status_TreeErr >= gDerivator_status_CNT || gDerivator_status_TreeErr < 0) { fprintf(context->logStream, "UNKNOWN STATUS CODE = %d\n", gDerivator_status_TreeErr); ASSERT_LOG(false, gDerivator_status_CNT, gDerivator_statusMsg[gDerivator_status_CNT], context->logStream); } ASSERT_LOG((gTree_addChild(&context->tree, sumRoot, &mulRoot, gDerivator_Node{gDerivator_Node_mode_mul, gDerivator_Node_func_CNT, 0, 0})) == gTree_status_OK, gDerivator_status_TreeErr, gDerivator_statusMsg[gDerivator_status_TreeErr], context->logStream); }); })
                                                                                           ;

        ({ ({ if (gDerivator_status_TreeErr >= gDerivator_status_CNT || gDerivator_status_TreeErr < 0) { fprintf(context->logStream, "UNKNOWN STATUS CODE = %d\n", gDerivator_status_TreeErr); ASSERT_LOG(false, gDerivator_status_CNT, gDerivator_statusMsg[gDerivator_status_CNT], context->logStream); } ASSERT_LOG((gTree_addChild(&context->tree, mulRoot, NULL, gDerivator_Node{gDerivator_Node_mode_num, gDerivator_Node_func_CNT, -1, 0})) == gTree_status_OK, gDerivator_status_TreeErr, gDerivator_statusMsg[gDerivator_status_TreeErr], context->logStream); }); })
                                                                                            ;

        status = gDerivator_parser_term(context, subStart, subEnd, mulRoot);
    }

    return status;
}

static gDerivator_status gDerivator_parser_term(gDerivator *context,
                                                const size_t start,
                                                const size_t end,
                                                const size_t subRoot)
{
    ASSERT_LOG(gPtrValid(context), gDerivator_status_BadStructPtr, gDerivator_statusMsg[gDerivator_status_BadStructPtr], stderr);
    assert(start < context->LexemeIds.len);
    assert(end <= context->LexemeIds.len);
    assert(start <= end);

    gDerivator_status status = gDerivator_status_OK;

    size_t subStart = -1, subEnd = -1;
    size_t *data = context->LexemeIds.data;
    long brackCnt = 0;

    size_t firstPos = -1;
    for (size_t i = start; i < end; ++i) {
        gDerivator_Node *node = &(({ assert(data[i] != -1); gTree_Node *node = NULL; ({ if (gDerivator_status_ObjPoolErr >= gDerivator_status_CNT || gDerivator_status_ObjPoolErr < 0) { fprintf(context->logStream, "UNKNOWN STATUS CODE = %d\n", gDerivator_status_ObjPoolErr); ASSERT_LOG(false, gDerivator_status_CNT, gDerivator_statusMsg[gDerivator_status_CNT], context->logStream); } ASSERT_LOG(gObjPool_get(&context->tree.pool, (data[i]), &node) == gObjPool_status_OK, gDerivator_status_ObjPoolErr, gDerivator_statusMsg[gDerivator_status_ObjPoolErr], context->logStream); }); assert(gPtrValid(node)); node; })->data);
        if (node->mode == gDerivator_Node_mode_opBrack)
            ++brackCnt;
        if (node->mode == gDerivator_Node_mode_clBrack)
            --brackCnt;

        if ((node->mode == gDerivator_Node_mode_mul || node->mode == gDerivator_Node_mode_div) && brackCnt == 0) {
            if (firstPos == -1)
                firstPos = i;
            else
                firstPos = -2;
        }
    }




    if (firstPos == -1) {
        return gDerivator_parser_prior(context, start, end, subRoot);
    } else if (firstPos != -2) {
        ({ ({ if (gDerivator_status_TreeErr >= gDerivator_status_CNT || gDerivator_status_TreeErr < 0) { fprintf(context->logStream, "UNKNOWN STATUS CODE = %d\n", gDerivator_status_TreeErr); ASSERT_LOG(false, gDerivator_status_CNT, gDerivator_statusMsg[gDerivator_status_CNT], context->logStream); } ASSERT_LOG((gTree_addExistChild(&context->tree, subRoot, data[firstPos])) == gTree_status_OK, gDerivator_status_TreeErr, gDerivator_statusMsg[gDerivator_status_TreeErr], context->logStream); }); });

        status = gDerivator_parser_prior(context, start, firstPos, data[firstPos]);
        fprintf(stderr, "\t\tstatus = %d\n", status);
        ({ if (status >= gDerivator_status_CNT || status < 0) { fprintf(context->logStream, "UNKNOWN STATUS CODE = %d\n", status); ASSERT_LOG(false, gDerivator_status_CNT, gDerivator_statusMsg[gDerivator_status_CNT], context->logStream); } ASSERT_LOG(!status, status, gDerivator_statusMsg[status], context->logStream); });

        status = gDerivator_parser_prior(context, firstPos + 1, end, data[firstPos]);
        fprintf(stderr, "\t\tstatus = %d\n", status);

        ({ gDerivator_status status = (status); fprintf(stderr, "STATUS IN IS_OK = %d\n", status); ({ if (status >= gDerivator_status_CNT || status < 0) { fprintf(context->logStream, "UNKNOWN STATUS CODE = %d\n", status); ASSERT_LOG(false, gDerivator_status_CNT, gDerivator_statusMsg[gDerivator_status_CNT], context->logStream); } ASSERT_LOG(status == gDerivator_status_OK, status, gDerivator_statusMsg[status], context->logStream); }); });
        return status;
    }

    size_t mulRoot = -1;
    bool isMul = true;
    ({ ({ if (gDerivator_status_TreeErr >= gDerivator_status_CNT || gDerivator_status_TreeErr < 0) { fprintf(context->logStream, "UNKNOWN STATUS CODE = %d\n", gDerivator_status_TreeErr); ASSERT_LOG(false, gDerivator_status_CNT, gDerivator_statusMsg[gDerivator_status_CNT], context->logStream); } ASSERT_LOG((gTree_addChild(&context->tree, subRoot, &mulRoot, gDerivator_Node{gDerivator_Node_mode_mul, gDerivator_Node_func_CNT, 0, 0})) == gTree_status_OK, gDerivator_status_TreeErr, gDerivator_statusMsg[gDerivator_status_TreeErr], context->logStream); }); })
                                                                                           ;
    brackCnt = 0;
    for (subStart = subEnd = start; subEnd < end; ++subEnd) {
        gDerivator_Node *node = &(({ assert(data[subEnd] != -1); gTree_Node *node = NULL; ({ if (gDerivator_status_ObjPoolErr >= gDerivator_status_CNT || gDerivator_status_ObjPoolErr < 0) { fprintf(context->logStream, "UNKNOWN STATUS CODE = %d\n", gDerivator_status_ObjPoolErr); ASSERT_LOG(false, gDerivator_status_CNT, gDerivator_statusMsg[gDerivator_status_CNT], context->logStream); } ASSERT_LOG(gObjPool_get(&context->tree.pool, (data[subEnd]), &node) == gObjPool_status_OK, gDerivator_status_ObjPoolErr, gDerivator_statusMsg[gDerivator_status_ObjPoolErr], context->logStream); }); assert(gPtrValid(node)); node; })->data);
        if (node->mode == gDerivator_Node_mode_opBrack)
            ++brackCnt;
        if (node->mode == gDerivator_Node_mode_clBrack)
            --brackCnt;

        if ((node->mode == gDerivator_Node_mode_mul || node->mode == gDerivator_Node_mode_div) && brackCnt == 0) {



            if (isMul) {
                status = gDerivator_parser_prior(context, subStart, subEnd, mulRoot);
                ({ gDerivator_status status = (status); fprintf(stderr, "STATUS IN IS_OK = %d\n", status); ({ if (status >= gDerivator_status_CNT || status < 0) { fprintf(context->logStream, "UNKNOWN STATUS CODE = %d\n", status); ASSERT_LOG(false, gDerivator_status_CNT, gDerivator_statusMsg[gDerivator_status_CNT], context->logStream); } ASSERT_LOG(status == gDerivator_status_OK, status, gDerivator_statusMsg[status], context->logStream); }); });
            } else {
                size_t expRoot = -1;
                ({ ({ if (gDerivator_status_TreeErr >= gDerivator_status_CNT || gDerivator_status_TreeErr < 0) { fprintf(context->logStream, "UNKNOWN STATUS CODE = %d\n", gDerivator_status_TreeErr); ASSERT_LOG(false, gDerivator_status_CNT, gDerivator_statusMsg[gDerivator_status_CNT], context->logStream); } ASSERT_LOG((gTree_addChild(&context->tree, mulRoot, &expRoot, gDerivator_Node{gDerivator_Node_mode_exp, gDerivator_Node_func_CNT, 0, 0})) == gTree_status_OK, gDerivator_status_TreeErr, gDerivator_statusMsg[gDerivator_status_TreeErr], context->logStream); }); })
                                                                                                   ;

                status = gDerivator_parser_prior(context, subStart, subEnd, expRoot);
                ({ gDerivator_status status = (status); fprintf(stderr, "STATUS IN IS_OK = %d\n", status); ({ if (status >= gDerivator_status_CNT || status < 0) { fprintf(context->logStream, "UNKNOWN STATUS CODE = %d\n", status); ASSERT_LOG(false, gDerivator_status_CNT, gDerivator_statusMsg[gDerivator_status_CNT], context->logStream); } ASSERT_LOG(status == gDerivator_status_OK, status, gDerivator_statusMsg[status], context->logStream); }); });

                ({ ({ if (gDerivator_status_TreeErr >= gDerivator_status_CNT || gDerivator_status_TreeErr < 0) { fprintf(context->logStream, "UNKNOWN STATUS CODE = %d\n", gDerivator_status_TreeErr); ASSERT_LOG(false, gDerivator_status_CNT, gDerivator_statusMsg[gDerivator_status_CNT], context->logStream); } ASSERT_LOG((gTree_addChild(&context->tree, expRoot, NULL, gDerivator_Node{gDerivator_Node_mode_num, gDerivator_Node_func_CNT, -1, 0})) == gTree_status_OK, gDerivator_status_TreeErr, gDerivator_statusMsg[gDerivator_status_TreeErr], context->logStream); }); })
                                                                                                    ;
            }
            isMul = (node->mode == gDerivator_Node_mode_mul);
            ({ ({ if (gDerivator_status_ObjPoolErr >= gDerivator_status_CNT || gDerivator_status_ObjPoolErr < 0) { fprintf(context->logStream, "UNKNOWN STATUS CODE = %d\n", gDerivator_status_ObjPoolErr); ASSERT_LOG(false, gDerivator_status_CNT, gDerivator_statusMsg[gDerivator_status_CNT], context->logStream); } ASSERT_LOG(gObjPool_free(&context->tree.pool, data[subEnd]) == gObjPool_status_OK, gDerivator_status_ObjPoolErr, gDerivator_statusMsg[gDerivator_status_ObjPoolErr], context->logStream); }); });
            ++subEnd;
            subStart = subEnd;
        }
    }
   if (isMul) {
        status = gDerivator_parser_prior(context, subStart, subEnd, mulRoot);
        ({ gDerivator_status status = (status); fprintf(stderr, "STATUS IN IS_OK = %d\n", status); ({ if (status >= gDerivator_status_CNT || status < 0) { fprintf(context->logStream, "UNKNOWN STATUS CODE = %d\n", status); ASSERT_LOG(false, gDerivator_status_CNT, gDerivator_statusMsg[gDerivator_status_CNT], context->logStream); } ASSERT_LOG(status == gDerivator_status_OK, status, gDerivator_statusMsg[status], context->logStream); }); });
    } else {
        size_t expRoot = -1;
        ({ ({ if (gDerivator_status_TreeErr >= gDerivator_status_CNT || gDerivator_status_TreeErr < 0) { fprintf(context->logStream, "UNKNOWN STATUS CODE = %d\n", gDerivator_status_TreeErr); ASSERT_LOG(false, gDerivator_status_CNT, gDerivator_statusMsg[gDerivator_status_CNT], context->logStream); } ASSERT_LOG((gTree_addChild(&context->tree, mulRoot, &expRoot, gDerivator_Node{gDerivator_Node_mode_exp, gDerivator_Node_func_CNT, 0, 0})) == gTree_status_OK, gDerivator_status_TreeErr, gDerivator_statusMsg[gDerivator_status_TreeErr], context->logStream); }); })
                                                                                           ;

        status = gDerivator_parser_prior(context, subStart, subEnd, expRoot);
        ({ gDerivator_status status = (status); fprintf(stderr, "STATUS IN IS_OK = %d\n", status); ({ if (status >= gDerivator_status_CNT || status < 0) { fprintf(context->logStream, "UNKNOWN STATUS CODE = %d\n", status); ASSERT_LOG(false, gDerivator_status_CNT, gDerivator_statusMsg[gDerivator_status_CNT], context->logStream); } ASSERT_LOG(status == gDerivator_status_OK, status, gDerivator_statusMsg[status], context->logStream); }); });

        ({ ({ if (gDerivator_status_TreeErr >= gDerivator_status_CNT || gDerivator_status_TreeErr < 0) { fprintf(context->logStream, "UNKNOWN STATUS CODE = %d\n", gDerivator_status_TreeErr); ASSERT_LOG(false, gDerivator_status_CNT, gDerivator_statusMsg[gDerivator_status_CNT], context->logStream); } ASSERT_LOG((gTree_addChild(&context->tree, expRoot, NULL, gDerivator_Node{gDerivator_Node_mode_num, gDerivator_Node_func_CNT, -1, 0})) == gTree_status_OK, gDerivator_status_TreeErr, gDerivator_statusMsg[gDerivator_status_TreeErr], context->logStream); }); })
                                                                                            ;
    }

    return status;
}

static gDerivator_status gDerivator_parser_prior(gDerivator *context,
                                                 const size_t start,
                                                 const size_t end,
                                                 const size_t subRoot)
{
    ASSERT_LOG(gPtrValid(context), gDerivator_status_BadStructPtr, gDerivator_statusMsg[gDerivator_status_BadStructPtr], stderr);
    assert(start < context->LexemeIds.len);
    assert(end <= context->LexemeIds.len);
    assert(start <= end);

    gDerivator_status status = gDerivator_status_OK;

    size_t *data = context->LexemeIds.data;
    if (({ assert(data[start] != -1); gTree_Node *node = NULL; ({ if (gDerivator_status_ObjPoolErr >= gDerivator_status_CNT || gDerivator_status_ObjPoolErr < 0) { fprintf(context->logStream, "UNKNOWN STATUS CODE = %d\n", gDerivator_status_ObjPoolErr); ASSERT_LOG(false, gDerivator_status_CNT, gDerivator_statusMsg[gDerivator_status_CNT], context->logStream); } ASSERT_LOG(gObjPool_get(&context->tree.pool, (data[start]), &node) == gObjPool_status_OK, gDerivator_status_ObjPoolErr, gDerivator_statusMsg[gDerivator_status_ObjPoolErr], context->logStream); }); assert(gPtrValid(node)); node; })->data.mode == gDerivator_Node_mode_opBrack) {
        ({ if (gDerivator_status_ParsingErr_NoBrack >= gDerivator_status_CNT || gDerivator_status_ParsingErr_NoBrack < 0) { fprintf(context->logStream, "UNKNOWN STATUS CODE = %d\n", gDerivator_status_ParsingErr_NoBrack); ASSERT_LOG(false, gDerivator_status_CNT, gDerivator_statusMsg[gDerivator_status_CNT], context->logStream); } ASSERT_LOG(({ assert(data[end - 1] != -1); gTree_Node *node = NULL; ({ if (gDerivator_status_ObjPoolErr >= gDerivator_status_CNT || gDerivator_status_ObjPoolErr < 0) { fprintf(context->logStream, "UNKNOWN STATUS CODE = %d\n", gDerivator_status_ObjPoolErr); ASSERT_LOG(false, gDerivator_status_CNT, gDerivator_statusMsg[gDerivator_status_CNT], context->logStream); } ASSERT_LOG(gObjPool_get(&context->tree.pool, (data[end - 1]), &node) == gObjPool_status_OK, gDerivator_status_ObjPoolErr, gDerivator_statusMsg[gDerivator_status_ObjPoolErr], context->logStream); }); assert(gPtrValid(node)); node; })->data.mode == gDerivator_Node_mode_clBrack, gDerivator_status_ParsingErr_NoBrack, gDerivator_statusMsg[gDerivator_status_ParsingErr_NoBrack], context->logStream); })
                                                                         ;
        status = gDerivator_parser_expr(context, start + 1, end - 1, subRoot);
        ({ ({ if (gDerivator_status_ObjPoolErr >= gDerivator_status_CNT || gDerivator_status_ObjPoolErr < 0) { fprintf(context->logStream, "UNKNOWN STATUS CODE = %d\n", gDerivator_status_ObjPoolErr); ASSERT_LOG(false, gDerivator_status_CNT, gDerivator_statusMsg[gDerivator_status_CNT], context->logStream); } ASSERT_LOG(gObjPool_free(&context->tree.pool, data[start]) == gObjPool_status_OK, gDerivator_status_ObjPoolErr, gDerivator_statusMsg[gDerivator_status_ObjPoolErr], context->logStream); }); });
        ({ ({ if (gDerivator_status_ObjPoolErr >= gDerivator_status_CNT || gDerivator_status_ObjPoolErr < 0) { fprintf(context->logStream, "UNKNOWN STATUS CODE = %d\n", gDerivator_status_ObjPoolErr); ASSERT_LOG(false, gDerivator_status_CNT, gDerivator_statusMsg[gDerivator_status_CNT], context->logStream); } ASSERT_LOG(gObjPool_free(&context->tree.pool, data[end - 1]) == gObjPool_status_OK, gDerivator_status_ObjPoolErr, gDerivator_statusMsg[gDerivator_status_ObjPoolErr], context->logStream); }); });

    } else if (end - start == 1) {
        ({ ({ if (gDerivator_status_TreeErr >= gDerivator_status_CNT || gDerivator_status_TreeErr < 0) { fprintf(context->logStream, "UNKNOWN STATUS CODE = %d\n", gDerivator_status_TreeErr); ASSERT_LOG(false, gDerivator_status_CNT, gDerivator_statusMsg[gDerivator_status_CNT], context->logStream); } ASSERT_LOG((gTree_addExistChild(&context->tree, subRoot, data[start])) == gTree_status_OK, gDerivator_status_TreeErr, gDerivator_statusMsg[gDerivator_status_TreeErr], context->logStream); }); });

    } else if (({ assert(data[start] != -1); gTree_Node *node = NULL; ({ if (gDerivator_status_ObjPoolErr >= gDerivator_status_CNT || gDerivator_status_ObjPoolErr < 0) { fprintf(context->logStream, "UNKNOWN STATUS CODE = %d\n", gDerivator_status_ObjPoolErr); ASSERT_LOG(false, gDerivator_status_CNT, gDerivator_statusMsg[gDerivator_status_CNT], context->logStream); } ASSERT_LOG(gObjPool_get(&context->tree.pool, (data[start]), &node) == gObjPool_status_OK, gDerivator_status_ObjPoolErr, gDerivator_statusMsg[gDerivator_status_ObjPoolErr], context->logStream); }); assert(gPtrValid(node)); node; })->data.mode == gDerivator_Node_mode_func) {
        ({ ({ if (gDerivator_status_TreeErr >= gDerivator_status_CNT || gDerivator_status_TreeErr < 0) { fprintf(context->logStream, "UNKNOWN STATUS CODE = %d\n", gDerivator_status_TreeErr); ASSERT_LOG(false, gDerivator_status_CNT, gDerivator_statusMsg[gDerivator_status_CNT], context->logStream); } ASSERT_LOG((gTree_addExistChild(&context->tree, subRoot, data[start])) == gTree_status_OK, gDerivator_status_TreeErr, gDerivator_statusMsg[gDerivator_status_TreeErr], context->logStream); }); });
        status = gDerivator_parser_prior(context, start + 1, end, data[start]);

    } else {
        ({ if (gDerivator_status_ParsingErr_UnknownLex >= gDerivator_status_CNT || gDerivator_status_ParsingErr_UnknownLex < 0) { fprintf(context->logStream, "UNKNOWN STATUS CODE = %d\n", gDerivator_status_ParsingErr_UnknownLex); ASSERT_LOG(false, gDerivator_status_CNT, gDerivator_statusMsg[gDerivator_status_CNT], context->logStream); } ASSERT_LOG(false, gDerivator_status_ParsingErr_UnknownLex, gDerivator_statusMsg[gDerivator_status_ParsingErr_UnknownLex], context->logStream); });
    }
    return status;
}


static gDerivator_status gDerivator_derivate(gDerivator *context, const size_t rootId)
{
    ASSERT_LOG(gPtrValid(context), gDerivator_status_BadStructPtr, gDerivator_statusMsg[gDerivator_status_BadStructPtr], stderr);

    size_t len = context->LexemeIds.len;
    ({ if (gDerivator_status_EmptyLexer >= gDerivator_status_CNT || gDerivator_status_EmptyLexer < 0) { fprintf(context->logStream, "UNKNOWN STATUS CODE = %d\n", gDerivator_status_EmptyLexer); ASSERT_LOG(false, gDerivator_status_CNT, gDerivator_statusMsg[gDerivator_status_CNT], context->logStream); } ASSERT_LOG(len != 0 && len < GDERIVATOR_LEX_LIM, gDerivator_status_EmptyLexer, gDerivator_statusMsg[gDerivator_status_EmptyLexer], context->logStream); });
    ({ if (gDerivator_status_EmptyTree >= gDerivator_status_CNT || gDerivator_status_EmptyTree < 0) { fprintf(context->logStream, "UNKNOWN STATUS CODE = %d\n", gDerivator_status_EmptyTree); ASSERT_LOG(false, gDerivator_status_CNT, gDerivator_statusMsg[gDerivator_status_CNT], context->logStream); } ASSERT_LOG(({ assert(context->tree.root != -1); gTree_Node *node = NULL; ({ if (gDerivator_status_ObjPoolErr >= gDerivator_status_CNT || gDerivator_status_ObjPoolErr < 0) { fprintf(context->logStream, "UNKNOWN STATUS CODE = %d\n", gDerivator_status_ObjPoolErr); ASSERT_LOG(false, gDerivator_status_CNT, gDerivator_statusMsg[gDerivator_status_CNT], context->logStream); } ASSERT_LOG(gObjPool_get(&context->tree.pool, (context->tree.root), &node) == gObjPool_status_OK, gDerivator_status_ObjPoolErr, gDerivator_statusMsg[gDerivator_status_ObjPoolErr], context->logStream); }); assert(gPtrValid(node)); node; })->child != -1, gDerivator_status_EmptyTree, gDerivator_statusMsg[gDerivator_status_EmptyTree], context->logStream); });

    gTree_Node *node = ({ assert(rootId != -1); gTree_Node *node = NULL; ({ if (gDerivator_status_ObjPoolErr >= gDerivator_status_CNT || gDerivator_status_ObjPoolErr < 0) { fprintf(context->logStream, "UNKNOWN STATUS CODE = %d\n", gDerivator_status_ObjPoolErr); ASSERT_LOG(false, gDerivator_status_CNT, gDerivator_statusMsg[gDerivator_status_CNT], context->logStream); } ASSERT_LOG(gObjPool_get(&context->tree.pool, (rootId), &node) == gObjPool_status_OK, gDerivator_status_ObjPoolErr, gDerivator_statusMsg[gDerivator_status_ObjPoolErr], context->logStream); }); assert(gPtrValid(node)); node; });
    gDerivator_status status = gDerivator_status_OK;
    size_t childId = node->child;

    if (node->data.mode == gDerivator_Node_mode_sum || node->data.mode == gDerivator_Node_mode_sub) {
        while (childId != -1) {
            status = gDerivator_derivate(context, childId);
            ({ gDerivator_status status = (status); fprintf(stderr, "STATUS IN IS_OK = %d\n", status); ({ if (status >= gDerivator_status_CNT || status < 0) { fprintf(context->logStream, "UNKNOWN STATUS CODE = %d\n", status); ASSERT_LOG(false, gDerivator_status_CNT, gDerivator_statusMsg[gDerivator_status_CNT], context->logStream); } ASSERT_LOG(status == gDerivator_status_OK, status, gDerivator_statusMsg[status], context->logStream); }); });

            childId = ({ assert(childId != -1); gTree_Node *node = NULL; ({ if (gDerivator_status_ObjPoolErr >= gDerivator_status_CNT || gDerivator_status_ObjPoolErr < 0) { fprintf(context->logStream, "UNKNOWN STATUS CODE = %d\n", gDerivator_status_ObjPoolErr); ASSERT_LOG(false, gDerivator_status_CNT, gDerivator_statusMsg[gDerivator_status_CNT], context->logStream); } ASSERT_LOG(gObjPool_get(&context->tree.pool, (childId), &node) == gObjPool_status_OK, gDerivator_status_ObjPoolErr, gDerivator_statusMsg[gDerivator_status_ObjPoolErr], context->logStream); }); assert(gPtrValid(node)); node; })->sibling;
        }
    } else if (node->data.mode == gDerivator_Node_mode_mul) {
        size_t sumNodeId = ({ size_t id = -1; gTree_Node *node = NULL; ({ if (gDerivator_status_ObjPoolErr >= gDerivator_status_CNT || gDerivator_status_ObjPoolErr < 0) { fprintf(context->logStream, "UNKNOWN STATUS CODE = %d\n", gDerivator_status_ObjPoolErr); ASSERT_LOG(false, gDerivator_status_CNT, gDerivator_statusMsg[gDerivator_status_CNT], context->logStream); } ASSERT_LOG(gObjPool_alloc(&context->tree.pool, &id) == gObjPool_status_OK, gDerivator_status_ObjPoolErr, gDerivator_statusMsg[gDerivator_status_ObjPoolErr], context->logStream); }); id; });

        size_t siblingId = childId;
        while (siblingId != -1) {
            size_t mulNodeId = ({ size_t id = -1; gTree_Node *node = NULL; ({ if (gDerivator_status_ObjPoolErr >= gDerivator_status_CNT || gDerivator_status_ObjPoolErr < 0) { fprintf(context->logStream, "UNKNOWN STATUS CODE = %d\n", gDerivator_status_ObjPoolErr); ASSERT_LOG(false, gDerivator_status_CNT, gDerivator_statusMsg[gDerivator_status_CNT], context->logStream); } ASSERT_LOG(gObjPool_alloc(&context->tree.pool, &id) == gObjPool_status_OK, gDerivator_status_ObjPoolErr, gDerivator_statusMsg[gDerivator_status_ObjPoolErr], context->logStream); }); id; });

            size_t iterId = childId;
            while (iterId != -1) {
                if (iterId != siblingId) {
                    size_t clonedId = -1;
                    ({ ({ if (gDerivator_status_TreeErr >= gDerivator_status_CNT || gDerivator_status_TreeErr < 0) { fprintf(context->logStream, "UNKNOWN STATUS CODE = %d\n", gDerivator_status_TreeErr); ASSERT_LOG(false, gDerivator_status_CNT, gDerivator_statusMsg[gDerivator_status_CNT], context->logStream); } ASSERT_LOG((gTree_cloneSubtree (&context->tree, iterId, &clonedId)) == gTree_status_OK, gDerivator_status_TreeErr, gDerivator_statusMsg[gDerivator_status_TreeErr], context->logStream); }); });
                    fprintf(stderr, "iterId = %lu; clonedId = %lu; mulNodeId = %lu\n", iterId, clonedId, mulNodeId);
                    ({ ({ if (gDerivator_status_TreeErr >= gDerivator_status_CNT || gDerivator_status_TreeErr < 0) { fprintf(context->logStream, "UNKNOWN STATUS CODE = %d\n", gDerivator_status_TreeErr); ASSERT_LOG(false, gDerivator_status_CNT, gDerivator_statusMsg[gDerivator_status_CNT], context->logStream); } ASSERT_LOG((gTree_addExistChild(&context->tree, mulNodeId, clonedId)) == gTree_status_OK, gDerivator_status_TreeErr, gDerivator_statusMsg[gDerivator_status_TreeErr], context->logStream); }); });
                }
                iterId = ({ assert(siblingId != -1); gTree_Node *node = NULL; ({ if (gDerivator_status_ObjPoolErr >= gDerivator_status_CNT || gDerivator_status_ObjPoolErr < 0) { fprintf(context->logStream, "UNKNOWN STATUS CODE = %d\n", gDerivator_status_ObjPoolErr); ASSERT_LOG(false, gDerivator_status_CNT, gDerivator_statusMsg[gDerivator_status_CNT], context->logStream); } ASSERT_LOG(gObjPool_get(&context->tree.pool, (siblingId), &node) == gObjPool_status_OK, gDerivator_status_ObjPoolErr, gDerivator_statusMsg[gDerivator_status_ObjPoolErr], context->logStream); }); assert(gPtrValid(node)); node; })->sibling;
            }

            size_t clonedId = -1;
            ({ ({ if (gDerivator_status_TreeErr >= gDerivator_status_CNT || gDerivator_status_TreeErr < 0) { fprintf(context->logStream, "UNKNOWN STATUS CODE = %d\n", gDerivator_status_TreeErr); ASSERT_LOG(false, gDerivator_status_CNT, gDerivator_statusMsg[gDerivator_status_CNT], context->logStream); } ASSERT_LOG((gTree_cloneSubtree(&context->tree, siblingId, &clonedId)) == gTree_status_OK, gDerivator_status_TreeErr, gDerivator_statusMsg[gDerivator_status_TreeErr], context->logStream); }); });
            ({ ({ if (gDerivator_status_TreeErr >= gDerivator_status_CNT || gDerivator_status_TreeErr < 0) { fprintf(context->logStream, "UNKNOWN STATUS CODE = %d\n", gDerivator_status_TreeErr); ASSERT_LOG(false, gDerivator_status_CNT, gDerivator_statusMsg[gDerivator_status_CNT], context->logStream); } ASSERT_LOG((gTree_addExistChild(&context->tree, mulNodeId, clonedId)) == gTree_status_OK, gDerivator_status_TreeErr, gDerivator_statusMsg[gDerivator_status_TreeErr], context->logStream); }); });
            ({ ({ if (gDerivator_status_TreeErr >= gDerivator_status_CNT || gDerivator_status_TreeErr < 0) { fprintf(context->logStream, "UNKNOWN STATUS CODE = %d\n", gDerivator_status_TreeErr); ASSERT_LOG(false, gDerivator_status_CNT, gDerivator_statusMsg[gDerivator_status_CNT], context->logStream); } ASSERT_LOG((gTree_addExistChild(&context->tree, sumNodeId, mulNodeId)) == gTree_status_OK, gDerivator_status_TreeErr, gDerivator_statusMsg[gDerivator_status_TreeErr], context->logStream); }); });
            ({ gDerivator_status status = (gDerivator_derivate(context, siblingId)); fprintf(stderr, "STATUS IN IS_OK = %d\n", status); ({ if (status >= gDerivator_status_CNT || status < 0) { fprintf(context->logStream, "UNKNOWN STATUS CODE = %d\n", status); ASSERT_LOG(false, gDerivator_status_CNT, gDerivator_statusMsg[gDerivator_status_CNT], context->logStream); } ASSERT_LOG(status == gDerivator_status_OK, status, gDerivator_statusMsg[status], context->logStream); }); });

            siblingId = ({ assert(siblingId != -1); gTree_Node *node = NULL; ({ if (gDerivator_status_ObjPoolErr >= gDerivator_status_CNT || gDerivator_status_ObjPoolErr < 0) { fprintf(context->logStream, "UNKNOWN STATUS CODE = %d\n", gDerivator_status_ObjPoolErr); ASSERT_LOG(false, gDerivator_status_CNT, gDerivator_statusMsg[gDerivator_status_CNT], context->logStream); } ASSERT_LOG(gObjPool_get(&context->tree.pool, (siblingId), &node) == gObjPool_status_OK, gDerivator_status_ObjPoolErr, gDerivator_statusMsg[gDerivator_status_ObjPoolErr], context->logStream); }); assert(gPtrValid(node)); node; })->sibling;
        }

        ({ ({ if (gDerivator_status_TreeErr >= gDerivator_status_CNT || gDerivator_status_TreeErr < 0) { fprintf(context->logStream, "UNKNOWN STATUS CODE = %d\n", gDerivator_status_TreeErr); ASSERT_LOG(false, gDerivator_status_CNT, gDerivator_statusMsg[gDerivator_status_CNT], context->logStream); } ASSERT_LOG((gTree_replaceNode(&context->tree, rootId, sumNodeId)) == gTree_status_OK, gDerivator_status_TreeErr, gDerivator_statusMsg[gDerivator_status_TreeErr], context->logStream); }); });
        ({ ({ if (gDerivator_status_TreeErr >= gDerivator_status_CNT || gDerivator_status_TreeErr < 0) { fprintf(context->logStream, "UNKNOWN STATUS CODE = %d\n", gDerivator_status_TreeErr); ASSERT_LOG(false, gDerivator_status_CNT, gDerivator_statusMsg[gDerivator_status_CNT], context->logStream); } ASSERT_LOG((gTree_killSubtree(&context->tree, rootId)) == gTree_status_OK, gDerivator_status_TreeErr, gDerivator_statusMsg[gDerivator_status_TreeErr], context->logStream); }); });
    } else if (node->data.mode == gDerivator_Node_mode_num) {
        node->data.value = 0;
    } else if (node->data.mode == gDerivator_Node_mode_var) {
        node->data.value = 1;
        node->data.mode = gDerivator_Node_mode_num;
    } else {
        fprintf(stderr, "ERROR: this feature has not been implemented yet!\n");
        assert(false);
    }

    return status;
}
# 2 "./gderivator.cpp" 2

int main()
{
    gDerivator der;
    gDerivator_ctor(&der, stderr);

    gDerivator_lexer(&der, "123 * x");
    gDerivator_parser(&der);


    FILE *out = fopen("dump.gv", "w");
    gTree_dumpPoolGraphViz(&der.tree, out);
    fclose(out);

    gDerivator_dtor(&der);
}
