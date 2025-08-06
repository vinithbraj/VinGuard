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

#include "VinGuardKSerializer.h"

namespace VinGuard {
	struct filter_message : serializer
	{
		filter_message() {
			SERIALIZE_INIT()
		}
		
		ULONG file_action;
		ULONG filter_verdict;
		ULONG instance_cookie;
		auto_unicode_string m_canonical_file_path;

		SERIALIZE_MAP_BEGIN()
			SERIALIZE_MAP_ENTRY_POD(ULONG, file_action)
			SERIALIZE_MAP_ENTRY_POD(ULONG, filter_verdict)
			SERIALIZE_MAP_ENTRY_POD(ULONG, instance_cookie)
			SERIALIZE_MAP_ENTRY_CHAR(&m_canonical_file_path._str,0)
		SERIALIZE_MAP_END()
	};
}
