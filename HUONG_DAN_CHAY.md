# Hướng dẫn tải code, build và kiểm thử trên Linux

Tài liệu này dành cho giảng viên hoặc người chấm bài. Sau khi cài công cụ và
tải code ở bước 1–2, các lệnh từ bước 3 trở đi được chạy từ **thư mục gốc của
repository**, tức thư mục có `Makefile`, `README.md`, `src/` và `include/`.

## 1. Yêu cầu môi trường

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

## 2. Tải code từ GitHub

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

## 3. Kiểm tra đang đứng đúng thư mục

```sh
pwd
test -f Makefile && test -d src && test -d include && echo "Project root: OK"
```

Kết quả cần có:

```text
Project root: OK
```

## 4. Build sạch

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

## 5. Chạy thử các lệnh cơ bản

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

### Kiểm tra lần lượt đầy đủ 19 option

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

## 6. Chạy bộ test tự động

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

## 7. Chạy kiểm tra đầy đủ bằng một lệnh

```sh
make verify
```

Target này thực hiện `make clean`, build lại chương trình và chạy toàn bộ test.

## 8. Chạy demo chức năng

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

## 9. Kiểm tra sanitizer - tùy chọn

```sh
make sanitize
make test
```

Sau đó có thể quay lại bản release bình thường bằng:

```sh
make clean
make
```

## 10. Dọn sản phẩm build

```sh
make clean
```

Lệnh này chỉ xóa `build/` và executable `./ls`.

## 11. Danh sách option đã triển khai

```text
-A -a -c -d -F -f -h -i -k -l -n -q -R -r -S -s -t -u -w
```

Chi tiết thiết kế và quy tắc ghi đè option nằm trong `README.md`.

## 12. Xử lý lỗi thường gặp

| Thông báo/hiện tượng | Cách xử lý |
|---|---|
| `git`, `cc`, `make` hoặc `python3`: `command not found` | Cài công cụ theo bước 1, sau đó kiểm tra lại phiên bản. |
| `Repository not found` khi clone | Kiểm tra URL và quyền truy cập; repository public có thể clone mà không cần đăng nhập. |
| `destination path 'LeCaoSonTien_24IT270_midterm' already exists` | Nếu đó là bản code cần chấm, mở thư mục đó bằng `cd LeCaoSonTien_24IT270_midterm`; nếu muốn clone mới, mở terminal tại một thư mục khác chưa có thư mục con cùng tên. |
| Không tìm thấy `Makefile` hoặc không có target `test` | Đang đứng sai thư mục; quay lại bước 3 và mở thư mục chứa `Makefile` của bài nộp. |
| `./ls: No such file or directory` | Chạy `make` và kiểm tra có dòng `Build: OK` ở bước 4. Nếu build báo lỗi, xử lý lỗi build trước. |
| Lỗi header/hàm POSIX trên Windows | Chạy trong Ubuntu/WSL hoặc Linux như bước 1. |
| Inode, UID/GID, thời gian, block khác hình minh họa hoặc máy khác | Đây là dữ liệu phụ thuộc máy; đối chiếu đặc điểm trong bảng ở bước 5 và kết quả `make test`. |

Nếu `make test` báo `FAIL` hoặc `ERROR`, lưu toàn bộ output build/test để xác
định test nào lỗi. `OK` cùng mã thoát `0` mới là kết quả kiểm thử thành công.
