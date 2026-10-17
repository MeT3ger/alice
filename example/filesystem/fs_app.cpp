#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>
#include <fcntl.h>
#include <unistd.h>

#ifdef __APPLE__
#define fdatasync(fd) fsync(fd)
#endif

namespace fs = std::filesystem;

/*
 * Minimal crash consistency example using C++17 std::filesystem.
 * Updates "data_fs.db" via a temporary file and fs::rename.
 *
 * - "flawed" mode (default): writes via std::ofstream and renames via fs::rename.
 *   Dirty OS page cache blocks remain uncommitted, risking 0-byte or lost data upon crash.
 * - "safe" mode: flushes dirty pages with fdatasync and directory entry with fsync.
 */

int main(int argc, char **argv) {
    bool safe = (argc > 1 && std::string(argv[1]) == "safe");

    fs::path tmp_path = "data_fs.tmp";
    fs::path target_path = "data_fs.db";

    // 1. Write payload to temporary file using std::ofstream
    {
        std::ofstream ofs(tmp_path);
        if (!ofs.is_open()) {
            std::cerr << "Failed to open " << tmp_path << std::endl;
            return 1;
        }
        ofs << "std::filesystem: version 2\npayload: 42\n";
        ofs.flush();
    }

    // 2. In safe mode: flush dirty pages of the temporary file to disk
    if (safe) {
        int fd = open(tmp_path.c_str(), O_WRONLY);
        if (fd >= 0) {
            fdatasync(fd);
            close(fd);
        }
    }

    // 3. Atomically replace target file using C++17 std::filesystem::rename
    fs::rename(tmp_path, target_path);

    // 4. In safe mode: flush parent directory to persist the directory entry update
    if (safe) {
        int dir_fd = open(".", O_RDONLY);
        if (dir_fd >= 0) {
            fsync(dir_fd);
            close(dir_fd);
        }
    }

    // 5. Notify caller of transaction commit
    std::cout << "STD_FS_COMMIT_OK" << std::endl;
    return 0;
}
