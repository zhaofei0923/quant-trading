#ifndef _STRATEGY_TOOLS_H
#define _STRATEGY_TOOLS_H

#include <chrono>
#include <string>

// Utility helpers for strategy-side price conversions
// Keep consistent with client-side implementation to avoid precision drift.

namespace zutil
{
	using namespace std;

	// Convert a double price to integer representation using multiplier (e.g., 1000)
	// dValue: price in double
	// iMULTIPLE: conversion multiplier, e.g., 1000
	// Logic mirrors client/helper_tools.h to mitigate floating-point rounding issues
	inline unsigned long long ConvertDoubleToInt(double dValue, int iMULTIPLE)
	{
		unsigned long long iRet = 0;
		// Guard against cases like 19.9 * 1000 -> 19899.9 due to FP representation
		iRet = static_cast<unsigned long long>(dValue * iMULTIPLE + 1);
		// Normalize to nearest 10 (as in client implementation)
		iRet = (iRet / 10) * 10;

		return iRet;
	}

}; // namespace zutil

#endif

