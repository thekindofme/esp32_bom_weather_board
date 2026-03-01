# Hardware Listing Images

This folder is for storing the 4 listing images shared in chat on 2026-03-01.

Current files in this folder:
- `S840ed15c66df436ea07db1a7bd701434g.webp` (spec table 1)
- `S8e05e62818f246728175e8d4dcbc0e14R.webp` (spec table 2)
- `S9db9453d5c264104a270fb23658309c2g.webp` (pin map)
- `Sfc9ac38070b44925a2608fa678837eafM.webp` (dimensions + pcb)

Why this exists:
- Chat attachments are not directly exportable to files from this runtime.
- Keeping the images in-repo here preserves the hardware pin-map reference for future work.

If you later rename these to friendlier names, commit with:
```bash
git add docs/hardware/images/* docs/hardware/images/README.md
git commit -m "docs: organize listing 2 board reference images"
```
