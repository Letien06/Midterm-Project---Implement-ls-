# Midterm Project - Simplified `ls(1)`

**Sinh viên:** Lê Cao Sơn Tiến  
**MSSV:** 24IT270

**Repository:** [Midterm Project - Implement ls](https://github.com/Letien06/Midterm-Project---Implement-ls-)

## Hướng dẫn đầy đủ dành cho giảng viên

Bài C11/POSIX mô phỏng 19 tùy chọn của `ls(1)` theo manual NetBSD được cung
cấp. Các bước tải code, build, sử dụng, đáp án mẫu và kiểm thử nằm ngay dưới
đây. Bản hướng dẫn riêng cũng có tại [HUONG_DAN_CHAY.md](HUONG_DAN_CHAY.md).
Chạy project bằng user thường; super-user tự động bật `-A` theo manual.


Tài liệu này dành cho giảng viên hoặc người chấm bài. Sau khi cài công cụ và
tải code ở bước 1–2, các lệnh từ bước 3 trở đi được chạy từ **thư mục gốc của
repository**, tức thư mục có `Makefile`, `README.md`, `src/` và `include/`.

### 1. Yêu cầu môi trường

- Linux hoặc môi trường POSIX tương đương.
- Trình biên dịch C11: GCC hoặc Clang.
- GNU Make và Git để clone repository.
- Python 3 để chạy bộ test tự động.

Môi trường chính để chấm: Ubuntu/Debian. Dùng một terminal xuyên suốt tài liệu.
Các lệnh cài package chỉ cần chạy khi máy chưa có các công cụ trên; không cần
Docker để dùng project trên Linux. Hãy chạy project bằng user thường vì
super-user tự động bật `-A` theo manual.

Ubuntu/Debian có thể cài bằng:

```sh
sudo apt update
sudo apt install -y git build-essential python3
```

Fedora/RHEL có thể cài bằng:

```sh
sudo dnf install -y git gcc make python3
```

Không cần quyền `root` để build hoặc chạy chương trình.

**Nếu dùng Windows:** mở terminal Ubuntu của WSL và thực hiện các bước Linux
trong tài liệu này ở đó. Không chạy `make` trong PowerShell/CMD hoặc dùng GCC
Windows để build vì source sử dụng API POSIX. Nếu chưa có Ubuntu/WSL, dùng một
máy Linux hoặc cài Ubuntu trên WSL trước.

Kiểm tra công cụ đã có:

```sh
git --version
cc --version
make --version
python3 --version
```

Mỗi lệnh phải in thông tin phiên bản, không báo `command not found`.

#### Nếu chạy trên NetBSD

Máy cần kết nối mạng và cấu hình kho package cho `pkgin`. Chỉ bước cài công
cụ cần quyền root; gõ từng lệnh riêng, nhập mật khẩu khi `su -` yêu cầu:

```sh
su -
export PATH=/usr/pkg/bin:/usr/pkg/sbin:$PATH
pkgin update
pkgin install git gmake python312
exit
export PATH=/usr/pkg/bin:/usr/pkg/sbin:$PATH
git --version
cc --version
gmake --version
python3.12 --version
```

Sau đó chạy bước 2–3 bằng user thường. Trong các bước tiếp theo dùng `gmake`
thay cho `make` của NetBSD; Python được gọi bằng `python3.12`.

### 2. Tải code từ GitHub

Repository: [Midterm Project - Implement ls](https://github.com/Letien06/Midterm-Project---Implement-ls-).
Sao chép và chạy nguyên các lệnh bên dưới; không cần thay username hay đường dẫn.

Chạy trong một thư mục chưa có thư mục con `LeCaoSonTien_24IT270_midterm`:

```sh
git clone https://github.com/Letien06/Midterm-Project---Implement-ls-.git LeCaoSonTien_24IT270_midterm
cd LeCaoSonTien_24IT270_midterm
```

Nếu đã tải code bằng **Code → Download ZIP**, giải nén và mở terminal tại thư
mục chứa `Makefile`; bỏ qua hai lệnh clone ở trên và bắt đầu từ bước 3.
Tên thư mục khi giải nén ZIP của GitHub có thể có thêm hậu tố `-main`.

### 3. Kiểm tra đang đứng đúng thư mục

```sh
pwd
test -f Makefile && test -d src && test -d include && echo "Project root: OK"
```

Kết quả cần có:

```text
Project root: OK
```

### 4. Build sạch

Chạy lần lượt:

```sh
make clean
make
```

Kiểm tra executable:

```sh
test -x ./ls && echo "Build: OK"
```

Kết quả cần có:

```text
Build: OK
```

Chương trình của project phải được gọi bằng `./ls`, không gọi `ls`, vì `ls`
không có tiền tố `./` sẽ chạy chương trình hệ thống.

#### Build riêng trên NetBSD

GNU Make là `gmake`. Khai báo đầy đủ CPPFLAGS để giữ include path và bật
API mở rộng của NetBSD, gồm timestamp trong `struct stat` và `major()`/`minor()`:

```sh
NETBSD_FLAGS='-Iinclude -D_POSIX_C_SOURCE=200809L -D_DEFAULT_SOURCE -D_NETBSD_SOURCE -D_FILE_OFFSET_BITS=64'
gmake clean
gmake CPPFLAGS="$NETBSD_FLAGS"
test -x ./ls && echo "Build: OK"
gmake CPPFLAGS="$NETBSD_FLAGS" PYTHON=python3.12 test
echo $?
```

Kết quả cuối là `Build: OK`, `Ran 16 tests ...`, `OK` và `0`.
Các target làm build sạch phải tiếp tục truyền CPPFLAGS:

```sh
gmake CPPFLAGS="$NETBSD_FLAGS" PYTHON=python3.12 verify
gmake CPPFLAGS="$NETBSD_FLAGS" demo
```

### 5. Chạy thử các lệnh cơ bản

```sh
./ls
./ls -laF
./ls -lR src
./ls -n Makefile
./ls -i README.md
./ls -s README.md
./ls -d src
```

Các giá trị như inode, owner, group, block và thời gian có thể khác giữa các
máy. Các đặc điểm cần quan sát:

- `./ls`: mỗi entry được in trên một dòng.
- `./ls -laF`: có long format, file ẩn và hậu tố phân loại.
- `./ls -lR src`: liệt kê đệ quy và có header cho từng thư mục.
- `./ls -n Makefile`: UID/GID được in dưới dạng số.
- `./ls -i README.md`: có inode ở đầu dòng.
- `./ls -s README.md`: có số block được cấp phát ở đầu dòng.
- `./ls -d src`: chỉ in chính operand `src`, không mở nội dung thư mục.

#### Kiểm tra lần lượt đầy đủ 19 option

Mọi operand trong bảng là file/thư mục có sẵn sau khi clone repository.
Không cần tự thay đường dẫn mẫu.

| Lệnh chạy từ project root | Đặc điểm cần quan sát |
|---|---|
| `./ls -A` | Có file ẩn như `.gitignore`, không có `.` và `..`. |
| `./ls -a` | Có thêm `.` và `..`. |
| `./ls -cl README.md` | Long format dùng thời gian thay đổi trạng thái (ctime), không phải thời gian tạo file. |
| `./ls -d src` | In `src`, không liệt kê các file bên trong. |
| `./ls -F` | Thư mục có hậu tố `/`; demo minh họa thêm `*`, `@`, `\|`. |
| `./ls -f src` | Bỏ sắp xếp; không có một thứ tự cố định để so giữa các filesystem. |
| `./ls -lh README.md` | Kích thước dễ đọc, có hậu tố đơn vị. |
| `./ls -i README.md` | Có số inode ở đầu dòng. |
| `./ls -sk README.md` | Dung lượng cấp phát theo đơn vị 1024 byte. |
| `./ls -l README.md` | Có mode, link count, owner, group, size, date, pathname. |
| `./ls -n README.md` | Owner/group là UID/GID dạng số. |
| `./ls -q src` | Thay ký tự không in được bằng `?`; thấy rõ hơn trong `make demo`. |
| `./ls -R src` | Liệt kê đệ quy. |
| `./ls -r src` | Đảo thứ tự sắp xếp theo tên. |
| `./ls -S src` | Sắp xếp theo kích thước, lớn trước. |
| `./ls -s README.md` | Hiển thị block thực sự cấp phát, không phải số byte logic. |
| `./ls -t src` | File có mtime mới hơn đứng trước. |
| `./ls -ul README.md` | Long format dùng access time (atime). |
| `./ls -w src` | In nguyên tên, thấy khác biệt với `-q` trong demo có ký tự newline. |

Không so output byte-for-byte với GNU `ls`: đề dùng một manual NetBSD rút gọn,
và chương trình này mặc định chỉ in một entry mỗi dòng, kể cả trên terminal.

#### Ví dụ có đáp án cố định

Từ project root, tạo dữ liệu riêng trong thư mục tạm để không phụ thuộc kích
thước và timestamp của source sau mỗi lần cập nhật:

```sh
export LC_ALL=C
example_dir=$(mktemp -d /tmp/ls-example.XXXXXX)
printf 'abc' > "$example_dir/alpha.txt"
printf '1234567890' > "$example_dir/beta.txt"
printf 'hidden' > "$example_dir/.hidden"
mkdir "$example_dir/sub"
./ls -A "$example_dir"
```

Đáp án của lệnh cuối (mỗi tên một dòng):

```text
.hidden
alpha.txt
beta.txt
sub
```

`./ls -a "$example_dir"` in thêm `.` và `..` ở đầu. Với hai operand file có
kích thước 3 và 10 byte, lệnh sau cho đáp án chính xác:

```sh
./ls -S "$example_dir/alpha.txt" "$example_dir/beta.txt"
```

```text
/tmp/ls-example.XXXXXX/beta.txt
/tmp/ls-example.XXXXXX/alpha.txt
```

`XXXXXX` trong đáp án là hậu tố thực tế do `mktemp` sinh. Đảo lại bằng `-Sr`
thì alpha đứng trước beta. `./ls -d "$example_dir/sub"` chỉ in đường dẫn sub.
`./ls -lh "$example_dir/alpha.txt"` có trường size `3B`; mode, owner, group
và date do môi trường quyết định. Thư mục ví dụ được giữ lại để thử tiếp.

### 6. Chạy bộ test tự động

```sh
make test
```

Kết quả cuối cùng cần có dạng:

```text
Ran 16 tests in ...

OK
```

Kiểm tra exit status:

Chạy lệnh sau **ngay sau `make test`**, chưa chạy lệnh nào khác, vì `$?` là mã
thoát của lệnh vừa thực hiện:

```sh
echo $?
```

Kết quả phải là:

```text
0
```

Test suite dùng dữ liệu tạm và tự dọn dẹp. Nó không sửa source code và không
phụ thuộc vào output của GNU `ls` làm đáp án tuyệt đối.

### 7. Chạy kiểm tra đầy đủ bằng một lệnh

```sh
make verify
```

Target này thực hiện `make clean`, build lại chương trình và chạy toàn bộ test.

### 8. Chạy demo chức năng

```sh
make demo
```

Demo tạo một thư mục tạm chứa file thường, file ẩn, executable, symbolic link,
FIFO, cây thư mục con, tên bắt đầu bằng `-` và tên chứa newline. Demo chạy các
lệnh bao phủ cả 19 option và in lệnh trước từng kết quả. Thời gian, UID/GID,
inode và số block không phải giá trị cố định; `-f` không có thứ tự cố định.
Dữ liệu tạm được tự động xóa khi script kết thúc.

Khi thành công, dòng cuối có nội dung:

```text
Demo completed successfully. Temporary data will now be removed.
```

### 9. Kiểm tra sanitizer - tùy chọn

```sh
make sanitize
make test
```

Sau đó có thể quay lại bản release bình thường bằng:

```sh
make clean
make
```

### 10. Dọn sản phẩm build

```sh
make clean
```

Lệnh này chỉ xóa `build/` và executable `./ls`.

### 11. Danh sách option đã triển khai

```text
-A -a -c -d -F -f -h -i -k -l -n -q -R -r -S -s -t -u -w
```

Chi tiết thiết kế và quy tắc ghi đè option nằm trong `README.md`.

## 12. Chạy riêng đủ 19 tùy chọn

Đây là phần dùng khi đề yêu cầu chứng minh đã chạy **đủ 19 tùy chọn**. Mỗi
lệnh `run_one` bên dưới chỉ gọi một option mục tiêu; các option `-c`, `-h`,
`-k`, `-u` thêm `-l` hoặc `-s` làm ngữ cảnh hiển thị vì manual quy định chúng
sửa timestamp hoặc đơn vị của một tùy chọn khác. Không gộp các option còn lại.

Chuẩn bị dữ liệu kiểm soát một lần trong project root:

```sh
export LC_ALL=C
export TZ=UTC
FULL_DIR=$(mktemp -d /tmp/ls-full.XXXXXX)
printf 'small\n' > "$FULL_DIR/alpha.txt"
printf 'this is a larger file\n' > "$FULL_DIR/beta.txt"
printf 'hidden\n' > "$FULL_DIR/.hidden"
printf 'newline name\n' > "$FULL_DIR/line"
NONPRINTABLE_NAME=$(printf 'line\nbreak')
printf 'nonprintable name\n' > "$FULL_DIR/$NONPRINTABLE_NAME"
mkdir -p "$FULL_DIR/sub/deep"
printf 'nested\n' > "$FULL_DIR/sub/deep/nested.txt"
printf '#!/bin/sh\necho demo\n' > "$FULL_DIR/executable.sh"
chmod 755 "$FULL_DIR/executable.sh"
ln -s alpha.txt "$FULL_DIR/link-to-alpha"
mkfifo "$FULL_DIR/named-pipe"
```

Trên Linux dùng `make`; trên NetBSD thay bằng `gmake` và build trước bằng
`NETBSD_FLAGS` như mục 4. Sau đó chạy từng dòng riêng trong nhóm dưới đây:

```sh
run_one() {
    printf '\n[%s/19] %s\n' "$1" "$2"
    shift 2
    "$@"
    code=$?
    printf 'exit=%s\n' "$code"
    [ "$code" -eq 0 ] || FAILED=$((FAILED + 1))
}
FAILED=0
run_one 01 '-A' ./ls -A "$FULL_DIR"
run_one 02 '-a' ./ls -a "$FULL_DIR"
run_one 03 '-c (context -l)' ./ls -cl README.md
run_one 04 '-d' ./ls -d "$FULL_DIR/sub"
run_one 05 '-F' ./ls -F "$FULL_DIR"
run_one 06 '-f' ./ls -f "$FULL_DIR"
run_one 07 '-h (context -l)' ./ls -hl "$FULL_DIR/alpha.txt"
run_one 08 '-i' ./ls -i "$FULL_DIR/alpha.txt"
run_one 09 '-k (context -s)' ./ls -sk "$FULL_DIR/alpha.txt"
run_one 10 '-l' ./ls -l "$FULL_DIR/alpha.txt"
run_one 11 '-n' ./ls -n "$FULL_DIR/alpha.txt"
run_one 12 '-q' ./ls -q "$FULL_DIR"
run_one 13 '-R' ./ls -R "$FULL_DIR"
run_one 14 '-r' ./ls -r "$FULL_DIR"
run_one 15 '-S' ./ls -S "$FULL_DIR"
run_one 16 '-s' ./ls -s "$FULL_DIR/alpha.txt"
run_one 17 '-t' ./ls -t "$FULL_DIR"
run_one 18 '-u (context -l)' ./ls -ul "$FULL_DIR/alpha.txt"
run_one 19 '-w' ./ls -w "$FULL_DIR"
printf '\nFAILED=%s\n' "$FAILED"
```

Kết quả cần đạt: mỗi mục có `exit=0`, dòng cuối là `FAILED=0`. Khi xem output,
đối chiếu các đặc điểm sau: `-a` có `.`, `..`; `-A` bỏ hai mục này; `-d` chỉ in
operand `sub`; `-F` có `/`, `*`, `@`, `|`; `-h` in kích thước dễ đọc; `-i` có
inode; `-k` và `-s` có block; `-l` có metadata; `-n` có UID/GID; `-R` có
`sub:` và `deep:`; `-r` đảo tên; `-S` xếp file lớn trước; `-t` xếp mtime mới
trước; `-u` dùng access time; `-q` đổi newline trong tên thành `?`, còn `-w`
giữ tên thô. `-c` dùng ctime trong long format.

Chụp ảnh thành ba phần để chữ không quá nhỏ: mục 01–07, 08–14 và 15–19.
Giữ lại dòng `[xx/19]`, lệnh, output và `exit=0`; ảnh cuối phải có `FAILED=0`.
Sau khi chụp xong, dọn thư mục tạm:

```sh
rm -rf "$FULL_DIR"
```



### 12. Xử lý lỗi thường gặp

| Thông báo/hiện tượng | Cách xử lý |
|---|---|
| `git`, `cc`, `make` hoặc `python3`: `command not found` | Cài công cụ theo bước 1, sau đó kiểm tra lại phiên bản. |
| `Repository not found` khi clone | Kiểm tra URL và quyền truy cập; repository public có thể clone mà không cần đăng nhập. |
| `destination path 'LeCaoSonTien_24IT270_midterm' already exists` | Nếu đó là bản code cần chấm, mở thư mục đó bằng `cd LeCaoSonTien_24IT270_midterm`; nếu muốn clone mới, mở terminal tại một thư mục khác chưa có thư mục con cùng tên. |
| Không tìm thấy `Makefile` hoặc không có target `test` | Đang đứng sai thư mục; quay lại bước 3 và mở thư mục chứa `Makefile` của bài nộp. |
| `./ls: No such file or directory` | Chạy `make` và kiểm tra có dòng `Build: OK` ở bước 4. Nếu build báo lỗi, xử lý lỗi build trước. |
| Lỗi header/hàm POSIX trên Windows | Chạy trong Ubuntu/WSL hoặc Linux như bước 1. |
| NetBSD báo thiếu `st_atimespec`, `major` hoặc `minor` | Build sạch bằng `gmake` với toàn bộ `NETBSD_FLAGS` ở bước 4. |
| Inode, UID/GID, thời gian, block khác hình minh họa hoặc máy khác | Đây là dữ liệu phụ thuộc máy; đối chiếu đặc điểm trong bảng ở bước 5 và kết quả `make test`. |

Nếu `make test` báo `FAIL` hoặc `ERROR`, lưu toàn bộ output build/test để xác
định test nào lỗi. `OK` cùng mã thoát `0` mới là kết quả kiểm thử thành công.

## Project overview

This project is a C11/POSIX implementation of the `ls(1)` subset defined by
the supplied NetBSD manual. It deliberately implements one entry per line,
rather than adding unrelated GNU `ls` features such as colours or columns.

## Build and run

The following commands use GNU Make on Linux/WSL. For NetBSD, use the full
`gmake` commands and `NETBSD_FLAGS` in the instructor guide above:

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
