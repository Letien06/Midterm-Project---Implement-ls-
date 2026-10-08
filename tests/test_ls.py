#!/usr/bin/env python3
"""Regression tests for the documented subset of the ls manual."""

from __future__ import annotations

import os
import pwd
import re
import socket
import stat
import subprocess
import sys
import tempfile
import time
import unittest
from pathlib import Path
from typing import Dict, Optional


if len(sys.argv) != 2:
    raise SystemExit(f"usage: {sys.argv[0]} /path/to/ls")

BINARY = Path(sys.argv.pop()).resolve()


class LsTests(unittest.TestCase):
    def setUp(self) -> None:
        self.temporary = tempfile.TemporaryDirectory(prefix="ls-midterm-")
        self.root = Path(self.temporary.name)

    def tearDown(self) -> None:
        self.temporary.cleanup()

    def run_ls(self, *arguments: str, cwd: Optional[Path] = None,
               extra_environment: Optional[Dict[str, str]] = None) -> subprocess.CompletedProcess:
        environment = os.environ.copy()
        environment["LC_ALL"] = "C"
        environment["TZ"] = "UTC"
        environment.pop("BLOCKSIZE", None)
        if extra_environment:
            environment.update(extra_environment)
        return subprocess.run(
            [str(BINARY), *arguments],
            cwd=cwd or self.root,
            env=environment,
            text=True,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            check=False,
        )

    def assert_success(self, result: subprocess.CompletedProcess[str]) -> None:
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(result.stderr, "")

    def test_default_name_sort_and_reverse(self) -> None:
        (self.root / "bravo").write_text("b", encoding="utf-8")
        (self.root / "alpha").write_text("a", encoding="utf-8")
        normal = self.run_ls()
        reverse = self.run_ls("-r")
        self.assert_success(normal)
        self.assert_success(reverse)
        self.assertEqual(normal.stdout, "alpha\nbravo\n")
        self.assertEqual(reverse.stdout, "bravo\nalpha\n")

    def test_hidden_entry_modes(self) -> None:
        (self.root / "visible").touch()
        (self.root / ".hidden").touch()
        almost_all = self.run_ls("-A")
        all_entries = self.run_ls("-a")
        a_then_A = self.run_ls("-aA")
        A_then_a = self.run_ls("-Aa")
        self.assert_success(almost_all)
        self.assert_success(all_entries)
        self.assert_success(a_then_A)
        self.assert_success(A_then_a)
        self.assertEqual(almost_all.stdout, ".hidden\nvisible\n")
        self.assertEqual(all_entries.stdout, ".\n..\n.hidden\nvisible\n")
        self.assertEqual(a_then_A.stdout, ".hidden\nvisible\n")
        self.assertEqual(A_then_a.stdout, ".\n..\n.hidden\nvisible\n")

    def test_size_and_time_sorting(self) -> None:
        (self.root / "small").write_bytes(b"1")
        (self.root / "large").write_bytes(b"123456789")
        now = time.time_ns()
        os.utime(self.root / "small", ns=(now, now))
        os.utime(self.root / "large", ns=(now - 5_000_000_000, now - 5_000_000_000))

        by_size = self.run_ls("-S")
        by_time = self.run_ls("-t")
        oldest_first = self.run_ls("-tr")
        self.assert_success(by_size)
        self.assert_success(by_time)
        self.assert_success(oldest_first)
        self.assertEqual(by_size.stdout, "large\nsmall\n")
        self.assertEqual(by_time.stdout, "small\nlarge\n")
        self.assertEqual(oldest_first.stdout, "large\nsmall\n")

    def test_access_and_change_time_precedence(self) -> None:
        first = self.root / "first"
        second = self.root / "second"
        first.touch()
        second.touch()
        now = time.time_ns()
        os.utime(first, ns=(now, now - 10_000_000_000))
        os.utime(second, ns=(now - 10_000_000_000, now - 10_000_000_000))
        time.sleep(0.02)
        second.chmod(0o600)

        access_last = self.run_ls("-tcu")
        change_last = self.run_ls("-tuc")
        self.assert_success(access_last)
        self.assert_success(change_last)
        self.assertEqual(access_last.stdout.splitlines()[0], "first")
        self.assertEqual(change_last.stdout.splitlines()[0], "second")

    def test_classification_suffixes(self) -> None:
        (self.root / "directory").mkdir()
        executable = self.root / "executable"
        executable.write_text("#!/bin/sh\n", encoding="utf-8")
        executable.chmod(0o755)
        os.mkfifo(self.root / "fifo")
        os.symlink("executable", self.root / "link")
        socket_path = self.root / "socket"
        unix_socket = socket.socket(socket.AF_UNIX)
        try:
            unix_socket.bind(str(socket_path))
            result = self.run_ls("-F")
        finally:
            unix_socket.close()

        self.assert_success(result)
        self.assertEqual(
            result.stdout,
            "directory/\nexecutable*\nfifo|\nlink@\nsocket=\n",
        )

    def test_directory_as_file_and_symlink_long_output(self) -> None:
        directory = self.root / "directory"
        directory.mkdir()
        os.symlink("directory", self.root / "link")

        directory_result = self.run_ls("-dF", "directory")
        link_result = self.run_ls("-ldF", "link")
        self.assert_success(directory_result)
        self.assert_success(link_result)
        self.assertEqual(directory_result.stdout, "directory/\n")
        self.assertRegex(link_result.stdout, r"^l[rwxStTs-]{9} .* link@ -> directory\n$")

    def test_long_numeric_and_name_precedence(self) -> None:
        (self.root / "file").write_text("data", encoding="utf-8")
        numeric = self.run_ls("-ln", "file")
        named = self.run_ls("-nl", "file")
        self.assert_success(numeric)
        self.assert_success(named)

        numeric_fields = numeric.stdout.split()
        named_fields = named.stdout.split()
        self.assertTrue(numeric_fields[2].isdigit())
        self.assertTrue(numeric_fields[3].isdigit())
        expected_owner = pwd.getpwuid(os.getuid()).pw_name
        self.assertEqual(named_fields[2], expected_owner)

    def test_inode_blocks_human_and_kib_precedence(self) -> None:
        (self.root / "file").write_bytes(b"x" * 4097)
        inode = self.run_ls("-i", "file")
        kib_last = self.run_ls("-shk", "file")
        human_last = self.run_ls("-skh", "file")
        self.assert_success(inode)
        self.assert_success(kib_last)
        self.assert_success(human_last)
        self.assertRegex(inode.stdout, r"^\d+ file\n$")
        self.assertRegex(kib_last.stdout, r"^\d+ file\n$")
        self.assertRegex(human_last.stdout, r"^[0-9.]+[BKMGTP] file\n$")

    def test_blocksize_changes_reported_units(self) -> None:
        (self.root / "file").write_bytes(b"x")
        blocks_512 = self.run_ls("-s", "file", extra_environment={"BLOCKSIZE": "512"})
        blocks_1024 = self.run_ls("-s", "file", extra_environment={"BLOCKSIZE": "1024"})
        self.assert_success(blocks_512)
        self.assert_success(blocks_1024)
        count_512 = int(blocks_512.stdout.split()[0])
        count_1024 = int(blocks_1024.stdout.split()[0])
        self.assertEqual(count_512, count_1024 * 2)

    def test_total_rules_for_redirected_output(self) -> None:
        directory = self.root / "directory"
        directory.mkdir()
        (directory / "file").write_text("data", encoding="utf-8")
        blocks_only = self.run_ls("-s", "directory")
        long_output = self.run_ls("-l", "directory")
        self.assert_success(blocks_only)
        self.assert_success(long_output)
        self.assertFalse(blocks_only.stdout.startswith("total "))
        self.assertTrue(long_output.stdout.startswith("total "))

    def test_recursive_and_directory_override_order(self) -> None:
        tree = self.root / "tree"
        child = tree / "child"
        child.mkdir(parents=True)
        (child / "file").touch()

        recursive = self.run_ls("-R", "tree")
        directory_last = self.run_ls("-Rd", "tree")
        recursive_last = self.run_ls("-dR", "tree")
        self.assert_success(recursive)
        self.assert_success(directory_last)
        self.assert_success(recursive_last)
        self.assertIn("tree:\n", recursive.stdout)
        self.assertIn("tree/child:\n", recursive.stdout)
        self.assertEqual(directory_last.stdout, "tree\n")
        self.assertIn("tree/child:\n", recursive_last.stdout)

    def test_nonprintable_and_last_name_mode_wins(self) -> None:
        (self.root / "bad\nname").touch()
        question = self.run_ls("-wq")
        raw = self.run_ls("-qw")
        self.assert_success(question)
        self.assert_success(raw)
        self.assertEqual(question.stdout, "bad?name\n")
        self.assertEqual(raw.stdout, "bad\nname\n")

    def test_multiple_operands_files_before_directory(self) -> None:
        (self.root / "file").touch()
        directory = self.root / "directory"
        directory.mkdir()
        (directory / "inside").touch()
        result = self.run_ls("directory", "file")
        self.assert_success(result)
        self.assertEqual(result.stdout, "file\n\ndirectory:\ninside\n")

    def test_missing_operand_reports_error_and_continues(self) -> None:
        (self.root / "valid").touch()
        result = self.run_ls("missing", "valid")
        self.assertNotEqual(result.returncode, 0)
        self.assertIn("missing", result.stderr)
        self.assertEqual(result.stdout, "valid\n")

    def test_invalid_option_fails(self) -> None:
        result = self.run_ls("-Z")
        self.assertNotEqual(result.returncode, 0)
        self.assertIn("invalid option", result.stderr)

    def test_special_permission_characters(self) -> None:
        file_path = self.root / "special"
        file_path.touch()
        file_path.chmod(stat.S_IRUSR | stat.S_IWUSR | stat.S_ISUID | stat.S_ISGID)
        result = self.run_ls("-ld", "special")
        self.assert_success(result)
        self.assertRegex(result.stdout, r"^-rwS--S--- ")


if __name__ == "__main__":
    unittest.main(verbosity=2)
