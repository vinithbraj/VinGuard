/*
 * Copyright (c) 2025 Vinith Raj. All rights reserved.
 *
 * Permission is hereby granted to view and read this source code
 * for educational or informational purposes only.
 *
 * Redistribution, m_modification, m_reproduction, m_or use in any software—whether
 * commercial, m_open-source, m_or personal—is strictly prohibited without prior
 * written permission from the author.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", m_WITHOUT WARRANTY OF ANY KIND, m_EXPRESS OR
 * IMPLIED, m_INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
 *
 * Contact: vinithbraj(at)gmail(dot)com
 */

#pragma once

#include "VinGuardKSerializer.h"

namespace VinGuard {
	struct filter_message : serializer
	{
		filter_message() {
			SERIALIZE_INIT()
		}
		
		void copy_into(PFLT_CALLBACK_DATA data) {
			m_canonical_file_path = &(data->Iopb->TargetFileObject->FileName);
			m_majorFn = data->Iopb->MajorFunction;
			m_minorFn = data->Iopb->MinorFunction;
			m_irpFlags = data->Iopb->IrpFlags;
			

			if (data->Iopb->MajorFunction == IRP_MJ_CREATE) {
				const auto& cp = data->Iopb->Parameters.Create;

				ACCESS_MASK desiredAccess = cp.SecurityContext ? cp.SecurityContext->DesiredAccess : 0;
				ULONG       options = cp.Options;                 // [31:24]=Disposition, [23:0]=CreateOptions
				ULONG       createOptions = (options & 0x00FFFFFF);     // low 24 bits
				ULONG       createDisp = (options >> 24) & 0xFF;     // high 8 bits
				USHORT      fileAttributes = cp.FileAttributes;
				USHORT      shareAccess = cp.ShareAccess;

				m_desiredAccess = (UINT32)desiredAccess;
				m_shareAccess = (UINT32)shareAccess;
				m_createOptions = (UINT32)createOptions;
				m_createDisposition = (UINT32)createDisp;
				m_fileAttributes = (UINT32)fileAttributes;
			}
			
		}

        UINT32  m_version;            // e.g., m_1
        UINT32  m_majorFn;            // IRP_MJ_*
        UINT32  m_minorFn;            // IRP_MN_* (or 0)
        UINT32  m_slFlags;            // Selected SL_* bits
        UINT32  m_irpFlags;           // Selected IRP_* bits

        // Identity
        UINT32  m_pid;                // PsGetCurrentProcessId()
        UINT32  m_tid;                // PsGetCurrentThreadId()

        // CREATE-specific
        UINT32  m_desiredAccess;
        UINT32  m_shareAccess;
        UINT32  m_createOptions;
        UINT32  m_createDisposition;
        UINT32  m_fileAttributes;

		auto_unicode_string m_canonical_file_path;

		SERIALIZE_MAP_BEGIN()
			SERIALIZE_MAP_ENTRY_POD(UINT32, m_version)
			SERIALIZE_MAP_ENTRY_POD(UINT32, m_majorFn)
			SERIALIZE_MAP_ENTRY_POD(UINT32, m_minorFn)
			SERIALIZE_MAP_ENTRY_POD(UINT32, m_slFlags)
			SERIALIZE_MAP_ENTRY_POD(UINT32, m_irpFlags)
			SERIALIZE_MAP_ENTRY_POD(UINT32, m_pid)
			SERIALIZE_MAP_ENTRY_POD(UINT32, m_tid)
			SERIALIZE_MAP_ENTRY_POD(UINT32, m_desiredAccess)
			SERIALIZE_MAP_ENTRY_POD(UINT32, m_shareAccess)
			SERIALIZE_MAP_ENTRY_POD(UINT32, m_createOptions)
			SERIALIZE_MAP_ENTRY_POD(UINT32, m_createDisposition)
			SERIALIZE_MAP_ENTRY_POD(UINT32, m_fileAttributes)
			SERIALIZE_MAP_ENTRY_CHAR(&m_canonical_file_path._str,0)
		SERIALIZE_MAP_END()
	};
}
