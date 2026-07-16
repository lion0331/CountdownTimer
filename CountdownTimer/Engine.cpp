// Engine.cpp - 倒计时引擎 (FILETIME绝对时间, 不受睡眠影响)
#include "Engine.h"

// 获取当前绝对时间 (ms) — 基于 GetSystemTimeAsFileTime，睡眠后也正确
static ULONGLONG NowMs()
{
	FILETIME ft;
	GetSystemTimeAsFileTime(&ft);
	return ((ULONGLONG)ft.dwHighDateTime << 32 | ft.dwLowDateTime) / 10000ULL;
}

void CountdownEngine::SetDuration(int h, int m, int s)
{
	totalMs     = ((h * 3600ULL) + (m * 60ULL) + s) * 1000ULL;
	remainingMs = totalMs;
	targetTick  = 0;
	targetDate  = {};
	running      = false;
	finished     = false;
}

void CountdownEngine::SetTargetDate(const SYSTEMTIME& stLocal)
{
	FILETIME ftLocalAsUtc = {};
	SystemTimeToFileTime(&stLocal, &ftLocalAsUtc);
	ULARGE_INTEGER ul;
	ul.LowPart  = ftLocalAsUtc.dwLowDateTime;
	ul.HighPart = ftLocalAsUtc.dwHighDateTime;

	TIME_ZONE_INFORMATION tzi = {};
	GetTimeZoneInformation(&tzi);
	LONGLONG delta = (LONGLONG)tzi.Bias * 600000000LL;
	ul.QuadPart += delta;

	targetDate.dwLowDateTime  = ul.LowPart;
	targetDate.dwHighDateTime = ul.HighPart;

	ULONGLONG now = NowMs();
	ULONGLONG tgt = ul.QuadPart / 10000ULL;
	if (tgt > now) { totalMs = tgt - now; remainingMs = totalMs; }
	else           { totalMs = 0; remainingMs = 0; }
	targetTick = 0;
	running = false;
	finished = false;
}

void CountdownEngine::RestoreFromRemainingMs(ULONGLONG ms)
{
	totalMs     = ms;
	remainingMs = ms;
	targetTick  = 0;
	targetDate  = {};
	running      = false;
	finished     = false;
}

void CountdownEngine::Start()
{
	if (finished || remainingMs == 0) return;

	if (targetDate.dwHighDateTime != 0 || targetDate.dwLowDateTime != 0)
	{
		ULARGE_INTEGER ut;
		ut.LowPart  = targetDate.dwLowDateTime;
		ut.HighPart = targetDate.dwHighDateTime;
		ULONGLONG now = NowMs();
		ULONGLONG tgt = ut.QuadPart / 10000ULL;
		if (tgt > now) { remainingMs = tgt - now; totalMs = remainingMs; }
		else           { remainingMs = 0; finished = true; return; }
	}

	targetTick = NowMs() + remainingMs;  // 绝对时间, 不受睡眠影响
	running = true;
}

void CountdownEngine::Pause()
{
	running = false;
	if (targetTick > 0)
	{
		ULONGLONG now = NowMs();
		remainingMs = (targetTick > now) ? (targetTick - now) : 0;
		if (remainingMs == 0) finished = true;
	}
}

void CountdownEngine::Reset()
{
	remainingMs = totalMs;
	running     = false;
	finished    = false;
	targetTick  = 0;
}

void CountdownEngine::Tick()
{
	if (!running) return;
	ULONGLONG now = NowMs();
	if (now >= targetTick) { remainingMs = 0; running = false; finished = true; }
	else                    { remainingMs = targetTick - now; }
}

void CountdownEngine::GetDisplay(int& days, int& hours, int& minutes, int& seconds) const
{
	ULONGLONG secs = remainingMs / 1000;
	days    = (int)(secs / 86400);
	hours   = (int)((secs % 86400) / 3600);
	minutes = (int)((secs % 3600) / 60);
	seconds = (int)(secs % 60);
}
