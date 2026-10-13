#!/usr/bin/env python3

# Copyright (C) 2026 CharOfString <root@charofstring.cc>
#
# This program is free software: you can redistribute it and/or modify it under
# the terms of the GNU General Public License as published by the Free Software
# Foundation, either version 3 of the License, or (at your option) any later
# version.
#
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
# details.
#
# You should have received a copy of the GNU General Public License along with
# this program. If not, see <https://www.gnu.org/licenses/>.

"""Check all maintained C++ files without modifying them."""
import argparse
import re
from pathlib import Path
import shutil
import subprocess
import sys


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--build-dir', type=Path, default=Path('build'))
    args = parser.parse_args()
    root = Path(__file__).resolve().parents[1]
    build = args.build_dir.resolve()
    tools = ['clang-format-19', 'clang-tidy-19', 'cpplint']
    missing = [tool for tool in tools if not shutil.which(tool)]
    if missing:
        sys.exit('Missing style tools: ' + ', '.join(missing))
    if not (build / 'compile_commands.json').is_file():
        sys.exit('Configure with CMAKE_EXPORT_COMPILE_COMMANDS=ON and build first.')
    headers = sorted((root / 'src').rglob('*.h'))
    sources = sorted((root / 'src').rglob('*.cc'))
    tests = sorted((root / 'tests').rglob('*.cc'))
    test_headers = sorted((root / 'tests').rglob('*.h'))
    files = [str(p.relative_to(root))
             for p in headers + test_headers + sources + tests]
    for name in files:
        contents = (root / name).read_text()
        if re.search(r'^namespace \w+::', contents, re.MULTILINE):
            sys.exit(f'{name}: expand nested namespace declarations.')
        if re.search(r'^#define \w+_H_\n\S', contents, re.MULTILINE):
            sys.exit(f'{name}: add a blank line after the header guard.')
        if re.search(r'^namespace(?: \w+)? \{\n(?!namespace)\S',
                     contents, re.MULTILINE):
            sys.exit(f'{name}: add a blank line before the namespace body.')

    subprocess.run(['clang-format-19', '--dry-run', '--Werror', *files],
                   cwd=root, check=True)
    subprocess.run(['cpplint', *files], cwd=root, check=True)
    subprocess.run(['clang-tidy-19', '-p', str(build),
                    *[str(p) for p in sources + tests]], cwd=root, check=True)


if __name__ == '__main__':
    main()
