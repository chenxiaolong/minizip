#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "mz.h"
#include "mz_strm.h"
#include "mz_strm_android.h"

// TODO: Remove once Android supports LFS with the fopen (stdio.h) API

#if __ANDROID_API__ >= 21
#  define OPEN_FUNC open64
#else
#  define OPEN_FUNC open
#endif

mz_stream_vtbl mz_stream_android_vtbl = {
    mz_stream_android_open,
    mz_stream_android_is_open,
    mz_stream_android_read,
    mz_stream_android_write,
    mz_stream_android_tell,
    mz_stream_android_seek,
    mz_stream_android_close,
    mz_stream_android_error,
    mz_stream_android_create,
    mz_stream_android_delete,
};

typedef struct mz_stream_android_s
{
    mz_stream   stream;
    int         fd;
    int         should_close;
    int32_t     error;
} mz_stream_android;

static int convert_to_int(const char *str, int *out)
{
    char *end;
    errno = 0;
    long num = strtol(str, &end, 10);
    if (errno == ERANGE || num < INT_MIN || num > INT_MAX
            || *str == '\0' || *end != '\0') {
        return -1;
    }
    *out = (int) num;
    return 0;
}

static int fd_is_valid(int fd)
{
    return fcntl(fd, F_GETFD) != -1 || errno != EBADF;
}

int32_t mz_stream_android_open(void *stream, const char *path, int32_t mode)
{
    mz_stream_android *android = (mz_stream_android *) stream;
    int flags = 0;

    if (!path) {
        return MZ_STREAM_ERROR;
    }

    if ((mode & MZ_OPEN_MODE_READWRITE) == MZ_OPEN_MODE_READ) {
        flags |= O_LARGEFILE | O_RDONLY;
    } else if (mode & MZ_OPEN_MODE_APPEND) {
        flags |= O_LARGEFILE | O_WRONLY | O_APPEND;
    } else if (mode & MZ_OPEN_MODE_CREATE) {
        flags |= O_LARGEFILE | O_WRONLY | O_CREAT | O_TRUNC;
    } else {
        return MZ_STREAM_ERROR;
    }

    // Hack for /proc/self/fd/<FD> filenames, since minizip has no API for
    // opening file descriptors
    static const char *prefix = "/proc/self/fd/";
    const char *ptr = (const char *) path;
    if (strncmp(ptr, prefix, strlen(prefix)) == 0) {
        int fd;

        ptr += strlen(prefix);
        if (convert_to_int(ptr, &fd) < 0) {
            return MZ_STREAM_ERROR;
        }
        if (!fd_is_valid(fd)) {
            return MZ_STREAM_ERROR;
        }

        if (lseek64(fd, 0, SEEK_SET) < 0) {
            return MZ_STREAM_ERROR;
        }

        android->fd = fd;
        android->should_close = 0;
    } else {
        android->fd = OPEN_FUNC(path, flags, 0666);
        if (android->fd < 0) {
            android->error = errno;
            return MZ_STREAM_ERROR;
        }

        android->should_close = 1;
    }

    return MZ_OK;
}

int32_t mz_stream_android_is_open(void *stream)
{
    mz_stream_android *android = (mz_stream_android *) stream;
    if (android->fd < 0) {
        return MZ_STREAM_ERROR;
    }
    return MZ_OK;
}

int32_t mz_stream_android_read(void *stream, void *buf, int32_t size)
{
    mz_stream_android *android = (mz_stream_android *) stream;
    ssize_t n = read(android->fd, buf, (size_t) size);
    if (n < 0) {
        android->error = errno;
        return MZ_STREAM_ERROR;
    }
    return n;
}

int32_t mz_stream_android_write(void *stream, const void *buf, int32_t size)
{
    mz_stream_android *android = (mz_stream_android *) stream;
    ssize_t n = write(android->fd, buf, (size_t) size);
    if (n < 0) {
        android->error = errno;
        return MZ_STREAM_ERROR;
    }
    return n;
}

int64_t mz_stream_android_tell(void *stream)
{
    mz_stream_android *android = (mz_stream_android *) stream;
    off64_t ret = lseek64(android->fd, 0, SEEK_CUR);
    if (ret < 0) {
        android->error = errno;
        return MZ_STREAM_ERROR;
    }
    return ret;
}

int32_t mz_stream_android_seek(void *stream, int64_t offset, int32_t origin)
{
    mz_stream_android *android = (mz_stream_android *) stream;
    int whence;

    switch (origin) {
    case MZ_SEEK_CUR:
        whence = SEEK_CUR;
        break;
    case MZ_SEEK_END:
        whence = SEEK_END;
        break;
    case MZ_SEEK_SET:
        whence = SEEK_SET;
        break;
    default:
        return MZ_STREAM_ERROR;
    }

    if (lseek64(android->fd, offset, whence) < 0) {
        android->error = errno;
        return MZ_STREAM_ERROR;
    }

    return MZ_OK;
}

int32_t mz_stream_android_close(void *stream)
{
    mz_stream_android *android = (mz_stream_android *) stream;
    int fd = android->fd;
    android->fd = -1;

    if (android->should_close && close(fd) < 0) {
        android->error = errno;
        return MZ_STREAM_ERROR;
    }

    return MZ_OK;
}

int32_t mz_stream_android_error(void *stream)
{
    mz_stream_android *android = (mz_stream_android *) stream;
    return android->error;
}

void * mz_stream_android_create(void **stream)
{
    mz_stream_android *android = NULL;

    android = (mz_stream_android *) calloc(sizeof(mz_stream_android), 1);
    if (android) {
        android->stream.vtbl = &mz_stream_android_vtbl;
        android->fd = -1;
    }
    if (stream) {
        *stream = android;
    }

    return android;
}

void mz_stream_android_delete(void **stream)
{
    mz_stream_android *android = NULL;
    if (!stream) {
        return;
    }
    android = (mz_stream_android *) *stream;
    if (android) {
        free(android);
    }
    *stream = NULL;
}

void * mz_stream_android_get_interface(void)
{
    return &mz_stream_android_vtbl;
}
