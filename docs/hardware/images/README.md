# Hardware Listing Images

This folder is for storing the 4 listing images shared in chat on 2026-03-01.

Expected filenames:
- `listing2_spec_table_1.png`
- `listing2_spec_table_2.png`
- `listing2_pinmap.png`
- `listing2_dimensions_and_pcb.png`

Why this exists:
- Chat attachments are not directly exportable to files from this runtime.
- Keeping the images in-repo here preserves the hardware pin-map reference for future work.

After adding the files, keep this README and commit them with:
```bash
git add docs/hardware/images/*.png docs/hardware/images/README.md
git commit -m "docs: add listing 2 board reference images"
```
