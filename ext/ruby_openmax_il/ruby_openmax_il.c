#include <stdio.h>
#include <stdlib.h>

#include <ruby.h>

#include <bcm_host.h>
#include <OMX_Core.h>
#include <OMX_Component.h>

VALUE cOpenMaxIL;

const char *err2str(int err) {
    switch (err) {
    case OMX_ErrorInsufficientResources: return "OMX_ErrorInsufficientResources";
    case OMX_ErrorUndefined: return "OMX_ErrorUndefined";
    case OMX_ErrorInvalidComponentName: return "OMX_ErrorInvalidComponentName";
    case OMX_ErrorComponentNotFound: return "OMX_ErrorComponentNotFound";
    case OMX_ErrorInvalidComponent: return "OMX_ErrorInvalidComponent";
    case OMX_ErrorBadParameter: return "OMX_ErrorBadParameter";
    case OMX_ErrorNotImplemented: return "OMX_ErrorNotImplemented";
    case OMX_ErrorUnderflow: return "OMX_ErrorUnderflow";
    case OMX_ErrorOverflow: return "OMX_ErrorOverflow";
    case OMX_ErrorHardware: return "OMX_ErrorHardware";
    case OMX_ErrorInvalidState: return "OMX_ErrorInvalidState";
    case OMX_ErrorStreamCorrupt: return "OMX_ErrorStreamCorrupt";
    case OMX_ErrorPortsNotCompatible: return "OMX_ErrorPortsNotCompatible";
    case OMX_ErrorResourcesLost: return "OMX_ErrorResourcesLost";
    case OMX_ErrorNoMore: return "OMX_ErrorNoMore";
    case OMX_ErrorVersionMismatch: return "OMX_ErrorVersionMismatch";
    case OMX_ErrorNotReady: return "OMX_ErrorNotReady";
    case OMX_ErrorTimeout: return "OMX_ErrorTimeout";
    case OMX_ErrorSameState: return "OMX_ErrorSameState";
    case OMX_ErrorResourcesPreempted: return "OMX_ErrorResourcesPreempted";
    case OMX_ErrorPortUnresponsiveDuringAllocation: return "OMX_ErrorPortUnresponsiveDuringAllocation";
    case OMX_ErrorPortUnresponsiveDuringDeallocation: return "OMX_ErrorPortUnresponsiveDuringDeallocation";
    case OMX_ErrorPortUnresponsiveDuringStop: return "OMX_ErrorPortUnresponsiveDuringStop";
    case OMX_ErrorIncorrectStateTransition: return "OMX_ErrorIncorrectStateTransition";
    case OMX_ErrorIncorrectStateOperation: return "OMX_ErrorIncorrectStateOperation";
    case OMX_ErrorUnsupportedSetting: return "OMX_ErrorUnsupportedSetting";
    case OMX_ErrorUnsupportedIndex: return "OMX_ErrorUnsupportedIndex";
    case OMX_ErrorBadPortIndex: return "OMX_ErrorBadPortIndex";
    case OMX_ErrorPortUnpopulated: return "OMX_ErrorPortUnpopulated";
    case OMX_ErrorComponentSuspended: return "OMX_ErrorComponentSuspended";
    case OMX_ErrorDynamicResourcesUnavailable: return "OMX_ErrorDynamicResourcesUnavailable";
    case OMX_ErrorMbErrorsInFrame: return "OMX_ErrorMbErrorsInFrame";
    case OMX_ErrorFormatNotDetected: return "OMX_ErrorFormatNotDetected";
    case OMX_ErrorContentPipeOpenFailed: return "OMX_ErrorContentPipeOpenFailed";
    case OMX_ErrorContentPipeCreationFailed: return "OMX_ErrorContentPipeCreationFailed";
    case OMX_ErrorSeperateTablesUsed: return "OMX_ErrorSeperateTablesUsed";
    case OMX_ErrorTunnelingUnsupported: return "OMX_ErrorTunnelingUnsupported";
    default: return "unknown error";
    }
}

static VALUE initialize() {
  OMX_ERRORTYPE err;

  bcm_host_init();

  err = OMX_Init();

  if (err != OMX_ErrorNone) {
    fprintf(stderr, "OMX_Init() failed %s\n", err2str(err));
    exit(1);
  }

  fprintf(stderr, "OMX_Init() success\n");

  return Qtrue;
}

static VALUE list_components() {
  OMX_ERRORTYPE err = OMX_ErrorNone;
  char name[OMX_MAX_STRINGNAME_SIZE];

  VALUE array = rb_ary_new();

  for (int i = 0; OMX_ErrorNoMore != err; i++) {
    err = OMX_ComponentNameEnum(name, OMX_MAX_STRINGNAME_SIZE, i);

    if (OMX_ErrorNone == err) {
      rb_ary_push(array, rb_str_new_cstr(name));
    } else if (OMX_ErrorNoMore == err) {
      break;
    } else {
      fprintf(stderr, "OMX_ComponentNameEnum() failed %s\n", err2str(err));
      exit(1);
    }
  }

  return array;
}

typedef struct ComponentHandle {
  OMX_HANDLETYPE handle;
  OMX_VERSIONTYPE version;
} ComponentHandle;

static void setHeader(OMX_PTR header, OMX_U32 size, ComponentHandle cHandle) {
  /* header->nVersion */
  OMX_VERSIONTYPE* ver;

  /* header->nSize */
  *((OMX_U32*)header) = size;
  ver = (OMX_VERSIONTYPE*)(header + sizeof(OMX_U32));

  ver->s.nVersionMajor = cHandle.version.s.nVersionMajor;
  ver->s.nVersionMinor = cHandle.version.s.nVersionMinor;
  ver->s.nRevision = cHandle.version.s.nRevision;
  ver->s.nStep = cHandle.version.s.nStep;
}

void getPortsRange(ComponentHandle cHandle, VALUE component_info, int portType) {
  OMX_PORT_PARAM_TYPE param;
  OMX_ERRORTYPE err = OMX_ErrorNone;
  OMX_INDEXTYPE paramIndex;

  setHeader(&param, sizeof(OMX_PORT_PARAM_TYPE), cHandle);

  switch (portType) {
  case 0:
    paramIndex = OMX_IndexParamAudioInit;
  break;
  case 1:
    paramIndex = OMX_IndexParamVideoInit;
  break;
  case 2:
    paramIndex = OMX_IndexParamImageInit;
  break;
  case 3:
    paramIndex = OMX_IndexParamOtherInit;
  break;
  default:
  break;
  }

  err = OMX_GetParameter(cHandle.handle, paramIndex, &param);
  if (err != OMX_ErrorNone) {
    fprintf(stderr, "Error in getting audio OMX_PORT_PARAM_TYPE parameter %s\n", err2str(err));
    exit(1);
  }
  printf("Audio ports start at %d, %d in total\n", param.nStartPortNumber, param.nPorts);
}

static VALUE get_component_info(VALUE self, VALUE name) {
  OMX_VERSIONTYPE compVersion;
  OMX_UUIDTYPE uid;
  OMX_CALLBACKTYPE callbacks;
  OMX_ERRORTYPE err = OMX_ErrorNone;
  VALUE component_info;
  VALUE compVersionR;
  VALUE specVersionR;
  VALUE unavailable = Qfalse;
  char fullName[OMX_MAX_STRINGNAME_SIZE];
  char *componentName;
  ComponentHandle cHandle;

  Check_Type(name, T_STRING);
  componentName = RSTRING_PTR(name);

  component_info = rb_hash_new();
  rb_hash_aset(component_info, rb_str_new_literal("name"), name);

  err = OMX_GetHandle(&cHandle.handle, componentName, NULL, &callbacks);
  if (err != OMX_ErrorNone) {
    fprintf(stderr, "OMX_GetHandle() failed %s\n", err2str(err));
    if (err == OMX_ErrorInsufficientResources) {
      unavailable = Qtrue;
    } else {
      exit(1);
    }
  }
  if (unavailable) {
    rb_hash_aset(component_info, rb_str_new_literal("available"), Qfalse);
  } else {
    rb_hash_aset(component_info, rb_str_new_literal("available"), Qtrue);

    err = OMX_GetComponentVersion(cHandle.handle, fullName, &compVersion, &cHandle.version, &uid);
    if (err != OMX_ErrorNone) {
      fprintf(stderr, "OMX_GetComponentVersion() failed %s\n", err2str(err));
      exit(1);
    }

    compVersionR = rb_hash_new();
    specVersionR = rb_hash_new();
    rb_hash_aset(compVersionR, rb_str_new_literal("major"), INT2FIX(compVersion.s.nVersionMajor));
    rb_hash_aset(compVersionR, rb_str_new_literal("minor"), INT2FIX(compVersion.s.nVersionMinor));
    rb_hash_aset(compVersionR, rb_str_new_literal("revision"), INT2FIX(compVersion.s.nRevision));
    rb_hash_aset(compVersionR, rb_str_new_literal("step"), INT2FIX(compVersion.s.nStep));
    rb_hash_aset(specVersionR, rb_str_new_literal("major"), INT2FIX(cHandle.version.s.nVersionMajor));
    rb_hash_aset(specVersionR, rb_str_new_literal("minor"), INT2FIX(cHandle.version.s.nVersionMinor));
    rb_hash_aset(specVersionR, rb_str_new_literal("revision"), INT2FIX(cHandle.version.s.nRevision));
    rb_hash_aset(specVersionR, rb_str_new_literal("step"), INT2FIX(cHandle.version.s.nStep));

    rb_hash_aset(component_info, rb_str_new_literal("componentVersion"), compVersionR);
    rb_hash_aset(component_info, rb_str_new_literal("specVersion"), specVersionR);
    rb_hash_aset(component_info, rb_str_new_literal("fullName"), rb_str_new_cstr(fullName));
    rb_hash_aset(component_info, rb_str_new_literal("UUID"), rb_str_new_cstr((const char *)uid));

    getPortsRange(cHandle, component_info, 0);
  }
  return component_info;
}

static VALUE clean_up() {
  return Qtrue;
}

void Init_ruby_openmax_il(void) {
  cOpenMaxIL = rb_const_get(rb_cObject, rb_intern("OpenMaxIL"));

  //rb_define_alloc_func(cMyMalloc, my_malloc_alloc);
  rb_define_method(cOpenMaxIL, "initialize", initialize, 0);
  rb_define_method(cOpenMaxIL, "list_components", list_components, 0);
  rb_define_method(cOpenMaxIL, "get_component_info", get_component_info, 1);
  rb_define_method(cOpenMaxIL, "clean_up", clean_up, 0);
}
