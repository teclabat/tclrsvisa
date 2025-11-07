#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cstdint>
#include <tcl.h>


#include <visa.h>

#define NS_PREFIX "rsvisa::"                       /* Tcl namespace prefix for command definitions */


#define STATUS_TXT_LEN 100

extern "C" {
  // extern for C++.
  int Rsvisa_Init(Tcl_Interp *Interp);
  int Rsvisa_Unload(Tcl_Interp *Interp);
}

int GetResourceManager(int notUsed, Tcl_Interp *interp, int argc, char **argv) {
  ViStatus status = VI_SUCCESS;
  ViSession rmHandle;
  char statusTxt[STATUS_TXT_LEN];

  // Open Resource manager first
  status = viOpenDefaultRM (&rmHandle);
  if (status == VI_SUCCESS) {
    Tcl_SetObjResult(interp, Tcl_NewLongObj(rmHandle));
    return TCL_OK;
  } else {
    snprintf(statusTxt, STATUS_TXT_LEN, "%d", status);
    Tcl_AppendResult(interp, "VISA Error: ", &statusTxt, NULL);
    return TCL_ERROR;
  }
}

int OpenResource(int notUsed, Tcl_Interp *interp, int argc, char **argv) {
  ViStatus status = VI_SUCCESS;
  ViSession rmHandle, scopeHandle;
  char statusTxt[STATUS_TXT_LEN];

  char* ressourceUri;

  if (argc != 3) {
    Tcl_AppendResult(interp, "Resource handle or URI missing", NULL);
    return TCL_ERROR;
  }
  rmHandle = atol(argv[1]);
  ressourceUri = argv[2];

  status = viOpen(rmHandle, ressourceUri, VI_NULL, VI_NULL, &scopeHandle);
  if (status != VI_SUCCESS) {
    snprintf(statusTxt, STATUS_TXT_LEN, "%d", status);
    Tcl_AppendResult(interp, "VISA Error: ", &statusTxt, NULL);
    return TCL_ERROR;
  }

  status = viClear(scopeHandle); //clear instrument's io buffers
  if (status == VI_SUCCESS) {
    Tcl_SetObjResult(interp, Tcl_NewLongObj(scopeHandle));
    return TCL_OK;
  } else {
    snprintf(statusTxt, STATUS_TXT_LEN, "%d", status);
    Tcl_AppendResult(interp, "VISA Error: ", &statusTxt, NULL);
    return TCL_ERROR;
  }
}

int CloseResource(int notUsed, Tcl_Interp *interp, int argc, char **argv) {
  ViStatus status = VI_SUCCESS;
  ViSession rmHandle, scopeHandle;
  char statusTxt[STATUS_TXT_LEN];

  if (argc != 2) {
    Tcl_AppendResult(interp, "Instrument handle missing", NULL);
    return TCL_ERROR;
  }
  scopeHandle = atol(argv[1]);

  status = viClose(scopeHandle);

  if (status == VI_SUCCESS) {
    return TCL_OK;
  } else {
    snprintf(statusTxt, STATUS_TXT_LEN, "%d", status);
    Tcl_AppendResult(interp, "VISA Error: ", &statusTxt, NULL);
    return TCL_ERROR;
  }
}

int Idn(int notUsed, Tcl_Interp *interp, int argc, char **argv) {
  char idnQuery[] = "*IDN?\n";
  ViStatus status = VI_SUCCESS;
  char statusTxt[STATUS_TXT_LEN];

  ViUInt32 returnCount = 0;
  ViSession scopeHandle;

  int buflen = 1024;
  char response[buflen];

  if (argc != 2) {
    Tcl_AppendResult(interp, "Instrument handle missing", NULL);
    return TCL_ERROR;
  }
  scopeHandle = atol(argv[1]);

  status = viWrite(scopeHandle, (ViBuf)idnQuery, (ViUInt32)strlen(idnQuery), VI_NULL);
  if (status != VI_SUCCESS) {
    snprintf(statusTxt, STATUS_TXT_LEN, "%d", status);
    Tcl_AppendResult(interp, "VISA write error: ", &statusTxt, NULL);
    return TCL_ERROR;
  }

  while (1) {
    status = viRead(scopeHandle, (ViBuf)response, buflen, &returnCount);
    if (status != VI_SUCCESS) {
      snprintf(statusTxt, STATUS_TXT_LEN, "%d", status);
      Tcl_AppendResult(interp, "VISA read error: ", &statusTxt, NULL);
      return TCL_ERROR;
    }

    if (returnCount < buflen) break;
  }

  response[returnCount] = 0; //terminate the string properly
  Tcl_SetObjResult(interp, Tcl_NewByteArrayObj((const unsigned char*)response, returnCount));
  return TCL_OK;
}

int Stb(int notUsed, Tcl_Interp *interp, int argc, char **argv) {
  ViStatus status = VI_SUCCESS;
  char statusTxt[STATUS_TXT_LEN];

  ViUInt32 returnCount = 0;
  ViSession scopeHandle;

  if (argc != 2) {
    Tcl_AppendResult(interp, "Instrument handle missing", NULL);
    return TCL_ERROR;
  }
  scopeHandle = atol(argv[1]);

  ViUInt16 stb;
  status = viReadSTB(scopeHandle, &stb);

  if (status == VI_SUCCESS) {
    Tcl_SetObjResult(interp, Tcl_NewIntObj(stb));
    return TCL_OK;
  } else {
    snprintf(statusTxt, STATUS_TXT_LEN, "%d", status);
    Tcl_AppendResult(interp, "VISA Error: ", &statusTxt, NULL);
    return TCL_ERROR;
  }
}

int _WriteRead(size_t buflen, Tcl_Interp *interp, int argc, char **argv) {
  ViStatus status = VI_SUCCESS;
  char statusTxt[STATUS_TXT_LEN];

  ViUInt32 returnCount = 0;
  ViSession scopeHandle;

  size_t total  = buflen;
  char* buf = (char*) malloc(buflen);
  char* bp = buf;

  char* cmd;

  if (argc != 3) {
    Tcl_AppendResult(interp, "Instrument handle or command missing", NULL);
    return TCL_ERROR;
  }
  scopeHandle = atol(argv[1]);
  cmd = argv[2];

  status = viWrite(scopeHandle, (ViBuf)cmd, (ViUInt32)strlen(cmd), VI_NULL);
  if (status != VI_SUCCESS) {
    snprintf(statusTxt, STATUS_TXT_LEN, "%d", status);
    Tcl_AppendResult(interp, "VISA write error: ", &statusTxt, NULL);
    return TCL_ERROR;
  }

  size_t sum = 0;
  while (1) {
    status = viRead(scopeHandle, (ViBuf)bp, buflen, &returnCount);
    sum += returnCount;
    if ((status != VI_SUCCESS) && (status != VI_SUCCESS_MAX_CNT)) {
      free(buf);
      snprintf(statusTxt, STATUS_TXT_LEN, "%d", status);
      Tcl_AppendResult(interp, "VISA read error: ", &statusTxt, NULL);
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
        Tcl_AppendResult(interp, "Out of memory", NULL);
        return TCL_ERROR;
      }
      buf = newbuf;
    }
  }

  buf[sum] = 0; //terminate the string properly
  Tcl_SetObjResult(interp, Tcl_NewByteArrayObj((const unsigned char*)buf, sum));

  return TCL_OK;
}

int WriteRead(int notUsed, Tcl_Interp *interp, int argc, char **argv) {
  return _WriteRead(1024, interp, argc, argv);
}

int WriteReadBin(int notUsed, Tcl_Interp *interp, int argc, char **argv) {
  return _WriteRead(1000000, interp, argc, argv);
}

int Write(int notUsed, Tcl_Interp *interp, int argc, char **argv) {
  ViStatus status = VI_SUCCESS;
  char statusTxt[STATUS_TXT_LEN];

  ViUInt32 returnCount = 0;
  ViSession scopeHandle;

  char* cmd;

  if (argc != 3) {
    Tcl_AppendResult(interp, "Instrument handle or command missing", NULL);
    return TCL_ERROR;
  }
  scopeHandle = atol(argv[1]);
  cmd = argv[2];

  status = viWrite(scopeHandle, (ViBuf)cmd, (ViUInt32)strlen(cmd), VI_NULL);

  if (status == VI_SUCCESS) {
    return TCL_OK;
  } else {
    snprintf(statusTxt, STATUS_TXT_LEN, "%d", status);
    Tcl_AppendResult(interp, "VISA Error: ", &statusTxt, NULL);
    return TCL_ERROR;
  }
}

int Read(int notUsed, Tcl_Interp *interp, int argc, char **argv) {
  ViStatus status = VI_SUCCESS;
  char statusTxt[STATUS_TXT_LEN];

  ViUInt32 returnCount = 0;
  ViSession scopeHandle;

  size_t buflen = 1024;
  size_t total  = buflen;
  char* buf = (char*) malloc(buflen);
  char* bp = buf;

  if (argc != 2) {
    Tcl_AppendResult(interp, "Instrument handle missing", NULL);
    return TCL_ERROR;
  }
  scopeHandle = atol(argv[1]);

  size_t sum = 0;
  while (1) {
    status = viRead(scopeHandle, (ViBuf)bp, buflen, &returnCount);
    sum += returnCount;
    if ((status != VI_SUCCESS) && (status != VI_SUCCESS_MAX_CNT)) {
      free(buf);
      snprintf(statusTxt, STATUS_TXT_LEN, "%d", status);
      Tcl_AppendResult(interp, "VISA read error: ", &statusTxt, NULL);
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
        Tcl_AppendResult(interp, "Out of memory", NULL);
        return TCL_ERROR;
      }
      buf = newbuf;
    }
  }

  buf[sum] = 0; //terminate the string properly
  Tcl_SetObjResult(interp, Tcl_NewByteArrayObj((const unsigned char*)buf, sum));
  
  return TCL_OK;
}

int SetAttribute(int notUsed, Tcl_Interp *interp, int argc, char **argv) {
  ViStatus status = VI_SUCCESS;
  char statusTxt[STATUS_TXT_LEN];

  ViSession scopeHandle;
  int attr, value;

  if (argc != 4) {
    Tcl_AppendResult(interp, "Instrument handle, attribute type or attribute value missing", NULL);
    return TCL_ERROR;
  }
  scopeHandle = atol(argv[1]);
  attr = atol(argv[2]);
  value = atol(argv[3]);

  status = viSetAttribute(scopeHandle, (ViAttr) attr, (ViAttrState) value);
  if (status != VI_SUCCESS) {
    snprintf(statusTxt, STATUS_TXT_LEN, "%d", status);
    Tcl_AppendResult(interp, "VISA set attribute error: ", &statusTxt, NULL);
    return TCL_ERROR;
  }

  return TCL_OK;
}

int GetAttribute(int notUsed, Tcl_Interp *interp, int argc, char **argv) {
  ViStatus status = VI_SUCCESS;
  char statusTxt[STATUS_TXT_LEN];

  ViSession scopeHandle;
  int attr;

  int64_t value = 0;

  if (argc != 3) {
    Tcl_AppendResult(interp, "Instrument handle or attribute type missing", NULL);
    return TCL_ERROR;
  }
  scopeHandle = atol(argv[1]);
  attr = atol(argv[2]);

  status = viGetAttribute(scopeHandle, (ViAttr) attr, &value);
  if (status != VI_SUCCESS) {
    snprintf(statusTxt, STATUS_TXT_LEN, "%d", status);
    Tcl_AppendResult(interp, "VISA get attribute error: ", &statusTxt, NULL);
    return TCL_ERROR;
  }

  Tcl_SetObjResult(interp, Tcl_NewLongObj((long) value));

  return TCL_OK;
}

int SetTimeout(int notUsed, Tcl_Interp *interp, int argc, char **argv) {
  ViStatus status = VI_SUCCESS;
  char statusTxt[STATUS_TXT_LEN];

  ViSession scopeHandle;
  int timeout;

  if (argc != 3) {
    Tcl_AppendResult(interp, "Instrument handle or timeout missing", NULL);
    return TCL_ERROR;
  }
  scopeHandle = atol(argv[1]);
  timeout = atol(argv[2]);

  status = viSetAttribute(scopeHandle, VI_ATTR_TMO_VALUE, (ViAttrState) timeout);
  if (status != VI_SUCCESS) {
    snprintf(statusTxt, STATUS_TXT_LEN, "%d", status);
    Tcl_AppendResult(interp, "VISA set timeout error: ", &statusTxt, NULL);
    return TCL_ERROR;
  }

  return TCL_OK;
}

int GetTimeout(int notUsed, Tcl_Interp *interp, int argc, char **argv) {
  ViStatus status = VI_SUCCESS;
  char statusTxt[STATUS_TXT_LEN];

  ViSession scopeHandle;

  int64_t timeout = 0;

  if (argc != 2) {
    Tcl_AppendResult(interp, "Instrument handle missing", NULL);
    return TCL_ERROR;
  }
  scopeHandle = atol(argv[1]);

  status = viGetAttribute(scopeHandle, VI_ATTR_TMO_VALUE, &timeout);
  if (status != VI_SUCCESS) {
    snprintf(statusTxt, STATUS_TXT_LEN, "%d", status);
    Tcl_AppendResult(interp, "VISA get timeout error: ", &statusTxt, NULL);
    return TCL_ERROR;
  }

  Tcl_SetObjResult(interp, Tcl_NewLongObj((long) timeout));

  return TCL_OK;
}



int Rsvisa_Init(Tcl_Interp *Interp) {
  // create namespace
  if (Tcl_CreateNamespace(Interp, NS_PREFIX, NULL, NULL) == NULL) {
    return TCL_ERROR;
  }

  // initialize operation
  Tcl_CreateCommand (Interp, NS_PREFIX "GetResourceManager", (Tcl_CmdProc *)GetResourceManager, 0, 0);
  Tcl_CreateCommand (Interp, NS_PREFIX "OpenResource",       (Tcl_CmdProc *)OpenResource, 0, 0);
  Tcl_CreateCommand (Interp, NS_PREFIX "CloseResource",      (Tcl_CmdProc *)CloseResource, 0, 0);
  Tcl_CreateCommand (Interp, NS_PREFIX "Idn",                (Tcl_CmdProc *)Idn, 0, 0);
  Tcl_CreateCommand (Interp, NS_PREFIX "Stb",                (Tcl_CmdProc *)Stb, 0, 0);
  Tcl_CreateCommand (Interp, NS_PREFIX "WriteRead",          (Tcl_CmdProc *)WriteRead, 0, 0);
  Tcl_CreateCommand (Interp, NS_PREFIX "WriteReadBin",       (Tcl_CmdProc *)WriteReadBin, 0, 0);
  Tcl_CreateCommand (Interp, NS_PREFIX "Write",              (Tcl_CmdProc *)Write, 0, 0);
  Tcl_CreateCommand (Interp, NS_PREFIX "Read",               (Tcl_CmdProc *)Read, 0, 0);
  Tcl_CreateCommand (Interp, NS_PREFIX "SetAttribute",       (Tcl_CmdProc *)SetAttribute, 0, 0);
  Tcl_CreateCommand (Interp, NS_PREFIX "GetAttribute",       (Tcl_CmdProc *)GetAttribute, 0, 0);
  Tcl_CreateCommand (Interp, NS_PREFIX "SetTimeout",         (Tcl_CmdProc *)SetTimeout, 0, 0);
  Tcl_CreateCommand (Interp, NS_PREFIX "GetTimeout",         (Tcl_CmdProc *)GetTimeout, 0, 0);

  // provide package
  if (Tcl_PkgProvide(Interp, "rsvisa", "1.0.0") == TCL_ERROR) {
    return TCL_ERROR;
  }
  
  return TCL_OK;
}

int Rsvisa_Unload(Tcl_Interp *Interp, int flags) {
  // destroy operation.
  return TCL_OK;
}

