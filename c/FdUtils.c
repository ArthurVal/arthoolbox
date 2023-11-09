#include "FdUtils.h"

#include <assert.h> /* assert */
#include <sys/types.h>
#include <unistd.h> /* read */

ssize_t atb_Fd_ReadBytesUntil(int fd, char *d_first, size_t count, char delim) {
  assert(fd >= 0);
  assert(d_first != NULL);

  ssize_t total_count = 0;
  ssize_t last_read_count = 0;

  while ((total_count < (ssize_t)count) &&
         ((last_read_count = read(fd, d_first, 1)) == 1)) {

    total_count += last_read_count;

    if (*d_first == delim)
      break;
    else
      d_first += last_read_count;
  }

  return last_read_count == -1 ? last_read_count : total_count;
}
