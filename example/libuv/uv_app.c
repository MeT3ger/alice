#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <uv.h>

/*
 * Minimal crash consistency example using libuv.
 * Updates "data_uv.db" via a temporary file and atomic rename.
 *
 * - "flawed" mode (default): writes and renames without fsync.
 * - "safe" mode: flushes data via uv_fs_fdatasync and directory via uv_fs_fsync.
 */

int main(int argc, char **argv) {
    int safe = (argc > 1 && strcmp(argv[1], "safe") == 0);
    uv_loop_t *loop = uv_default_loop();
    uv_fs_t req;

    const char *tmp = "data_uv.tmp";
    const char *dst = "data_uv.db";
    const char *payload = "libuv: version 2\npayload: 42\n";
    size_t payload_len = strlen(payload);

    // 1. Create and write to temporary file
    int fd = uv_fs_open(loop, &req, tmp, O_WRONLY | O_CREAT | O_TRUNC, 0644, NULL);
    uv_fs_req_cleanup(&req);
    if (fd < 0) {
        fprintf(stderr, "Failed to open tmp file: %s\n", uv_strerror(fd));
        return 1;
    }

    uv_buf_t buf = uv_buf_init((char *)payload, payload_len);
    int written = uv_fs_write(loop, &req, fd, &buf, 1, 0, NULL);
    uv_fs_req_cleanup(&req);
    if (written < 0) {
        fprintf(stderr, "Failed to write: %s\n", uv_strerror(written));
        return 1;
    }

    // 2. In safe mode: ensure dirty data is flushed to persistent storage
    if (safe) {
        uv_fs_fdatasync(loop, &req, fd, NULL);
        uv_fs_req_cleanup(&req);
    }

    uv_fs_close(loop, &req, fd, NULL);
    uv_fs_req_cleanup(&req);

    // 3. Atomically replace target file
    uv_fs_rename(loop, &req, tmp, dst, NULL);
    uv_fs_req_cleanup(&req);

    // 4. In safe mode: ensure directory entry is flushed
    if (safe) {
        int dir_fd = uv_fs_open(loop, &req, ".", O_RDONLY, 0, NULL);
        uv_fs_req_cleanup(&req);
        if (dir_fd >= 0) {
            uv_fs_fsync(loop, &req, dir_fd, NULL);
            uv_fs_req_cleanup(&req);
            uv_fs_close(loop, &req, dir_fd, NULL);
            uv_fs_req_cleanup(&req);
        }
    }

    // 5. Notify caller of transaction commit
    printf("LIBUV_COMMIT_OK\n");
    fflush(stdout);

    uv_run(loop, UV_RUN_DEFAULT);
    uv_loop_close(loop);
    return 0;
}
