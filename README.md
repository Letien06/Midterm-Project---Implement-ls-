# Midterm Project - Simplified `ls(1)`

**Student:** Le Cao Son Tien

**Student ID:** 24IT270

## Hướng dẫn dành cho giảng viên

Đây là bài C11/POSIX mô phỏng lệnh `ls`. Thầy có thể tải source code, build và
kiểm tra theo từng bước trong [HUONG_DAN_CHAY.md](HUONG_DAN_CHAY.md). Tài liệu
có bước cài công cụ, tải code từ GitHub, lệnh chạy đủ 19 option, kết quả cần
quan sát và cách xử lý lỗi thường gặp.

**Môi trường:** Ubuntu/Debian, có GCC hoặc Clang, GNU Make và Python 3. Nếu dùng
Windows, chạy các lệnh dưới đây trong terminal Ubuntu của WSL; chương trình
dùng API POSIX nên không build trực tiếp bằng PowerShell hoặc CMD.

Nếu máy chưa có công cụ, mở terminal Ubuntu/Debian và chạy:

```sh
sudo apt update
sudo apt install -y git build-essential python3
```

Sau khi tải code, mở terminal tại thư mục chứa `Makefile`, `src/` và `include/`.
Chạy lần lượt các lệnh sau bằng user thường:

```sh
make clean
make
./ls
./ls -laF
./ls -lR src
make test
echo $?
```

Kết quả mong đợi:

- `make` hoàn tất, tạo executable `ls` ở thư mục gốc.
- Gọi chương trình bằng `./ls`; lệnh `ls` sẽ gọi chương trình có sẵn của hệ điều hành.
- `make test` kết thúc bằng `Ran 16 tests in ...` và `OK`.
- `echo $?` in `0` khi chạy ngay sau `make test` thành công.

Có thể build sạch và chạy toàn bộ test bằng một lệnh:

```sh
make verify
```

Để xem demo có dữ liệu mẫu cho đủ 19 option:

```sh
make demo
```

Repository: [Midterm Project - Implement ls](https://github.com/Letien06/Midterm-Project---Implement-ls-).
Lệnh clone chính xác nằm ở bước 2 của [HUONG_DAN_CHAY.md](HUONG_DAN_CHAY.md).

## Project overview

This project is a C11/POSIX implementation of the `ls(1)` subset defined by
the supplied NetBSD manual. It deliberately implements one entry per line,
rather than adding unrelated GNU `ls` features such as colours or columns.

## Build and run

Build on Linux, NetBSD, FreeBSD, macOS, or a normal WSL Linux distribution:

```sh
make
./ls
./ls -laF
./ls -lR src
```

The executable is `./ls`; it is never installed over the system `ls` command.
`make demo` demonstrates `--` before an operand beginning with `-` in a
temporary directory, without modifying the repository.

Development targets:

```sh
make help
make verify
make demo
make debug
make sanitize
make test
make clean
```

## Implemented manual options

| Group | Options | Behavior |
|---|---|---|
| Visibility | `-A`, `-a` | Includes hidden entries as specified; `-A` is automatically active for super-user. |
| File selection | `-d`, `-R` | Lists directories as files or recursively traverses subdirectories. |
| Layout | `-i`, `-s`, `-l`, `-n` | Inode, allocated blocks, long output, and numeric owner/group IDs. |
| Size display | `-h`, `-k` | Human-readable sizes or KiB block counts. |
| Name display | `-F`, `-q`, `-w` | Type suffixes and terminal-aware non-printable character behavior. |
| Ordering | `-f`, `-r`, `-S`, `-t` | Unsorted, reverse, size, and time ordering. |
| Time source | `-c`, `-u` | Status-change or access time for `-t` and long format. |

## Design

| File | Responsibility |
|---|---|
| `src/main.c` | Initializes locale/timezone, invokes listing, returns status. |
| `src/options.c` | Parses all 19 short options and their precedence rules. |
| `src/entry.c` | Reads `lstat`/`stat` metadata and owns dynamic entries. |
| `src/listing.c` | Splits operands, reads directories, filters entries, and recurses safely. |
| `src/sort.c` | Stable name/size/time ordering with reverse support. |
| `src/format.c` | Short/long display, permissions, blocks, names, link targets, and suffixes. |
| `src/units.c` | `BLOCKSIZE`, block rounding, overflow checks, and human-readable values. |
| `src/util.c` | Dynamic-array growth, path creation, error and output handling. |

## Important option interactions

The parser evaluates short options left to right. The rightmost option wins for
the documented pairs `-l/-n`, `-q/-w`, `-c/-u`, `-R/-d`, and `-h/-k`.

For the manual's otherwise unspecified `-S` with `-t` combination, the
rightmost sorting key is used. `-f` disables sorting regardless of its location
in the option list, so `-r` has no visible effect with `-f`.

## Format details

- Long-format permissions include set-user-ID, set-group-ID, and sticky bits as
  `s`, `S`, `t`, and `T`.
- Long dates are always `%b %e %H:%M`, matching the fields stated by the
  supplied manual. `TZ` is honored through the system timezone functions.
- `-s` uses `st_blocks` (512-byte allocated blocks) rather than logical file
  size, then rounds up in the requested unit.
- `BLOCKSIZE` accepts a positive integer optionally followed by `K`, `KiB`,
  `M`, `MiB`, `G`, `T`, `P`, or `E`; invalid values fall back to 512 bytes.
- `-h` uses binary units (`B`, `K`, `M`, `G`, `T`, `P`, `E`).
- A command-line symbolic link to a directory is followed unless `-d` is set.
  Links found inside a directory are never followed during recursion.
- Platform-specific whiteout/archive types are emitted when their platform
  macros are available; Linux normally cannot create NetBSD archive/whiteout
  entries.

## Robustness

The implementation checks allocation and filesystem errors, preserves already
collected entries when a later entry fails, avoids recursing into `.` and `..`,
tracks ancestor device/inode pairs to stop directory loops, releases all owned
memory, and returns a non-zero status if any error occurs.

## Tests and submission

`make test` executes fixture-based regression tests. The tests do not compare
the whole output to the host `ls`, because GNU and NetBSD have known formatting
differences.

Submit the source code from the single project folder
`LeCaoSonTien_24IT270_midterm/` to GitHub. Before uploading, run:

```sh
make verify
make clean
```

The repository root contains the Makefile, documentation, `src/`, `include/`,
and `tests/`.

## Additional documentation

- [`HUONG_DAN_CHAY.md`](HUONG_DAN_CHAY.md): exact Linux build, test, demo, and
  cleanup commands for the instructor.
