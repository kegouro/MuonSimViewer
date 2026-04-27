# MuonSimViewer

> **Nota:** Este fue el prototipo original. La versión completa y reescrita desde cero es [BeamLabStudio](https://github.com/kegouro/BeamLabStudio).

Visualizador 3D de trayectorias de muones exportadas desde COMSOL. Desarrollado en el CCTVal (Centro Científico Tecnológico de Valparaíso, USM) en el contexto de un proyecto de terapia oncológica con haces de muones.

## Contexto

Primer prototipo desarrollado en ~1 semana. Fue la base conceptual para BeamLabStudio, que lo reescribió completamente con soporte multi-formato (Geant4, COMSOL, CERN ROOT), motor de análisis separado, y distribución Windows portable.

## Qué hace

- Abre CSV de COMSOL (formato ancho y largo)
- Visualización 3D con VTK + Qt6
- Slider temporal con Play/Pausa
- Métricas por frame: partículas activas, eficiencia, r_rms, sigma_x, sigma_z
- Exporta PDF, CSV de estadísticas y MP4

## Dependencias

- CMake 3.21+, Qt6, VTK 9, ffmpeg

## Build

```bash
mkdir build && cd build
cmake .. -G Ninja -DCMAKE_BUILD_TYPE=Release
ninja
```

## Licencia

MIT — José Labarca, 2026
