# MuonSimViewer

> **Note:** This is the original prototype, developed in ~1 week.
> The full rewrite is [BeamLabStudio](https://github.com/kegouro/BeamLabStudio).

3D trajectory visualizer for particle simulation data exported from COMSOL Multiphysics.
Built with C++20, Qt6 and VTK.

---

## Features

- COMSOL CSV import (wide and long format autodetection)
- Interactive 3D trajectory visualization
- Timeline control with Play/Pause slider
- Per-frame statistics: active particles, efficiency, r_rms, sigma_x, sigma_z
- Export to PDF report, CSV and MP4

---

## Dependencies

- CMake 3.21+
- Qt6 (Widgets, Charts, PrintSupport, Concurrent)
- VTK 9
- ffmpeg (for MP4 export)

---

## Build

```bash
mkdir build && cd build
cmake .. -G Ninja -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_PREFIX_PATH="/path/to/Qt6;/path/to/VTK"
ninja
```

---

## History

This tool was the first prototype. As requirements grew, it was rebuilt from
scratch as [BeamLabStudio](https://github.com/kegouro/BeamLabStudio) — with
multi-format support (Geant4, COMSOL, CERN ROOT), a separate analysis engine,
and a portable Windows distribution.

---

## License

MIT — José Labarca, 2026
