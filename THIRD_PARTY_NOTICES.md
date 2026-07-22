# Third-party notices

`crosspointTH` preserves all upstream notices and adds the following Thai-related components.

## Noto Sans and Noto Serif

The built-in font sources are from the Noto font families and are distributed under the SIL Open
Font License 1.1. The full license texts are retained in:

- [`lib/EpdFont/builtinFonts/source/NotoSans/OFL.txt`](./lib/EpdFont/builtinFonts/source/NotoSans/OFL.txt)
- [`lib/EpdFont/builtinFonts/source/NotoSerif/OFL.txt`](./lib/EpdFont/builtinFonts/source/NotoSerif/OFL.txt)

The generated C headers contain subsetted font data and remain subject to that license.

## Sarabun

Sarabun font files and generated `.cpfont` files under `thai-fonts/` are distributed under the SIL
Open Font License 1.1. The OFL text linked above is the license text bundled with this repository;
the Sarabun upstream source is the
[Google Fonts Sarabun family](https://github.com/google/fonts/tree/main/ofl/sarabun).

## Thai dictionary data

`lib/ThaiBrk/ThaiDictData.h` is generated from word lists from
[TLWG/libthai](https://github.com/tlwg/libthai), identified by that project as LGPL-2.1-or-later.
The generated trie is treated as data. Source attribution and the reproducible generator are kept
in `scripts/gen_thai_dict.py` and the generated header.

## Upstream dependencies

CrossPoint Reader and FreeInk SDK dependencies retain the license and notice files already present
in their source directories. See the repository [LICENSE](./LICENSE), `freeink-sdk/LICENSE`, and
`freeink-sdk/NOTICE`.

## CrossInk reading-stat design reference

The lightweight reading-stat persistence and reader menu in `crosspointTH` were designed after
reviewing [uxjulia/CrossInk](https://github.com/uxjulia/CrossInk). The implementation here is
intentionally smaller and does not include CrossInk stats sync, charts, sleep themes, or date
history. CrossInk is distributed under the following MIT license:

Copyright (c) 2025 Dave Allie

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
associated documentation files (the "Software"), to deal in the Software without restriction,
including without limitation the rights to use, copy, modify, merge, publish, distribute,
sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial
portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT
NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES
OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
