// Engine.cpp - 倒计时引擎实现 (统一使用绝对时间 Tick)
#include "Engine.h"

void CountdownEngine::SetDuration(int h, int m, int s)
{
	totalMs = ((h * 3600ULL) + (m * 60ULL) + s) * 1000ULL;
	remainingMs = totalMs;
	targetTick = 0;
	targetDate = {};
	running = false;
	finished = false;
}

void CountdownEngine::SetTargetDate(const SYSTEMTIME& stLocal)
{
	// 用户输入的是本地时间。SystemTimeToFileTime 总是把 SYSTEMTIME 当 UTC。
	// 正确的转换：先 SystemTimeToFileTime → 再加时区偏差
	FILETIME ftLocalAsUtc = {};
	SystemTimeToFileTime(&stLocal, &ftLocalAsUtc);
	ULARGE_INTEGER ul;
	ul.LowPart = ftLocalAsUtc.dwLowDateTime;
	ul.HighPart = ftLocalAsUtc.dwHighDateTime;

	// UTC 偏差 (分钟)：UTC = Local + Bias。中国 Bias=-480。
	TIME_ZONE_INFORMATION tzi = {};
	GetTimeZoneInformation(&tzi);
	// 100ns 单位: 1 分钟 = 600,000,000 个 100ns
	LONGLONG delta = (LONGLONG)tzi.Bias * 600000000LL;
	ul.QuadPart += delta;  // stLocal 的正确 UTC 值

	targetDate.dwLowDateTime = ul.LowPart;
	targetDate.dwHighDateTime = ul.HighPart;

	// 计算当前与目标的差值
	FILETIME ftNow = {};
	GetSystemTimeAsFileTime(&ftNow);
	ULARGE_INTEGER un;
	un.LowPart = ftNow.dwLowDateTime;
	un.HighPart = ftNow.dwHighDateTime;

	if (ul.QuadPart > un.QuadPart)
	{
		totalMs = (ul.QuadPart - un.QuadPart) / 10000ULL;
		remainingMs = totalMs;
	}
	else
	{
		totalMs = 0;
		remainingMs = 0;
	}
	targetTick = 0;
	running = false;
	finished = false;
}

void CountdownEngine::RestoreFromRemainingMs(ULONGLONG ms)
{
	// 用于启动时恢复时长模式 — 直接用保存的剩余毫秒
	totalMs = ms;
	remainingMs = ms;
	targetTick = 0;
	targetDate = {};
	running = false;
	finished = false;
}

void CountdownEngine::Start()
{
	if (!finished && remainingMs > 0)
	{
		// 如果有 targetDate，重新基于当前绝对时间计算 remainingMs
		if (targetDate.dwHighDateTime != 0 || targetDate.dwLowDateTime != 0)
		{
			FILETIME ftNow = {};
			GetSystemTimeAsFileTime(&ftNow);
			ULARGE_INTEGER ut{}, un{};
			ut.LowPart = targetDate.dwLowDateTime; ut.HighPart = targetDate.dwHighDateTime;
			un.LowPart = ftNow.dwLowDateTime;       un.HighPart = ftNow.dwHighDateTime;
			if (ut.QuadPart > un.QuadPart)
			{
				remainingMs = (ut.QuadPart - un.QuadPart) / 10000ULL;
				totalMs = remainingMs;
			}
			else
			{
				remainingMs = 0; finished = true; return;
			}
		}
		targetTick = GetTickCount64() + remainingMs;
		running = true;
	}
}

void CountdownEngine::Pause()
{
	running = false;
	// 暂停时保留精确剩余 (以防 Tick 还没进来)
	if (targetTick > 0)
	{
		ULONGLONG now = GetTickCount64();
		remainingMs = (targetTick > now) ? (targetTick - now) : 0;
		if (remainingMs == 0) finished = true;
	}
}

void CountdownEngine::Reset()
{
	remainingMs = totalMs;
	running = false;
	finished = false;
	targetTick = 0;
}

void CountdownEngine::Tick()
{
	if (!running) return;
	ULONGLONG now = GetTickCount64();
	if (now >= targetTick)
	{
		remainingMs = 0;
		running = false;
		finished = true;
	}
	else
	{
		remainingMs = targetTick - now;
	}
}

void CountdownEngine::GetDisplay(int& days, int& hours, int& minutes, int& seconds) const
{
	ULONGLONG secs = remainingMs / 1000;
	days = (int)(secs / 86400);
	hours = (int)((secs % 86400) / 3600);
	minutes = (int)((secs % 3600) / 60);
	seconds = (int)(secs % 60);
}