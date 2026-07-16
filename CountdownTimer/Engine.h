// Engine.h - 倒计时引擎 (统一使用 GetTickCount64 绝对时间)
#pragma once
#include "Common.h"

struct CountdownEngine
{
	ULONGLONG targetTick = 0;   // GetTickCount64 目标 (ms)
	ULONGLONG totalMs = 0;   // 总毫秒数
	ULONGLONG remainingMs = 0;   // 当前剩余毫秒数
	bool      running = false;
	bool      finished = false;
	FILETIME  targetDate = {};  // 日期模式的目标绝对时间 (用于跨时钟一致性)

	void SetDuration(int h, int m, int s);
	void SetTargetDate(const SYSTEMTIME& st);
	void RestoreFromRemainingMs(ULONGLONG ms);  // 启动恢复 — 用保存的剩余毫秒数
	void Start();
	void Pause();
	void Reset();
	void Tick();   // 重新计算 remainingMs = targetTick - GetTickCount64()
	void GetDisplay(int& days, int& hours, int& minutes, int& seconds) const;

	bool      IsFinished()   const
	{
		return finished;
	}
	bool      IsRunning()    const
	{
		return running;
	}
	ULONGLONG RemainingMs()  const
	{
		return remainingMs;
	}
};
