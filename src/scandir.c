/*
 * scandir.c - scan directory entries (POSIX compliant)
 *
 * This function scans the directory dir, calling filter() on each
 * directory entry. Entries for which filter() returns nonzero are
 * stored in strings allocated via malloc(), sorted using qsort()
 * with comparison function compar, and collected in array namelist.
 *
 * POSIX.1-2001, POSIX.1-2008, 4.3BSD
 */

#include "amiga.h"
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

int scandir(const char *dir, struct dirent ***namelist,
            int (*filter)(const struct dirent *),
            int (*compar)(const struct dirent **, const struct dirent **))
{
    DIR *dirp;
    struct dirent *entry;
    struct dirent **list = NULL;
    int count = 0, i = 0;
    int allocated = 0;
    
    chkabort();
    
    if (dir == NULL || namelist == NULL) {
        errno = EFAULT;
        return -1;
    }
    
    /* Open directory */
    dirp = opendir(dir);
    if (dirp == NULL) {
        return -1;
    }
    
    /* Read directory entries */
    while ((entry = readdir(dirp)) != NULL) {
        /* Apply filter if provided */
        if (filter == NULL || filter(entry)) {
            /* Allocate space for new entry */
            if (count >= allocated) {
                allocated = allocated == 0 ? 16 : allocated * 2;
                list = realloc(list, allocated * sizeof(struct dirent *));
                if (list == NULL) {
                    /* Cleanup on error */
                    for (i = 0; i < count; i++) {
                        free(list[i]);
                    }
                    free(list);
                    closedir(dirp);
                    errno = ENOMEM;
                    return -1;
                }
            }
            
            /* Copy directory entry */
            list[count] = malloc(sizeof(struct dirent));
            if (list[count] == NULL) {
                /* Cleanup on error */
                for (i = 0; i < count; i++) {
                    free(list[i]);
                }
                free(list);
                closedir(dirp);
                errno = ENOMEM;
                return -1;
            }
            
            memcpy(list[count], entry, sizeof(struct dirent));
            count++;
        }
    }
    
    closedir(dirp);
    
    /* Sort entries if comparison function provided */
    if (compar != NULL && count > 1) {
        qsort(list, count, sizeof(struct dirent *), 
              (int (*)(const void *, const void *))compar);
    }
    
    *namelist = list;
    return count;
}

int alphasort(const struct dirent **a, const struct dirent **b)
{
    if (a == NULL || b == NULL) {
        return 0;
    }
    
    return strcmp((*a)->d_name, (*b)->d_name);
}
