/*
 * SPDX-License-Identifier: BSD-2-Clause
 * 
 * trace.h - Tracing
 * 
 * This header provides tracing functions for Amiga
 * 
 * Copyright (C) 2025 by amigazen project
 */

#ifndef _TRACE_H
#define _TRACE_H 1

#include <sys/types.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Trace event types */
#define TRACE_EVENT_BEGIN 0x01
#define TRACE_EVENT_END   0x02
#define TRACE_EVENT_FUNC  0x04
#define TRACE_EVENT_CALL  0x08
#define TRACE_EVENT_RET   0x10
#define TRACE_EVENT_MEM   0x20
#define TRACE_EVENT_IO    0x40
#define TRACE_EVENT_SYSCALL 0x80

/* Trace levels */
#define TRACE_LEVEL_NONE    0
#define TRACE_LEVEL_ERROR   1
#define TRACE_LEVEL_WARNING 2
#define TRACE_LEVEL_INFO    3
#define TRACE_LEVEL_DEBUG   4
#define TRACE_LEVEL_VERBOSE 5

/* Trace flags */
#define TRACE_FLAG_ENABLED  0x01
#define TRACE_FLAG_TIMESTAMP 0x02
#define TRACE_FLAG_PID      0x04
#define TRACE_FLAG_TID      0x08
#define TRACE_FLAG_FUNC     0x10
#define TRACE_FLAG_FILE     0x20
#define TRACE_FLAG_LINE     0x40

/* Trace buffer structure */
struct trace_buffer {
    void *data;
    size_t size;
    size_t used;
    size_t head;
    size_t tail;
};

/* Trace event structure */
struct trace_event {
    uint32_t type;
    uint32_t level;
    uint32_t flags;
    uint64_t timestamp;
    pid_t pid;
    pid_t tid;
    const char *func;
    const char *file;
    uint32_t line;
    const char *message;
    void *data;
    size_t data_size;
};

/* Function prototypes */
int trace_init(void);
void trace_cleanup(void);
int trace_set_level(int level);
int trace_set_flags(uint32_t flags);
int trace_enable(void);
int trace_disable(void);
int trace_event(int type, int level, const char *func, const char *file, 
                int line, const char *message, ...);
int trace_event_data(int type, int level, const char *func, const char *file,
                     int line, const void *data, size_t data_size,
                     const char *message, ...);
int trace_printf(const char *format, ...);
int trace_dump(const char *filename);
int trace_clear(void);
int trace_get_count(void);
int trace_get_buffer_size(void);
int trace_set_buffer_size(size_t size);

/* Convenience macros */
#define TRACE_ERROR(msg, ...) \
    trace_event(TRACE_EVENT_FUNC, TRACE_LEVEL_ERROR, __func__, __FILE__, __LINE__, msg, ##__VA_ARGS__)

#define TRACE_WARNING(msg, ...) \
    trace_event(TRACE_EVENT_FUNC, TRACE_LEVEL_WARNING, __func__, __FILE__, __LINE__, msg, ##__VA_ARGS__)

#define TRACE_INFO(msg, ...) \
    trace_event(TRACE_EVENT_FUNC, TRACE_LEVEL_INFO, __func__, __FILE__, __LINE__, msg, ##__VA_ARGS__)

#define TRACE_DEBUG(msg, ...) \
    trace_event(TRACE_EVENT_FUNC, TRACE_LEVEL_DEBUG, __func__, __FILE__, __LINE__, msg, ##__VA_ARGS__)

#define TRACE_VERBOSE(msg, ...) \
    trace_event(TRACE_EVENT_FUNC, TRACE_LEVEL_VERBOSE, __func__, __FILE__, __LINE__, msg, ##__VA_ARGS__)

#define TRACE_FUNC_ENTER() \
    trace_event(TRACE_EVENT_BEGIN, TRACE_LEVEL_DEBUG, __func__, __FILE__, __LINE__, "Enter")

#define TRACE_FUNC_EXIT() \
    trace_event(TRACE_EVENT_END, TRACE_LEVEL_DEBUG, __func__, __FILE__, __LINE__, "Exit")

#ifdef __cplusplus
}
#endif

#endif /* _TRACE_H */
