/*
 * Copyright (c) 2025 Vinith Raj. All rights reserved.
 *
 * Permission is hereby granted to view and read this source code
 * for educational or informational purposes only.
 *
 * Redistribution, modification, reproduction, or use in any software—whether
 * commercial, open-source, or personal—is strictly prohibited without prior
 * written permission from the author.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
 *
 * Contact: vinithbraj(at)gmail(dot)com
 */

#pragma once

#include "VinGuardUSerializer.h"
namespace VinGuard {
	
	const size_t filename_buffer_size = 1024;
	struct filter_message : serializer
	{
		ULONG m_file_action;
		ULONG m_filter_verdict;
		ULONG m_instance_cookie;
		WCHAR m_filename[filename_buffer_size];

		filter_message():
			m_file_action(0),
			m_filter_verdict(0),
			m_instance_cookie(0)
		{
			SERIALIZE_INIT();
			ZeroMemory(&m_filename, filename_buffer_size);
		}

		SERIALIZE_MAP_BEGIN()
			SERIALIZE_MAP_ENTRY_POD(ULONG, m_file_action)
			SERIALIZE_MAP_ENTRY_POD(ULONG, m_filter_verdict)
			SERIALIZE_MAP_ENTRY_POD(ULONG, m_instance_cookie)
			SERIALIZE_MAP_ENTRY_CHAR(m_filename, sizeof(m_filename))
		SERIALIZE_MAP_END()
	};
}
