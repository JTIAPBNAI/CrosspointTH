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
