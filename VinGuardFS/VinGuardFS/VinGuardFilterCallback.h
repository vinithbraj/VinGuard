#pragma once

extern "C"
{
#include <fltKernel.h>
#pragma comment(lib, "fltmgr.lib")
}

class VinGuardFilterCallback {

public:

    static const FLT_OPERATION_REGISTRATION Callbacks[];

    static FLT_PREOP_CALLBACK_STATUS PreCreateCallback(PFLT_CALLBACK_DATA Data, PCFLT_RELATED_OBJECTS FltObjects, PVOID* CompletionContext)
    {
        UNREFERENCED_PARAMETER(CompletionContext);
        UNREFERENCED_PARAMETER(FltObjects);

        if (Data->Iopb && Data->Iopb->TargetFileObject && Data->Iopb->TargetFileObject->FileName.Buffer) {
            DbgPrint("AVMinifilter: File open: %wZ\n", &Data->Iopb->TargetFileObject->FileName);
        }

        return FLT_PREOP_SUCCESS_NO_CALLBACK;
    }

};

const FLT_OPERATION_REGISTRATION VinGuardFilterCallback::Callbacks[] = {
     { IRP_MJ_CREATE, 0, VinGuardFilterCallback::PreCreateCallback, NULL },
     { IRP_MJ_OPERATION_END }
};
