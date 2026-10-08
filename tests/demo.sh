#!/bin/sh
set -eu
export LC_ALL=C
export TZ=UTC

program=${1:-./ls}
case "$program" in
    /*) ;;
    *) program="$(pwd)/${program#./}" ;;
esac

if [ ! -x "$program" ]; then
    printf 'error: executable not found: %s\n' "$program" >&2
    exit 1
fi

demo_root=$(mktemp -d "${TMPDIR:-/tmp}/ls-midterm-demo.XXXXXX")
cleanup() {
    rm -rf "$demo_root"
}
trap cleanup EXIT HUP INT TERM

mkdir -p "$demo_root/directory/subdirectory"
mkdir "$demo_root/names"
nonprintable_name=$(printf 'line\nbreak')
printf 'name demo\n' > "$demo_root/names/$nonprintable_name"
printf 'small\n' > "$demo_root/small.txt"
printf 'this file is intentionally larger than small.txt\n' > "$demo_root/large.txt"
printf '#!/bin/sh\necho demo\n' > "$demo_root/executable.sh"
printf 'nested\n' > "$demo_root/directory/subdirectory/nested.txt"
printf 'hidden\n' > "$demo_root/.hidden"
printf 'dash operand\n' > "$demo_root/-filename"
chmod 755 "$demo_root/executable.sh"
ln -s small.txt "$demo_root/link-to-small"
mkfifo "$demo_root/named-pipe"
touch -t 202401010101 "$demo_root/small.txt"
touch -t 202501010101 "$demo_root/large.txt"

run() {
    printf '\n$'
    for argument in "$@"; do
        printf ' %s' "$argument"
    done
    printf '\n'
    "$@"
}

cd "$demo_root"
printf 'Temporary demonstration directory: %s\n' "$demo_root"
run "$program"
run "$program" -A
run "$program" -a
run "$program" -F
run "$program" -l
run "$program" -n
run "$program" -i
run "$program" -s
run "$program" -lhS
run "$program" -f
run "$program" -sk small.txt
run "$program" -sh small.txt
run "$program" -cl small.txt
run "$program" -ul small.txt
run "$program" -tr
run "$program" -d directory
run "$program" -R directory
run "$program" -q names
run "$program" -w names
run "$program" -- -filename

printf '\nDemo completed successfully. Temporary data will now be removed.\n'
