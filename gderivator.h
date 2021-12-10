#pragma once

enum gDerivator_Node_mode {
    gDerivator_Node_mode_none,
    gDerivator_Node_mode_sum,
    gDerivator_Node_mode_sub,
    gDerivator_Node_mode_mul,
    gDerivator_Node_mode_div,
    gDerivator_Node_mode_CNT,
};

struct gDerivator_Node 
{
    gDerivator_Node_mode mode;
    long double value;
} typedef gDerivator_Node;

typedef gDerivator_Node GTREE_TYPE;

#include "gtree.h"


bool gTree_storeData(gDerivator_Node data, size_t level, FILE *out) //TODO
{
    return 0;
}

bool gTree_restoreData(gDerivator_Node *data, FILE *in) //TODO
{
    return 0;
}

bool gTree_printData(gDerivator_Node data, FILE *out) // TODO
{
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
    GDERIVATOR_ASSERT_LOG(gObjPool_get(&context->tree.pool, id, &node) == gObjPool_status_OK,    \
                            gDerivator_status_ObjPoolErr);                                        \
    assert(gPtrValid(node));                                                                       \
    node;                                                                                           \
})                                                                   


struct gDerivator {
    gTree *tree;
    FILE *logStream;
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

    return gDerivator_status_OK;
}

gDerivator_status gDerivator_dtor(gDerivator *context)
{
    GDERIVATOR_CHECK_SELF_PTR(context);

    gTree_status status = gTree_dtor(&context->tree);
    GDERIVATOR_ASSERT_LOG(status == gTree_status_OK, gDerivator_status_TreeErr);

    return gDerivator_status_OK;
}

gDerivator_status gDerivator_getExpr(gDerivator *context, const char *buffer)
{
        
}

