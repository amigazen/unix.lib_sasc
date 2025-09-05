/*
 * POSIX Message Queue Integration for Amiga
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 amigazen project
 */

#include "amiga.h"
#include "include/internal/mq_internal.h"

/* Initialize message queue system during library startup */
void _init_message_queues(void) {
    _mq_init();
}

/* Cleanup message queue system during library shutdown */
void _cleanup_message_queues(void) {
    _mq_cleanup();
}
