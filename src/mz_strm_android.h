#ifndef _MZ_STREAM_ANDROID_H
#define _MZ_STREAM_ANDROID_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

int32_t mz_stream_android_open(void *stream, const char *path, int32_t mode);
int32_t mz_stream_android_is_open(void *stream);
int32_t mz_stream_android_read(void *stream, void *buf, int32_t size);
int32_t mz_stream_android_write(void *stream, const void *buf, int32_t size);
int64_t mz_stream_android_tell(void *stream);
int32_t mz_stream_android_seek(void *stream, int64_t offset, int32_t origin);
int32_t mz_stream_android_close(void *stream);
int32_t mz_stream_android_error(void *stream);

void *  mz_stream_android_create(void **stream);
void    mz_stream_android_delete(void **stream);

void *  mz_stream_android_get_interface(void);

#ifdef __cplusplus
}
#endif

#endif
