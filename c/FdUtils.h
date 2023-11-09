#pragma once

#include <stdbool.h>   /* bool */
#include <sys/types.h> /* size_t, ssize_t */

/**
 *  \brief Read from fd into d_first, up to count byte(s) or when a byte matches
 *         delim
 *
 *  \param[in] fd File descriptor we wish to read from
 *  \param[out] d_first Begin of the destination byte a range
 *  \param[in] count Maximum number of bytes to read
 *  \param[in] delim Character used to stop reading
 *
 *  \return Number of bytes read, -1 on failure with errno from linux read().
 */
ssize_t atb_Fd_ReadBytesUntil(int fd, char *d_first, size_t count, char delim);
