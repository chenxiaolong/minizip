#include <fcntl.h>
#include <stdlib.h>
#include <string.h>

#include "zlib.h"
#include "ioapi.h"
#include "ioandroid.h"

// TODO: Remove once Android supports LFS with the fopen (stdio.h) API

#if __ANDROID_API__ >= 21
#define OPEN_FUNC open64
#else
#define OPEN_FUNC open
#endif

voidpf ZCALLBACK android_open64_file_func OF((voidpf opaque, const void *filename, int mode));
uLong ZCALLBACK android_read_file_func OF((voidpf opaque, voidpf stream, void *buf, uLong size));
uLong ZCALLBACK android_write_file_func OF((voidpf opaque, voidpf stream, const void *buf, uLong size));
ZPOS64_T ZCALLBACK android_tell64_file_func OF((voidpf opaque, voidpf stream));
long ZCALLBACK android_seek64_file_func OF((voidpf opaque, voidpf stream, ZPOS64_T offset, int origin));
int ZCALLBACK android_close_file_func OF((voidpf opaque, voidpf stream));
int ZCALLBACK android_error_file_func OF((voidpf opaque, voidpf stream));

typedef struct
{
    int fd;
    int filenameLength;
    void *filename;
} FILE_IOANDROID;

static voidpf file_build_ioandroid(int fd, const char *filename)
{
    FILE_IOANDROID *ioandroid = NULL;
    if (fd < 0) {
        return NULL;
    }

    ioandroid = (FILE_IOANDROID *) malloc(sizeof(FILE_IOANDROID));
    ioandroid->fd = fd;
    ioandroid->filenameLength = strlen(filename) + 1;
    ioandroid->filename = strdup(filename);

    return (voidpf) ioandroid;
}

voidpf ZCALLBACK android_open64_file_func(voidpf opaque, const void *filename,
                                          int mode)
{
    int fd = -1;
    int flags = 0;

    if ((mode & ZLIB_FILEFUNC_MODE_READWRITEFILTER) == ZLIB_FILEFUNC_MODE_READ) {
        flags |= O_LARGEFILE | O_RDONLY;
    } else if (mode & ZLIB_FILEFUNC_MODE_EXISTING) {
        flags |= O_LARGEFILE | O_RDWR;
    } else if (mode & ZLIB_FILEFUNC_MODE_CREATE) {
        flags |= O_LARGEFILE | O_WRONLY | O_CREAT | O_TRUNC;
    }

    if (filename && flags) {
        fd = OPEN_FUNC(filename, flags, 0666);
        return file_build_ioandroid(fd, (const char *) filename);
    }

    return NULL;
}

voidpf ZCALLBACK android_opendisk64_file_func(voidpf opaque, voidpf stream,
                                              int number_disk, int mode)
{
    FILE_IOANDROID *ioandroid = NULL;
    char *diskFilename = NULL;
    voidpf ret = NULL;
    int i = 0;

    if (!stream) {
        return NULL;
    }

    ioandroid = (FILE_IOANDROID *) stream;
    diskFilename = strdup(ioandroid->filename);

    for (i = ioandroid->filenameLength - 1; i >= 0; --i) {
        if (diskFilename[i] != '.') {
            continue;
        }
        snprintf(&diskFilename[i], ioandroid->filenameLength - i,
                 ".z%02d", number_disk + 1);
        break;
    }

    if (i >= 0) {
        ret = android_open64_file_func(opaque, diskFilename, mode);
    }

    free(diskFilename);
    return ret;
}

uLong ZCALLBACK android_read_file_func(voidpf opaque, voidpf stream,
                                       void *buf, uLong size)
{
    FILE_IOANDROID *ioandroid = NULL;
    uLong ret;

    if (!stream) {
        return -1;
    }

    ioandroid = (FILE_IOANDROID *) stream;
    ret = (uLong) read(ioandroid->fd, buf, (size_t) size);
    return ret;
}

uLong ZCALLBACK android_write_file_func(voidpf opaque, voidpf stream,
                                        const void *buf, uLong size)
{
    FILE_IOANDROID *ioandroid = NULL;
    uLong ret;

    if (!stream) {
        return -1;
    }

    ioandroid = (FILE_IOANDROID *) stream;
    ret = (uLong) write(ioandroid->fd, buf, (size_t) size);
    return ret;
}

ZPOS64_T ZCALLBACK android_tell64_file_func(voidpf opaque, voidpf stream)
{
    FILE_IOANDROID *ioandroid = NULL;
    ZPOS64_T ret;

    if (!stream) {
        return -1;
    }

    ioandroid = (FILE_IOANDROID *) stream;
    ret = lseek64(ioandroid->fd, 0, SEEK_CUR);
    return ret;
}

long ZCALLBACK android_seek64_file_func(voidpf opaque, voidpf stream,
                                        ZPOS64_T offset, int origin)
{
    FILE_IOANDROID *ioandroid = NULL;
    int whence;
    long ret = 0;

    if (!stream) {
        return -1;
    }

    ioandroid = (FILE_IOANDROID *) stream;

    switch (origin) {
    case ZLIB_FILEFUNC_SEEK_CUR:
        whence = SEEK_CUR;
        break;
    case ZLIB_FILEFUNC_SEEK_END:
        whence = SEEK_END;
        break;
    case ZLIB_FILEFUNC_SEEK_SET:
        whence = SEEK_SET;
        break;
    default:
        return -1;
    }

    if (lseek64(ioandroid->fd, offset, whence) < 0) {
        ret = -1;
    }

    return ret;
}

int ZCALLBACK android_close_file_func(voidpf opaque, voidpf stream)
{
    FILE_IOANDROID *ioandroid = NULL;
    int ret;

    if (!stream) {
        return -1;
    }

    ioandroid = (FILE_IOANDROID *) stream;

    if (ioandroid->filename) {
        free(ioandroid->filename);
    }

    ret = close(ioandroid->fd);
    free(ioandroid);
    return ret;
}

int ZCALLBACK android_error_file_func(voidpf opaque, voidpf stream)
{
    // Never return errors
    return 0;
}

void fill_android_filefunc64(zlib_filefunc64_def *pzlib_filefunc_def)
{
    pzlib_filefunc_def->zopen64_file = android_open64_file_func;
    pzlib_filefunc_def->zopendisk64_file = android_opendisk64_file_func;
    pzlib_filefunc_def->zread_file = android_read_file_func;
    pzlib_filefunc_def->zwrite_file = android_write_file_func;
    pzlib_filefunc_def->ztell64_file = android_tell64_file_func;
    pzlib_filefunc_def->zseek64_file = android_seek64_file_func;
    pzlib_filefunc_def->zclose_file = android_close_file_func;
    pzlib_filefunc_def->zerror_file = android_error_file_func;
    pzlib_filefunc_def->opaque = NULL;
}
