#pragma once

#define di(fmt, ...) \
    DbgPrintEx(DPFLTR_DEFAULT_ID, DPFLTR_INFO_LEVEL, "%s: " fmt, __FUNCTION__, __VA_ARGS__)

#define de(fmt, ...) \
    DbgPrintEx(DPFLTR_DEFAULT_ID, DPFLTR_ERROR_LEVEL, "%s: " fmt, __FUNCTION__, __VA_ARGS__)

