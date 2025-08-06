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
		
		ULONG m_somedata;
		auto_unicode_string m_canonical_file_path;
		auto_unicode_string m_canonical_file_path2;
		auto_unicode_string m_canonical_file_path3;
		auto_unicode_string m_canonical_file_path4;
		auto_unicode_string m_canonical_file_path5;
		auto_unicode_string m_canonical_file_path6;
		auto_unicode_string m_canonical_file_path7;

		SERIALIZE_MAP_BEGIN()
			SERIALIZE_MAP_ENTRY_CHAR(&m_canonical_file_path._str,0)
			SERIALIZE_MAP_ENTRY_CHAR(&m_canonical_file_path2._str, 0)
			SERIALIZE_MAP_ENTRY_CHAR(&m_canonical_file_path3._str, 0)
			SERIALIZE_MAP_ENTRY_CHAR(&m_canonical_file_path4._str, 0)
			SERIALIZE_MAP_ENTRY_CHAR(&m_canonical_file_path5._str, 0)
			SERIALIZE_MAP_ENTRY_CHAR(&m_canonical_file_path6._str, 0)
			SERIALIZE_MAP_ENTRY_CHAR(&m_canonical_file_path7._str, 0)
		SERIALIZE_MAP_END()
	};
}
