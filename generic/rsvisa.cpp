#include "../config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cstdint>
#include <tcl.h>
#include <rsvisa/visa.h>
#define NS_PREFIX PACKAGE_NAME"::"
#define STATUS_TXT_LEN 100

// Forward declarations for command functions using modern Tcl object API
extern "C" {
    // Package initialization and unload
    int Rsvisa_Init(Tcl_Interp *interp);
    int Rsvisa_Unload(Tcl_Interp *interp, int flags);

    // Command implementations
    int GetResourceManager(ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *CONST objv[]);
    int OpenResource(ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *CONST objv[]);
    int CloseResource(ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *CONST objv[]);
    int Idn(ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *CONST objv[]);
    int Stb(ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *CONST objv[]);
    int WriteRead(ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *CONST objv[]);
    int WriteReadBin(ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *CONST objv[]);
    int Write(ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *CONST objv[]);
    int Read(ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *CONST objv[]);
    int SetAttribute(ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *CONST objv[]);
    int GetAttribute(ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *CONST objv[]);
    int SetTimeout(ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *CONST objv[]);
    int GetTimeout(ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *CONST objv[]);
}

int GetResourceManager(ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *CONST objv[]) {
    ViStatus status = VI_SUCCESS;
    ViSession rmHandle;
    char statusTxt[STATUS_TXT_LEN];

    if (objc != 1) {
        Tcl_WrongNumArgs(interp, 1, objv, "");
        return TCL_ERROR;
    }

    // Open Resource manager first
    status = viOpenDefaultRM (&rmHandle);
    if (status == VI_SUCCESS) {
        Tcl_SetObjResult(interp, Tcl_NewLongObj(rmHandle));
        return TCL_OK;
    } else {
        snprintf(statusTxt, STATUS_TXT_LEN, "VISA Error: %d", status);
        Tcl_SetObjResult(interp, Tcl_NewStringObj(statusTxt, -1));
        return TCL_ERROR;
    }
}

int OpenResource(ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *CONST objv[]) {
    ViStatus status = VI_SUCCESS;
    ViSession rmHandle, scopeHandle;
    char statusTxt[STATUS_TXT_LEN];
    long rmHandleLong;
    char* resourceUri;

    if (objc != 3) {
        Tcl_WrongNumArgs(interp, 1, objv, "resourceManager resourceUri");
        return TCL_ERROR;
    }

    // Get resource manager handle
    if (Tcl_GetLongFromObj(interp, objv[1], &rmHandleLong) != TCL_OK) {
        return TCL_ERROR;
    }
    rmHandle = (ViSession)rmHandleLong;

    // Get resource URI string
    resourceUri = Tcl_GetString(objv[2]);

    status = viOpen(rmHandle, resourceUri, VI_NULL, VI_NULL, &scopeHandle);
    if (status != VI_SUCCESS) {
        snprintf(statusTxt, STATUS_TXT_LEN, "VISA Error: %d", status);
        Tcl_SetObjResult(interp, Tcl_NewStringObj(statusTxt, -1));
        return TCL_ERROR;
    }

    status = viClear(scopeHandle); //clear instrument's io buffers
    if (status == VI_SUCCESS) {
        Tcl_SetObjResult(interp, Tcl_NewLongObj(scopeHandle));
        return TCL_OK;
    } else {
        snprintf(statusTxt, STATUS_TXT_LEN, "VISA Error: %d", status);
        Tcl_SetObjResult(interp, Tcl_NewStringObj(statusTxt, -1));
        return TCL_ERROR;
    }
}

int CloseResource(ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *CONST objv[]) {
    ViStatus status = VI_SUCCESS;
    ViSession scopeHandle;
    char statusTxt[STATUS_TXT_LEN];
    long scopeHandleLong;

    if (objc != 2) {
        Tcl_WrongNumArgs(interp, 1, objv, "instrumentHandle");
        return TCL_ERROR;
    }

    if (Tcl_GetLongFromObj(interp, objv[1], &scopeHandleLong) != TCL_OK) {
        return TCL_ERROR;
    }
    scopeHandle = (ViSession)scopeHandleLong;

    status = viClose(scopeHandle);

    if (status == VI_SUCCESS) {
        return TCL_OK;
    } else {
        snprintf(statusTxt, STATUS_TXT_LEN, "VISA Error: %d", status);
        Tcl_SetObjResult(interp, Tcl_NewStringObj(statusTxt, -1));
        return TCL_ERROR;
    }
}

int Idn(ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *CONST objv[]) {
    char idnQuery[] = "*IDN?\n";
    ViStatus status = VI_SUCCESS;
    char statusTxt[STATUS_TXT_LEN];

    ViUInt32 returnCount = 0;
    ViSession scopeHandle;
    long scopeHandleLong;

    int buflen = 1024;
    char response[buflen];

    if (objc != 2) {
        Tcl_WrongNumArgs(interp, 1, objv, "instrumentHandle");
        return TCL_ERROR;
    }

    if (Tcl_GetLongFromObj(interp, objv[1], &scopeHandleLong) != TCL_OK) {
        return TCL_ERROR;
    }
    scopeHandle = (ViSession)scopeHandleLong;

    status = viWrite(scopeHandle, (ViBuf)idnQuery, (ViUInt32)strlen(idnQuery), VI_NULL);
    if (status != VI_SUCCESS) {
        snprintf(statusTxt, STATUS_TXT_LEN, "VISA write error: %d", status);
        Tcl_SetObjResult(interp, Tcl_NewStringObj(statusTxt, -1));
        return TCL_ERROR;
    }

    while (1) {
        status = viRead(scopeHandle, (ViBuf)response, buflen, &returnCount);
        if (status != VI_SUCCESS) {
            snprintf(statusTxt, STATUS_TXT_LEN, "VISA read error: %d", status);
            Tcl_SetObjResult(interp, Tcl_NewStringObj(statusTxt, -1));
            return TCL_ERROR;
        }

        if (returnCount < buflen) break;
    }

    response[returnCount] = 0; //terminate the string properly
    Tcl_SetObjResult(interp, Tcl_NewByteArrayObj((const unsigned char*)response, returnCount));
    return TCL_OK;
}

int Stb(ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *CONST objv[]) {
    ViStatus status = VI_SUCCESS;
    char statusTxt[STATUS_TXT_LEN];

    ViSession scopeHandle;
    long scopeHandleLong;

    if (objc != 2) {
        Tcl_WrongNumArgs(interp, 1, objv, "instrumentHandle");
        return TCL_ERROR;
    }

    if (Tcl_GetLongFromObj(interp, objv[1], &scopeHandleLong) != TCL_OK) {
        return TCL_ERROR;
    }
    scopeHandle = (ViSession)scopeHandleLong;

    ViUInt16 stb;
    status = viReadSTB(scopeHandle, &stb);

    if (status == VI_SUCCESS) {
        Tcl_SetObjResult(interp, Tcl_NewIntObj(stb));
        return TCL_OK;
    } else {
        snprintf(statusTxt, STATUS_TXT_LEN, "VISA Error: %d", status);
        Tcl_SetObjResult(interp, Tcl_NewStringObj(statusTxt, -1));
        return TCL_ERROR;
    }
}

int _WriteRead(size_t buflen, Tcl_Interp *interp, int objc, Tcl_Obj *CONST objv[]) {
    ViStatus status = VI_SUCCESS;
    char statusTxt[STATUS_TXT_LEN];

    ViUInt32 returnCount = 0;
    ViSession scopeHandle;
    long scopeHandleLong;

    size_t total  = buflen;
    char* buf = (char*) malloc(buflen);
    if (buf == NULL) {
        Tcl_SetObjResult(interp, Tcl_NewStringObj("Out of memory", -1));
        return TCL_ERROR;
    }
    char* bp = buf;

    char* cmd;

    if (objc != 3) {
        free(buf);
        Tcl_WrongNumArgs(interp, 1, objv, "instrumentHandle command");
        return TCL_ERROR;
    }

    if (Tcl_GetLongFromObj(interp, objv[1], &scopeHandleLong) != TCL_OK) {
        free(buf);
        return TCL_ERROR;
    }
    scopeHandle = (ViSession)scopeHandleLong;

    cmd = Tcl_GetString(objv[2]);

    status = viWrite(scopeHandle, (ViBuf)cmd, (ViUInt32)strlen(cmd), VI_NULL);
    if (status != VI_SUCCESS) {
        free(buf);
        snprintf(statusTxt, STATUS_TXT_LEN, "VISA write error: %d", status);
        Tcl_SetObjResult(interp, Tcl_NewStringObj(statusTxt, -1));
        return TCL_ERROR;
    }

    size_t sum = 0;
    while (1) {
        status = viRead(scopeHandle, (ViBuf)bp, buflen, &returnCount);
        sum += returnCount;
        if ((status != VI_SUCCESS) && (status != VI_SUCCESS_MAX_CNT)) {
            free(buf);
            snprintf(statusTxt, STATUS_TXT_LEN, "VISA read error: %d", status);
            Tcl_SetObjResult(interp, Tcl_NewStringObj(statusTxt, -1));
            return TCL_ERROR;
        }

        bp += returnCount;
        if ((returnCount < buflen) || (status == VI_SUCCESS_MAX_CNT)) {
            break;
        } else {
            total += buflen;
            char* newbuf = (char*) realloc(buf, total);
            if (newbuf == NULL) {
                free(buf);
                Tcl_SetObjResult(interp, Tcl_NewStringObj("Out of memory", -1));
                return TCL_ERROR;
            }
            buf = newbuf;
            bp = buf + sum;
        }
    }

    buf[sum] = 0; //terminate the string properly
    Tcl_SetObjResult(interp, Tcl_NewByteArrayObj((const unsigned char*)buf, sum));
    free(buf);

    return TCL_OK;
}

int WriteRead(ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *CONST objv[]) {
    return _WriteRead(1024, interp, objc, objv);
}

int WriteReadBin(ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *CONST objv[]) {
    return _WriteRead(1000000, interp, objc, objv);
}

int Write(ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *CONST objv[]) {
    ViStatus status = VI_SUCCESS;
    char statusTxt[STATUS_TXT_LEN];

    ViSession scopeHandle;
    long scopeHandleLong;

    char* cmd;

    if (objc != 3) {
        Tcl_WrongNumArgs(interp, 1, objv, "instrumentHandle command");
        return TCL_ERROR;
    }

    if (Tcl_GetLongFromObj(interp, objv[1], &scopeHandleLong) != TCL_OK) {
        return TCL_ERROR;
    }
    scopeHandle = (ViSession)scopeHandleLong;

    cmd = Tcl_GetString(objv[2]);

    status = viWrite(scopeHandle, (ViBuf)cmd, (ViUInt32)strlen(cmd), VI_NULL);

    if (status == VI_SUCCESS) {
        return TCL_OK;
    } else {
        snprintf(statusTxt, STATUS_TXT_LEN, "VISA Error: %d", status);
        Tcl_SetObjResult(interp, Tcl_NewStringObj(statusTxt, -1));
        return TCL_ERROR;
    }
}

int Read(ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *CONST objv[]) {
    ViStatus status = VI_SUCCESS;
    char statusTxt[STATUS_TXT_LEN];

    ViUInt32 returnCount = 0;
    ViSession scopeHandle;
    long scopeHandleLong;

    size_t buflen = 1024;
    size_t total  = buflen;
    char* buf = (char*) malloc(buflen);
    if (buf == NULL) {
        Tcl_SetObjResult(interp, Tcl_NewStringObj("Out of memory", -1));
        return TCL_ERROR;
    }
    char* bp = buf;

    if (objc != 2) {
        free(buf);
        Tcl_WrongNumArgs(interp, 1, objv, "instrumentHandle");
        return TCL_ERROR;
    }

    if (Tcl_GetLongFromObj(interp, objv[1], &scopeHandleLong) != TCL_OK) {
        free(buf);
        return TCL_ERROR;
    }
    scopeHandle = (ViSession)scopeHandleLong;

    size_t sum = 0;
    while (1) {
        status = viRead(scopeHandle, (ViBuf)bp, buflen, &returnCount);
        sum += returnCount;
        if ((status != VI_SUCCESS) && (status != VI_SUCCESS_MAX_CNT)) {
            free(buf);
            snprintf(statusTxt, STATUS_TXT_LEN, "VISA read error: %d", status);
            Tcl_SetObjResult(interp, Tcl_NewStringObj(statusTxt, -1));
            return TCL_ERROR;
        }

        bp += returnCount;
        if ((returnCount < buflen) || (status == VI_SUCCESS_MAX_CNT)) {
            break;
        } else {
            total += buflen;
            char* newbuf = (char*) realloc(buf, total);
            if (newbuf == NULL) {
                free(buf);
                Tcl_SetObjResult(interp, Tcl_NewStringObj("Out of memory", -1));
                return TCL_ERROR;
            }
            buf = newbuf;
            bp = buf + sum;
        }
    }

    buf[sum] = 0; //terminate the string properly
    Tcl_SetObjResult(interp, Tcl_NewByteArrayObj((const unsigned char*)buf, sum));
    free(buf);

    return TCL_OK;
}

int SetAttribute(ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *CONST objv[]) {
    ViStatus status = VI_SUCCESS;
    char statusTxt[STATUS_TXT_LEN];

    ViSession scopeHandle;
    long scopeHandleLong;
    int attr, value;

    if (objc != 4) {
        Tcl_WrongNumArgs(interp, 1, objv, "instrumentHandle attribute value");
        return TCL_ERROR;
    }

    if (Tcl_GetLongFromObj(interp, objv[1], &scopeHandleLong) != TCL_OK) {
        return TCL_ERROR;
    }
    scopeHandle = (ViSession)scopeHandleLong;

    if (Tcl_GetIntFromObj(interp, objv[2], &attr) != TCL_OK) {
        return TCL_ERROR;
    }

    if (Tcl_GetIntFromObj(interp, objv[3], &value) != TCL_OK) {
        return TCL_ERROR;
    }

    status = viSetAttribute(scopeHandle, (ViAttr) attr, (ViAttrState) value);
    if (status != VI_SUCCESS) {
        snprintf(statusTxt, STATUS_TXT_LEN, "VISA set attribute error: %d", status);
        Tcl_SetObjResult(interp, Tcl_NewStringObj(statusTxt, -1));
        return TCL_ERROR;
    }

    return TCL_OK;
}

int GetAttribute(ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *CONST objv[]) {
    ViStatus status = VI_SUCCESS;
    char statusTxt[STATUS_TXT_LEN];

    ViSession scopeHandle;
    long scopeHandleLong;
    int attr;

    int64_t value = 0;

    if (objc != 3) {
        Tcl_WrongNumArgs(interp, 1, objv, "instrumentHandle attribute");
        return TCL_ERROR;
    }

    if (Tcl_GetLongFromObj(interp, objv[1], &scopeHandleLong) != TCL_OK) {
        return TCL_ERROR;
    }
    scopeHandle = (ViSession)scopeHandleLong;

    if (Tcl_GetIntFromObj(interp, objv[2], &attr) != TCL_OK) {
        return TCL_ERROR;
    }

    status = viGetAttribute(scopeHandle, (ViAttr) attr, &value);
    if (status != VI_SUCCESS) {
        snprintf(statusTxt, STATUS_TXT_LEN, "VISA get attribute error: %d", status);
        Tcl_SetObjResult(interp, Tcl_NewStringObj(statusTxt, -1));
        return TCL_ERROR;
    }

    Tcl_SetObjResult(interp, Tcl_NewLongObj((long) value));

    return TCL_OK;
}

int SetTimeout(ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *CONST objv[]) {
    ViStatus status = VI_SUCCESS;
    char statusTxt[STATUS_TXT_LEN];

    ViSession scopeHandle;
    long scopeHandleLong;
    int timeout;

    if (objc != 3) {
        Tcl_WrongNumArgs(interp, 1, objv, "instrumentHandle timeout");
        return TCL_ERROR;
    }

    if (Tcl_GetLongFromObj(interp, objv[1], &scopeHandleLong) != TCL_OK) {
        return TCL_ERROR;
    }
    scopeHandle = (ViSession)scopeHandleLong;

    if (Tcl_GetIntFromObj(interp, objv[2], &timeout) != TCL_OK) {
        return TCL_ERROR;
    }

    status = viSetAttribute(scopeHandle, VI_ATTR_TMO_VALUE, (ViAttrState) timeout);
    if (status != VI_SUCCESS) {
        snprintf(statusTxt, STATUS_TXT_LEN, "VISA set timeout error: %d", status);
        Tcl_SetObjResult(interp, Tcl_NewStringObj(statusTxt, -1));
        return TCL_ERROR;
    }

    return TCL_OK;
}

int GetTimeout(ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *CONST objv[]) {
    ViStatus status = VI_SUCCESS;
    char statusTxt[STATUS_TXT_LEN];

    ViSession scopeHandle;
    long scopeHandleLong;

    int64_t timeout = 0;

    if (objc != 2) {
        Tcl_WrongNumArgs(interp, 1, objv, "instrumentHandle");
        return TCL_ERROR;
    }

    if (Tcl_GetLongFromObj(interp, objv[1], &scopeHandleLong) != TCL_OK) {
        return TCL_ERROR;
    }
    scopeHandle = (ViSession)scopeHandleLong;

    status = viGetAttribute(scopeHandle, VI_ATTR_TMO_VALUE, &timeout);
    if (status != VI_SUCCESS) {
        snprintf(statusTxt, STATUS_TXT_LEN, "VISA get timeout error: %d", status);
        Tcl_SetObjResult(interp, Tcl_NewStringObj(statusTxt, -1));
        return TCL_ERROR;
    }

    Tcl_SetObjResult(interp, Tcl_NewLongObj((long) timeout));

    return TCL_OK;
}



int Rsvisa_Init(Tcl_Interp *interp) {
    if (Tcl_InitStubs(interp, "8.6", 0) == NULL)
        return TCL_ERROR;
    
    // create namespace
    if (Tcl_CreateNamespace(interp, NS_PREFIX, NULL, NULL) == NULL) {
        return TCL_ERROR;
    }

    // Register commands using modern Tcl_CreateObjCommand
    Tcl_CreateObjCommand(interp, NS_PREFIX "GetResourceManager", GetResourceManager, NULL, NULL);
    Tcl_CreateObjCommand(interp, NS_PREFIX "OpenResource",       OpenResource,       NULL, NULL);
    Tcl_CreateObjCommand(interp, NS_PREFIX "CloseResource",      CloseResource,      NULL, NULL);
    Tcl_CreateObjCommand(interp, NS_PREFIX "Idn",                Idn,                NULL, NULL);
    Tcl_CreateObjCommand(interp, NS_PREFIX "Stb",                Stb,                NULL, NULL);
    Tcl_CreateObjCommand(interp, NS_PREFIX "WriteRead",          WriteRead,          NULL, NULL);
    Tcl_CreateObjCommand(interp, NS_PREFIX "WriteReadBin",       WriteReadBin,       NULL, NULL);
    Tcl_CreateObjCommand(interp, NS_PREFIX "Write",              Write,              NULL, NULL);
    Tcl_CreateObjCommand(interp, NS_PREFIX "Read",               Read,               NULL, NULL);
    Tcl_CreateObjCommand(interp, NS_PREFIX "SetAttribute",       SetAttribute,       NULL, NULL);
    Tcl_CreateObjCommand(interp, NS_PREFIX "GetAttribute",       GetAttribute,       NULL, NULL);
    Tcl_CreateObjCommand(interp, NS_PREFIX "SetTimeout",         SetTimeout,         NULL, NULL);
    Tcl_CreateObjCommand(interp, NS_PREFIX "GetTimeout",         GetTimeout,         NULL, NULL);

    // provide package
    if (Tcl_PkgProvide(interp, PACKAGE_NAME, PACKAGE_VERSION) != TCL_OK) {
        return TCL_ERROR;
    }

    return TCL_OK;
}

int Rsvisa_Unload(Tcl_Interp *interp, int flags) {
    // destroy operation.
    return TCL_OK;
}
