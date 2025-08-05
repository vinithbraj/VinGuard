#pragma once

#include "VinGuardKSerializer.h"

namespace VinGuard {
	struct filter_message : serializer
	{
		ULONG m_somedata;
		SERIALIZE_MAP_BEGIN()
			SERIALIZE_MAP_ENTRY_POD(ULONG, m_somedata)
		SERIALIZE_MAP_END()
	};
}
