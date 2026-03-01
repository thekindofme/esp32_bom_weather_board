# Hardware Listing Images

This folder is for storing the 4 listing images shared in chat on 2026-03-01.

Current files in this folder (JPEG):
- `listing2_spec_table_connectivity.jpg`
- `listing2_spec_table_power_env.jpg`
- `listing2_pinmap_io_touch_tf.jpg`
- `listing2_dimensions_pcb_views.jpg`

Why this exists:
- Chat attachments are not directly exportable to files from this runtime.
- Keeping the images in-repo here preserves the hardware pin-map reference for future work.

To commit updates in this folder:
```bash
git add docs/hardware/images/* docs/hardware/images/README.md
git commit -m "docs: update listing 2 board reference images"
```
