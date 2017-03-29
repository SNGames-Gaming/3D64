#pragma once

#ifndef CORE_RSP_HLE_H_
#define CORE_RSP_HLE_H_

#include "Memory.h"

// true if it returns as HLE or LLE
inline bool RSP_IsRunning(){ return (Memory_SP_GetRegister( SP_STATUS_REG ) & SP_STATUS_HALT) == 0; }

enum EProcessResult
{
	PR_NOT_STARTED,	// Couldn't start
	PR_STARTED,		// Was started asynchronously, active
	PR_COMPLETED,	// Was processed synchronously, completed
};

void RSP_HLE_ProcessTask();

#endif // CORE_RSP_HLE_H_
