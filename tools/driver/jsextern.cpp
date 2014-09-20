/**
 * jslang, A Javascript AOT compiler base on LLVM
 *
 * Copyright (c) 2014 Eddid Zhang <zhangheng607@163.com>
 * All Rights Reserved.
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 2.0 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 */

/**
 * write some platform dependcy function in jsextern.c
 * addGlobalMapping when run in JIT mode
 * link with jsextern object when compile an AOT binary
 */

#include <stdio.h>
#include "ejs-value.h"
#if defined(WIN32)
#include <Windows.h>
#include <intrin.h>
#pragma comment(lib, "kernel32.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "Winmm.lib")
#else
#include <unistd.h>
#include <sys/time.h>
#include <sys/mman.h>
#endif

extern "C" {

void jsextern_print_tick()
{
	static DWORD msLastTime;
	DWORD msTime = timeGetTime();
	printf("---%d(ms)---\n", msLastTime ? msTime - msLastTime : 0);
	msLastTime = msTime;
}

void jsextern_os_msleep(int mSecond)
{
#if defined(WIN32)
    ::Sleep(mSecond);
#else
    usleep(mSecond * 1000);
#endif
}

#if 0
#if defined(WIN32)
/*
 * Implementation of mmap()/munmap() replacement for Windows.
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#define PROT_READ   0x0001
#define PROT_WRITE  0x0002
#define PROT_EXEC   0x0004
#define PROT_NONE   0x0008

#define MAP_SHARED  0x0001
#define MAP_PRIVATE 0x0002
#define MAP_FIXED   0x0004
#define MAP_ANONYMOUS 0x20
#define MAP_ANON      MAP_ANONYMOUS

#define MAP_FAILED  ((void *)-1)

typedef struct {
    unsigned int prot_flag;
    DWORD        win_flag;
} Protection_Scheme_t;

typedef struct _MapList_t {
    HANDLE             hMap;
    void              *Base;
    struct _MapList_t *Next;
} MapList_t;

static const Protection_Scheme_t Protection_Scheme[] = {
    { PROT_READ,                      PAGE_READONLY          },
    { PROT_READ|PROT_WRITE,           PAGE_READWRITE         },
    { PROT_READ|PROT_WRITE|PROT_EXEC, PAGE_EXECUTE_READWRITE },
    { PROT_EXEC,                      PAGE_EXECUTE           },
    { PROT_READ|PROT_EXEC,            PAGE_EXECUTE_READ      },
};

static MapList_t *MapList = NULL;

static void *mmap(unsigned int address,
                  unsigned int size,
                  unsigned int protection,
                  unsigned int flags,
                  int          fd,
                  int          offset)
{
  HANDLE       hFile, hMapFile;
  DWORD        dwProtect, dwAccess;
  void        *Base;
  MapList_t   *Item;
  unsigned int x;

  /* Check if fd is valid */
  if (fd == -1)
    return MAP_FAILED;

  /* Retrieve system handle from fd */
  hFile = (HANDLE)_get_osfhandle(fd);
  if (hFile == INVALID_HANDLE_VALUE)
    return MAP_FAILED;

  /* Search protection schemes */
  for (dwProtect=PAGE_NOACCESS, x=0;
       x < (sizeof(Protection_Scheme)/sizeof(Protection_Scheme_t));
       x++)
  {
    if (Protection_Scheme[x].prot_flag == protection)
    {
      dwProtect = Protection_Scheme[x].win_flag;
      break;
    }
  }

  if (flags & MAP_PRIVATE) {
    dwAccess = FILE_MAP_COPY;
    dwProtect = PAGE_WRITECOPY;
  } else
  if ((protection & PROT_WRITE))
    dwAccess = FILE_MAP_WRITE;
  else
    dwAccess = FILE_MAP_READ;

  /* Create mapping object */
  hMapFile = CreateFileMapping(hFile, NULL, dwProtect, 0, size, NULL);
  if (hMapFile == INVALID_HANDLE_VALUE)
    return MAP_FAILED;

  /* Select which portions of the file we need (entire file) */
  Base = MapViewOfFile(hMapFile, dwAccess, 0, offset, size);

  if (Base == NULL) {
    /* Free the mapping object */
    CloseHandle(hMapFile);
    return MAP_FAILED;
  }

  /* Allocate item for list mmaps... */
  Item = (MapList_t *)malloc(sizeof(MapList_t));
  if (Item == NULL) {
    UnmapViewOfFile(Base);
    CloseHandle(hMapFile);

    return MAP_FAILED;
  }

  Item->hMap = hMapFile;
  Item->Base = Base;
  Item->Next = MapList;

  if (MapList == NULL)
    MapList = Item;

  return Base;
}

static int munmap(void *addr, unsigned int size)
{
  MapList_t *Item, *Prev;

  Prev = NULL;
  Item = MapList;

  while (Item != NULL) {
    if (Item->Base == addr) {
      UnmapViewOfFile(Item->Base);
      CloseHandle(Item->hMap);

      /* Delete this item from linked list */
      if (Prev != NULL)
        Prev->Next = Item->Next;
      else
        MapList = Item->Next;

      free(Item);

      return 0;
    }
    Prev = Item;
    Item = Item->Next;
  }

  return -1;
}
#endif /* _WIN32 */
#endif

bool jsextern_os_swap(int64_t* ptr, int64_t comparand, int64_t replacement)
{
#if 1
    *ptr = replacement;
    return true;
#elif defined(WIN32)
    return (_InterlockedCompareExchange64(reinterpret_cast<volatile int64_t*>(ptr), replacement, comparand) == comparand);
#else
    return __sync_bool_compare_and_swap(reinterpret_cast<volatile int64_t*>(ptr), comparand, replacement);
#endif
}

#if 1
#include "pcre.h"
void *jsextern_pcre_compile(const char *pattern)
{
    const unsigned char* pcre_tables;
    const char *pcre_error = NULL;
    int pcre_erroffset;
    void *compiled_pattern;

#if defined(JSLANG_UTF8)
    pcre_tables = pcre_maketables();
    compiled_pattern = (void *)pcre_compile((const char *)pattern, PCRE_UTF8 | PCRE_NO_UTF8_CHECK, &pcre_error, &pcre_erroffset, pcre_tables);
#else
    pcre_tables = pcre16_maketables();
    compiled_pattern = (void *)pcre16_compile((const unsigned short *)pattern, PCRE_UTF16 | PCRE_NO_UTF16_CHECK, &pcre_error, &pcre_erroffset, pcre_tables);
#endif

    return compiled_pattern;
}

void *jsextern_pcre_study(void *compiled_pattern)
{
    void *study_result;
    const char *pcre_error = NULL;

#if defined(JSLANG_UTF8)
    study_result = pcre_study((pcre *)compiled_pattern, 0, &pcre_error);
#else
    study_result = pcre16_study((pcre16 *)compiled_pattern, 0, &pcre_error);
#endif

    return study_result;
}

int jsextern_pcre_bracketcount(void *compiled_pattern)
{
    int capture_count = 0;
#if defined(JSLANG_UTF8)
    pcre_fullinfo((pcre*)compiled_pattern, NULL, PCRE_INFO_CAPTURECOUNT, &capture_count);
#else
    pcre_fullinfo((pcre16 *)compiled_pattern, NULL, PCRE_INFO_CAPTURECOUNT, &capture_count);
#endif
    return capture_count;
}

int jsextern_pcre_exec(void *compiled_pattern, void *study_result, const char *subject, int length, int start_offset, int *vector, int vector_length)
{
    int rv;
#if defined(JSLANG_UTF8)
    rv = pcre_exec((pcre*)compiled_pattern, (pcre_extra *)study_result, (const char *)subject, length, 0, PCRE_NO_UTF8_CHECK, vector, vector_length);
#else
    rv = pcre16_exec((pcre16*)compiled_pattern, (pcre16_extra *)study_result, (const unsigned short *)subject, length, 0, PCRE_NO_UTF16_CHECK, vector, vector_length);
#endif

    if (0 > rv)
    {
        return 0;
    }
    else if (0 == rv)
    {
        return vector_length / 3;
    }
    return  rv;
}

void jsextern_pcre_free(void *compiled_pattern)
{
#if defined(JSLANG_UTF8)
    pcre_free(compiled_pattern);
#else
    pcre16_free(compiled_pattern);
#endif
}
#endif

#if 1
#include "pthread.h"
typedef struct
{
    pthread_t tid;
}JSThread;

int jsextern_thread_create(void *(*start)(void *), void *arg)
{
    JSThread *thread = (JSThread *)malloc(sizeof(JSThread));
    int result = pthread_create(&thread->tid, NULL, start, arg);
    if (0 != result)
    {
        printf("%s:%d pthread_create failed !\n", __FUNCTION__, __LINE__);
        free((void *)thread);
        thread = NULL;
    }
    return (int)thread;
}

void  jsextern_thread_destroy(int handle)
{
    JSThread *thread = (JSThread *)handle;
    if (NULL == thread)
    {
        return ;
    }

    int result = pthread_cancel(thread->tid);
    if (0 !=  result)
    {
        printf("%s:%d pthread destroy failed !\n", __FUNCTION__, __LINE__);
        return ;
    }
    free((void *)thread);
}

typedef struct
{
    pthread_mutex_t mutex;
}JSMutex;
int jsextern_mutex_create()
{
    JSMutex *mutex = (JSMutex *)malloc(sizeof(JSMutex));
    pthread_mutexattr_t mattr;
    
    pthread_mutexattr_init (&mattr);
    pthread_mutexattr_settype (&mattr, PTHREAD_MUTEX_RECURSIVE);
    
    pthread_mutex_init(&mutex->mutex, &mattr);

    return (int)mutex;
}

void jsextern_mutex_destroy(int handle)
{
    JSMutex *mutex = (JSMutex *)handle;
    if (NULL == mutex)
    {
        return ;
    }

    int result = pthread_mutex_destroy(&mutex->mutex);
    if (0 !=  result)
    {
        printf("%s:%d mutex destroy failed !\n", __FUNCTION__, __LINE__);
        return ;
    }
    free((void *)mutex);
}

void jsextern_mutex_lock(int handle)
{
    JSMutex *mutex = (JSMutex *)handle;
    if (NULL == mutex)
    {
        printf("%s:%d mutex=NULL !\n", __FUNCTION__, __LINE__);
        return ;
    }

    pthread_mutex_lock(&mutex->mutex);
}

void jsextern_mutex_unlock(int handle)
{
    JSMutex *mutex = (JSMutex *)handle;
    if (NULL == mutex)
    {
        printf("%s:%d mutex=NULL !\n", __FUNCTION__, __LINE__);
        return ;
    }

    pthread_mutex_unlock(&mutex->mutex);
}

typedef struct
{
    pthread_mutex_t mutex;
    pthread_cond_t cond;
}JSSignal;
int jsextern_signal_create()
{
    JSSignal *signal = (JSSignal *)malloc(sizeof(JSSignal));
    pthread_mutexattr_t mattr;
    
    pthread_mutexattr_init (&mattr);
    pthread_mutexattr_settype (&mattr, PTHREAD_MUTEX_RECURSIVE);
    
    pthread_mutex_init(&signal->mutex, &mattr);

    return (int)signal;
}

void jsextern_signal_destroy(int handle)
{
    JSSignal *signal = (JSSignal *)handle;
    if (NULL == signal)
    {
        return ;
    }

    int result = pthread_mutex_destroy(&signal->mutex);
    if (0 !=  result)
    {
        printf("%s:%d signal destroy failed !\n", __FUNCTION__, __LINE__);
        return ;
    }
    free((void *)signal);
}

void jsextern_signal_wait(int handle)
{
    JSSignal *signal = (JSSignal *)handle;
    if (NULL == signal)
    {
        printf("%s:%d signal=NULL !\n", __FUNCTION__, __LINE__);
        return ;
    }

    pthread_mutex_lock(&signal->mutex);
    pthread_cond_wait(&signal->cond, &signal->mutex);
    pthread_mutex_lock(&signal->mutex);
}

void jsextern_signal_send(int handle)
{
    JSSignal *signal = (JSSignal *)handle;
    if (NULL == signal)
    {
        printf("%s:%d signal=NULL !\n", __FUNCTION__, __LINE__);
        return ;
    }

    pthread_mutex_lock(&signal->mutex);
    pthread_cond_signal(&signal->cond);
    pthread_mutex_unlock(&signal->mutex);
}

#endif

}
