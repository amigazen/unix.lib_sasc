/*
 * amiga_termcap_db_embedded.c - Embedded termcap database
 *
 * Copyright (c) 2025 amigazen project
 *
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * This file contains the termcap database embedded as a simple string.
 * All Amigas are the same - no need for external files!
 */

#include "amiga_termcap_private.h"
#include "termcap.h"

/*
 * Simple embedded termcap database
 * Just the essential capabilities for AmigaOS console
 */
static const char embedded_termcap_data[] = 
    "amiga-console|amiga|amigaos-console:"
    "co#80:li#25:am:bs:cr=^M:do=^J:le=^H:nd=^L:up=^K:"
    "cl=\\033[2J:cd=\\033[J:ce=\\033[K:ho=\\033[H:"
    "cm=\\033[%d;%dH:"
    "vi=\\033[0p:ve=\\033[0p:"
    "md=\\033[1m:me=\\033[0m:mr=\\033[7m:so=\\033[7m:se=\\033[0m:"
    "us=\\033[4m:ue=\\033[0m:Co#16:"
    "AF=\\033[3%dm:AB=\\033[4%dm:"
    "k1=\\033[11~:k2=\\033[12~:k3=\\033[13~:k4=\\033[14~:"
    "k5=\\033[15~:k6=\\033[17~:k7=\\033[18~:k8=\\033[19~:"
    "k9=\\033[20~:k0=\\033[21~:k;=\\033[23~:k,=\\033[24~:"
    "kh=\\033[H:kl=\\033[D:kr=\\033[C:ku=\\033[A:kd=\\033[B:"
    "kP=\\033[5~:kN=\\033[6~:kI=\\033[2~:kD=\\033[3~:"
    "kb=^H:pc=\\0:bl=^G:"
    "cuu=\\033[%dA:cud=\\033[%dB:cub=\\033[%dD:cuf=\\033[%dC:"
    "cnl=\\033[%dE:cpl=\\033[%dF:"
    "dl=\\033[%dM:al=\\033[%dL:"
    "ed=\\033[0J:el=\\033[K:"
    "sgr=\\033[%dm:"
    "setaf=\\033[3%dm:setab=\\033[4%dm:"
    "dch=\\033[%dP:"
    "sc=\\033[s:rc=\\033[u:"
    "tbc=\\033[3g:"
    "smcup=\\033[?1049h:rmcup=\\033[?1049l:"
    "smkx=\\033[?1h:rmkx=\\033[?1l:";

/*
 * amiga_termcap_load_embedded_db - load embedded termcap database
 * 
 * This function loads the embedded termcap database into memory.
 * Simple and reliable - no file I/O needed!
 */
int
amiga_termcap_load_embedded_db(void)
{
    struct termcap_entry *entry;
    
    /* Allocate database structure */
    g_termcap_db = amiga_termcap_malloc(sizeof(struct termcap_db));
    if (!g_termcap_db) {
        return AMIGA_TERMCAP_ERROR_MEMORY;
    }
    
    g_termcap_db->entries = NULL;
    g_termcap_db->entry_count = 0;
    g_termcap_db->db_path = amiga_termcap_strdup("embedded");
    
    /* Create single entry for amiga-console */
    entry = amiga_termcap_malloc(sizeof(struct termcap_entry));
    if (!entry) {
        amiga_termcap_free(g_termcap_db);
        return AMIGA_TERMCAP_ERROR_MEMORY;
    }
    
    entry->name = amiga_termcap_strdup("amiga-console");
    entry->aliases = amiga_termcap_strdup("amiga|amigaos-console");
    entry->capabilities = amiga_termcap_strdup("co#80:li#25:am:bs:cr=^M:do=^J:le=^H:nd=^L:up=^K:cl=\\033[2J:cd=\\033[J:ce=\\033[K:ho=\\033[H:cm=\\033[%d;%dH:vi=\\033[0p:ve=\\033[0p:md=\\033[1m:me=\\033[0m:mr=\\033[7m:so=\\033[7m:se=\\033[0m:us=\\033[4m:ue=\\033[0m:Co#16:AF=\\033[3%dm:AB=\\033[4%dm:k1=\\033[11~:k2=\\033[12~:k3=\\033[13~:k4=\\033[14~:k5=\\033[15~:k6=\\033[17~:k7=\\033[18~:k8=\\033[19~:k9=\\033[20~:k0=\\033[21~:k;=\\033[23~:k,=\\033[24~:kh=\\033[H:kl=\\033[D:kr=\\033[C:ku=\\033[A:kd=\\033[B:kP=\\033[5~:kN=\\033[6~:kI=\\033[2~:kD=\\033[3~:kb=^H:pc=\\0:bl=^G:cuu=\\033[%dA:cud=\\033[%dB:cub=\\033[%dD:cuf=\\033[%dC:cnl=\\033[%dE:cpl=\\033[%dF:dl=\\033[%dM:al=\\033[%dL:ed=\\033[0J:el=\\033[K:sgr=\\033[%dm:setaf=\\033[3%dm:setab=\\033[4%dm:dch=\\033[%dP:sc=\\033[s:rc=\\033[u:tbc=\\033[3g:smcup=\\033[?1049h:rmcup=\\033[?1049l:smkx=\\033[?1h:rmkx=\\033[?1l:");
    entry->next = NULL;
    
    if (!entry->name || !entry->aliases || !entry->capabilities) {
        if (entry->name) amiga_termcap_free(entry->name);
        if (entry->aliases) amiga_termcap_free(entry->aliases);
        if (entry->capabilities) amiga_termcap_free(entry->capabilities);
        amiga_termcap_free(entry);
        amiga_termcap_free(g_termcap_db);
        return AMIGA_TERMCAP_ERROR_MEMORY;
    }
    
    g_termcap_db->entries = entry;
    g_termcap_db->entry_count = 1;
    
    return 0;
}

