

#ifndef THREAD_H
#define THREAD_H

#include "../platform.h"

class Thread
{
public:
	bool quit;
	SDL_mutex* mutex;
	SDL_Thread* handle;
};

#define THREADS	8

extern Thread g_thread[THREADS];

int32_t ThreadFun(void* param);

#endif