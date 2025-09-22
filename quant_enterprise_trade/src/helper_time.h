#ifndef _HELPER_TIME_H
#define _HELPER_TIME_H

#include <time.h>
#include <chrono>
#include <string>
/*
作者：赵志根
微信：401985690
qq群号：450286917
*/

namespace zutil
{

	using namespace std;

	// (int)yyyymmdd
	inline int GetToday()
	{
		time_t t = time(NULL);
		struct tm *tm = localtime(&t);
		return (tm->tm_year + 1900) * 10000 + (tm->tm_mon + 1) * 100 + tm->tm_mday;
	}

	// (int)HHMMSSsss
	inline uint32_t GetNowMilliTime()
	{
		char now_time[15];

		time_t t = time(NULL);
		struct tm *tm = localtime(&t);
		strftime(now_time, 15, "%H%M%S", tm);
		return atoi(now_time) * 1000; // FIXME: only to second
	}

	// (uint32_t)HHMMSS
	inline uint32_t GetNowSedondTime()
	{
		char now_time[15];

		time_t t = time(NULL);
		struct tm *tm = localtime(&t);
		strftime(now_time, 15, "%H%M%S", tm);
		return atoi(now_time);
	}

	// (string)HHMMSS
	inline string GetStrSecondTime()
	{
		char now_time[15];

		time_t t = time(NULL);
		struct tm *tm = localtime(&t);
		strftime(now_time, 15, "%H%M%S", tm);
		return now_time;
	}

	// (int)nano seconds count
	inline int64_t GetTimestamp()
	{
		return chrono::steady_clock::now().time_since_epoch().count();
	}

	// (int)yymmdd, (int)HHMMSSsss -> (int)(int)yyyymmddHHMMSSsss
	inline int64_t GetDateTime(const uint64_t date_int, const uint64_t time_int)
	{
		return date_int * 1000000000 + time_int;
	}

	// (int)yyyymmddHHMMSSsss -> (int)yyyymmdd
	inline uint32_t GetIntDate(const int64_t time_int)
	{
		return time_int / 1000000000;
	}

	// (int)yyyymmddHHMMSSsss -> HHMMSSsss
	inline uint32_t GetIntTime(const int64_t time_int)
	{
		return time_int % 1000000000;
	}

	// HH:MM:SS and ms -> HHMMSSsss
	inline uint32_t GetIntTime(const char *time_str, int ms)
	{
		uint32_t hh, mm, ss;
		sscanf(time_str, "%d:%d:%d", &hh, &mm, &ss);
		return (hh * 10000 + mm * 100 + ss) * 1000 + ms;
	}

	// HH:MM:SS.sss and ms -> HHMMSSsss
	inline uint32_t GetIntTime(const char *time_str)
	{
		uint32_t hh, mm, ss, ms;
		sscanf(time_str, "%d:%d:%d.%d", &hh, &mm, &ss, &ms);
		return (hh * 10000 + mm * 100 + ss) * 1000 + ms;
	}

}; // namespace zutil

#endif
