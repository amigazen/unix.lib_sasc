#include "amiga.h"
#include <sys/stat.h> // Required for the mode_t type

int chmod(const char *path, mode_t mode)
{
  chkabort(); // Check for Ctrl-C abort signal

  // Call the AmigaDOS function with the converted protection bits.
  // SetProtection returns 0 on failure, non-zero on success.
  if (SetProtection((char *)path, _make_protection(mode)))
  {
    return 0; // Success
  }

  // SetProtection failed, so set errno and return -1.
  ERROR; // This macro likely sets errno from IoErr() and returns -1
}