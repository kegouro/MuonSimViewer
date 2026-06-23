
                  MUONSIMVIEWER v4.0 - README BILINGÜE COMPLETO
              Visualizador 3D de Trayectorias para Simulaciones COMSOL
                               Por José Labarca
                  Jlabarca@usm.cl/Joselabarcabaeza11@hotmail.com
                         "El prototipo que inició todo"


================================================================================

                              ENGLISH VERSION
                              
================================================================================

# MuonSimViewer v4.0

**High-performance C++17/Qt6 + OpenGL application for 3D visualization and analysis of COMSOL Multiphysics particle tracking simulations.**

> **Author's Note (José Labarca):**  
> This was the original prototype — built in roughly one intense week while I was learning COMSOL from zero for the muon beam cancer therapy project at CCTVal.  
> It started as a quick viewer because the built-in COMSOL tools were painful for batch analysis.  
> Then it grew… and grew… until I realized I needed a full rewrite.  
> That rewrite became [BeamLabStudio](https://github.com/kegouro/BeamLabStudio) — the professional, multi-format, cross-platform version.  
> MuonSimViewer remains the spiritual ancestor: the tool that proved the concept and showed what was possible.

**This is v4.0** — significantly enhanced from the original weekend hack. It features dual OpenGL viewports, QtCharts statistics, PDF scientific reports, MP4 video export, and a beautiful "Deep Space Laboratory" dark theme.

---

## THE MANIFESTO (Read This)

This software was created **for science, during exam hell, by a physicist who needed better tools**.  
It exists **for the world** — especially for anyone working on muon beam therapy for cancer.

**COMMERCIAL USE, SALE, OR MONETIZATION OF ANY KIND IS STRICTLY PROHIBITED**  
without my explicit written permission. This applies to MuonSimViewer and **all derivatives** (including the full rewrite BeamLabStudio).

I built the original version unpaid, in one week, while also preparing for my IELTS and surviving Cuántica II.  
If you want to use it commercially, talk to me first.

---

## What is MuonSimViewer?

MuonSimViewer is a specialized 3D visualization and analysis tool that takes raw COMSOL particle tracking CSV exports and turns them into:

- Interactive dual-viewport 3D scenes (Beam View + Trajectory View)
- Real-time animated playback with time slider
- Live particle counters, efficiency tracking, and beam statistics
- Radial distribution histograms and RMS beam spread charts
- Publication-ready PDF reports with embedded 3D screenshots
- MP4 video animations (via ffmpeg)
- CSV statistics exports

It was designed specifically for the muon beam cancer therapy simulations at CCTVal, but works with any COMSOL 3D particle tracking export.

---

## Key Features (Detailed)

### 1. Intelligent COMSOL CSV Parser
- Auto-detects both "wide" format (columns with @ t=...) and "long" format
- Handles Spanish headers, scientific notation, and frozen particle detection
- Automatically computes tube radius, detector plane, and time scaling (s → ns)

### 2. Dual OpenGL 3D Viewports
- **Beam View**: Real-time particle positions inside a semi-transparent tube with entry ring (blue) and detector ring (green)
- **Trajectory View**: Full trajectories colored by initial radius R₀ using a plasma colormap
- MSAA × 4 anti-aliasing, smooth camera controls (orbit + zoom)

### 3. Interactive Timeline
- Slider + Play/Pause animation at 25 fps
- Per-frame statistics update live (in-flight, detected, lost, efficiency, r_rms, σ_x, σ_z)

### 4. Statistical Dashboard
- Live particle counter cards
- Radial histogram (current frame)
- RMS + σ_x + σ_z vs time line chart
- Per-frame statistics table (scrollable)
- Simulation metadata table (model name, nodes, expressions, etc.)

### 5. Professional Export Pipeline
- **PDF Report**: Multi-page scientific document with 3D screenshots, charts, metadata, and statistics table
- **MP4 Video**: Side-by-side dual-view animation (1920×540, H.264, faststart)
- **CSV Statistics**: Per-frame data ready for further analysis

### 6. Deep Space Laboratory Theme
Custom Qt6 dark theme with neon cyan, green, gold, and red accents — optimized for long analysis sessions.

---

## Build & Run (Linux)

```bash
# Quick start (recommended)
chmod +x build_and_run.sh
./build_and_run.sh path/to/your_comsol_data.csv

# Manual build
mkdir build && cd build
cmake .. -G Ninja -DCMAKE_BUILD_TYPE=Release
ninja
./MuonSimViewer /path/to/data.csv
```

**Dependencies (Ubuntu/Debian)**
```bash
sudo apt install qt6-base-dev qt6-charts-dev qt6-svg-dev \
                 cmake build-essential libgl1-mesa-dev ffmpeg
```

**Fedora/RHEL**
```bash
sudo dnf install qt6-qtbase-devel qt6-qtcharts-devel qt6-qtsvg-devel \
                 cmake gcc-c++ mesa-libGL-devel ffmpeg
```

---

## Usage

1. **Open file**: File → Open COMSOL CSV (or pass as command-line argument)
2. **Scrub time**: Drag the bottom slider or press ▶ PLAY
3. **Rotate views**: Click + drag in either 3D viewport; scroll wheel to zoom
4. **Reset cameras**: ↺ RESET button or Ctrl+R
5. **Export PDF**: 📄 PDF button — generates a complete scientific report
6. **Export Video**: 🎬 VIDEO — renders MP4 (requires ffmpeg)
7. **Export Stats**: 💾 CSV — saves per-frame statistics table

---

## COMSOL CSV Format Supported

The parser automatically handles:
- Header comments: `% Model, Version, Date, Nodes, Expressions, Description`
- Column headers with time suffixes: `qx @ t=0.1[s], qy @ t=0.1[s], ...`
- Any number of particles and timesteps
- Scientific notation and very small time values (auto-scales to ns)
- Frozen particle detection (position stops changing → particle stopped)
- Automatic tube radius estimation from 95th percentile of transverse positions

---

## Architecture (For Those Who Want to Modify It)

```
src/
├── main.cpp                 Entry point + theme application
├── SimulationData.h         Core structs (Vec3, Particle, FrameStats, SimulationData)
├── ComsolParser.{h,cpp}     Generic COMSOL CSV parser with progress callbacks
├── Viewport3D.{h,cpp}       OpenGL 3D rendering (QOpenGLExtraFunctions + custom shaders)
├── StatsPanel.{h,cpp}       Scrollable Qt statistics dashboard (QtCharts)
├── PDFExporter.{h,cpp}      QPdfWriter multi-page scientific report
├── VideoExporter.{h,cpp}    QProcess + ffmpeg MP4 encoder
├── MainWindow.{h,cpp}       Main UI orchestration + export coordination
└── AppTheme.h               Global CSS + color palette (Deep Space Laboratory)
```

**Key Technical Details**
- Trajectories are pre-baked into GPU buffers at load time
- Frame updates only change draw counts (no re-upload)
- Parser uses vectorized statistics — handles 2000+ particles × 300 timesteps in < 2 seconds
- OpenGL 3.3 Core Profile + GLSL 330

---

## Performance

- Load time: ~1.5–2.5 seconds for typical 1500-particle × 250-timestep files
- Animation: Smooth 25 fps even on integrated graphics
- Memory: ~80–120 MB for large datasets (trajectories stored as float arrays)

---

## History & Relationship to BeamLabStudio

MuonSimViewer was the **first prototype** — built in one week to prove the concept.  
As requirements grew (Geant4 support, ROOT files, cross-platform distribution, bit-exact reproducibility), it was rebuilt from scratch as **BeamLabStudio**.

MuonSimViewer remains valuable as:
- A lightweight, focused tool for pure COMSOL workflows
- A beautiful reference implementation of Qt6 + OpenGL + QtCharts
- The spiritual ancestor of the larger project

If you need multi-format support (Geant4, COMSOL, CERN ROOT), headless CLI, or Windows portable builds, use **BeamLabStudio** instead.

---

## License + No-Profit Clause

MIT License — **with an important extra clause**:

**Commercial use, sale, or any form of monetization is strictly prohibited** without my explicit written permission.  
This applies to MuonSimViewer and every derivative (including BeamLabStudio).

I built the original version unpaid, during exam season, while learning COMSOL from zero.  
Let's keep it for science.

Full license text is in the `LICENSE` file (same as BeamLabStudio).

---

## Credits

- **Author & Original Concept**: José Labarca (Universidad Santa María, CCTVal — muon beam cancer therapy)
- **Enhanced v4.0 Development**: José Labarca + Claude (Anthropic) for architecture and export modules
- **Libraries**: Qt 6, OpenGL 3.3, QtCharts, QPdfWriter, ffmpeg
- **Inspiration**: The muon group at CCTVal who needed something better than COMSOL's built-in tools
- **Personal Thanks**: To my professors who let me work on this instead of forcing me to use only the official COMSOL post-processor

---

**If you use MuonSimViewer in your research, please cite it and drop me an email.**  
I would love to know that this prototype I built in one week is still helping real science.

**See you at KIT, October 2026.**

— José

================================================================================

                           VERSIÓN EN ESPAÑOL
                           
================================================================================

# MuonSimViewer v4.0

**Aplicación de alto rendimiento en C++17/Qt6 + OpenGL para visualización 3D y análisis de simulaciones de tracking de partículas de COMSOL Multiphysics.**

> **Nota del autor (José Labarca):**  
> Este fue el prototipo original — lo construí en aproximadamente una semana intensa mientras aprendía COMSOL desde cero para el proyecto de terapia con haces de muones contra el cáncer en el CCTVal.  
> Empezó como un visor rápido porque las herramientas nativas de COMSOL eran un dolor para análisis por lotes.  
> Después creció… y creció… hasta que me di cuenta de que necesitaba una reescritura completa.  
> Esa reescritura se convirtió en [BeamLabStudio](https://github.com/kegouro/BeamLabStudio) — la versión profesional, multi-formato y multiplataforma.  
> MuonSimViewer sigue siendo el ancestro espiritual: la herramienta que demostró el concepto y mostró lo que era posible.

**Esta es la v4.0** — significativamente mejorada respecto al hack original de fin de semana. Incluye doble viewport OpenGL, gráficos QtCharts, reportes PDF científicos, exportación de video MP4 y un hermoso tema oscuro "Deep Space Laboratory".

---

## EL MANIFIESTO (Léelo)

Este software fue creado **para la ciencia, durante el infierno de exámenes, por un físico que necesitaba mejores herramientas**.  
Existe **para el mundo** — especialmente para cualquiera que trabaje en terapia con haces de muones contra el cáncer.

**EL USO COMERCIAL, VENTA O CUALQUIER FORMA DE MONETIZACIÓN ESTÁ ESTRICTAMENTE PROHIBIDA**  
sin mi permiso expreso por escrito. Esto aplica a MuonSimViewer y **todos sus derivados** (incluyendo la reescritura completa BeamLabStudio).

Construí la versión original sin cobrar, en una semana, mientras preparaba mi IELTS y sobrevivía Cuántica II.  
Si querés usarlo comercialmente, hablame primero.

---

## ¿Qué es MuonSimViewer?

MuonSimViewer es una herramienta especializada de visualización 3D y análisis que toma exportaciones CSV crudas de tracking de partículas de COMSOL y las convierte en:

- Escenas 3D interactivas con doble viewport (Beam View + Trajectory View)
- Reproducción animada en tiempo real con slider de tiempo
- Contadores de partículas en vivo, seguimiento de eficiencia y estadísticas del haz
- Histogramas de distribución radial y gráficos de dispersión del haz vs tiempo
- Reportes PDF listos para publicación con capturas 3D embebidas
- Animaciones de video MP4 (vía ffmpeg)
- Exportación de estadísticas en CSV

Fue diseñado específicamente para las simulaciones de terapia con haces de muones en el CCTVal, pero funciona con cualquier exportación de tracking de partículas 3D de COMSOL.

---

## Características Principales (Detalladas)

### 1. Parser Inteligente de CSV COMSOL
- Detecta automáticamente formato "ancho" (columnas con @ t=...) y formato "largo"
- Maneja encabezados en español, notación científica y detección de partículas congeladas
- Calcula automáticamente radio del tubo, plano del detector y escalado de tiempo (s → ns)

### 2. Doble Viewport OpenGL 3D
- **Beam View**: Posiciones de partículas en tiempo real dentro de un tubo semi-transparente con anillo de entrada (azul) y anillo detector (verde)
- **Trajectory View**: Trayectorias completas coloreadas por radio inicial R₀ usando un colormap plasma
- MSAA × 4 anti-aliasing, controles de cámara suaves (órbita + zoom)

### 3. Timeline Interactivo
- Slider + reproducción Play/Pause a 25 fps
- Estadísticas por frame se actualizan en vivo (en vuelo, detectadas, perdidas, eficiencia, r_rms, σ_x, σ_z)

### 4. Dashboard Estadístico
- Tarjetas de contador de partículas en vivo
- Histograma radial (frame actual)
- Gráfico de línea RMS + σ_x + σ_z vs tiempo
- Tabla de estadísticas por frame (scrollable)
- Tabla de metadatos de simulación (nombre del modelo, nodos, expresiones, etc.)

### 5. Pipeline de Exportación Profesional
- **Reporte PDF**: Documento científico multi-página con capturas 3D, gráficos, metadatos y tabla de estadísticas
- **Video MP4**: Animación dual-view lado a lado (1920×540, H.264, faststart)
- **Estadísticas CSV**: Datos por frame listos para análisis posterior

### 6. Tema Deep Space Laboratory
Tema oscuro Qt6 personalizado con acentos neón cyan, verde, dorado y rojo — optimizado para sesiones largas de análisis.

---

## Compilación y Ejecución (Linux)

```bash
# Inicio rápido (recomendado)
chmod +x build_and_run.sh
./build_and_run.sh ruta/a/tu_datos_comsol.csv

# Compilación manual
mkdir build && cd build
cmake .. -G Ninja -DCMAKE_BUILD_TYPE=Release
ninja
./MuonSimViewer /ruta/a/datos.csv
```

**Dependencias (Ubuntu/Debian)**
```bash
sudo apt install qt6-base-dev qt6-charts-dev qt6-svg-dev \
                 cmake build-essential libgl1-mesa-dev ffmpeg
```

**Fedora/RHEL**
```bash
sudo dnf install qt6-qtbase-devel qt6-qtcharts-devel qt6-qtsvg-devel \
                 cmake gcc-c++ mesa-libGL-devel ffmpeg
```

---

## Uso

1. **Abrir archivo**: Archivo → Abrir CSV COMSOL (o pasarlo como argumento de línea de comandos)
2. **Mover en el tiempo**: Arrastrá el slider inferior o presioná ▶ PLAY
3. **Rotar vistas**: Clic + arrastrar en cualquier viewport 3D; rueda del mouse para zoom
4. **Resetear cámaras**: Botón ↺ RESET o Ctrl+R
5. **Exportar PDF**: Botón 📄 PDF — genera un reporte científico completo
6. **Exportar Video**: Botón 🎬 VIDEO — renderiza MP4 (requiere ffmpeg)
7. **Exportar Estadísticas**: Botón 💾 CSV — guarda la tabla de estadísticas por frame

---

## Formato CSV COMSOL Soportado

El parser maneja automáticamente:
- Comentarios de encabezado: `% Model, Version, Date, Nodes, Expressions, Description`
- Nombres de columna con sufijos de tiempo: `qx @ t=0.1[s], qy @ t=0.1[s], ...`
- Cualquier cantidad de partículas y timesteps
- Notación científica y valores de tiempo muy pequeños (auto-escala a ns)
- Detección de partículas congeladas (la posición deja de cambiar → partícula detenida)
- Estimación automática del radio del tubo desde el percentil 95 de posiciones transversales

---

## Arquitectura (Para Quienes Quieran Modificarlo)

```
src/
├── main.cpp                 Punto de entrada + aplicación del tema
├── SimulationData.h         Estructuras de datos centrales (Vec3, Particle, FrameStats, SimulationData)
├── ComsolParser.{h,cpp}     Parser genérico de CSV COMSOL con callbacks de progreso
├── Viewport3D.{h,cpp}       Renderizado 3D OpenGL (QOpenGLExtraFunctions + shaders personalizados)
├── StatsPanel.{h,cpp}       Dashboard estadístico Qt scrolleable (QtCharts)
├── PDFExporter.{h,cpp}      Reporte científico multi-página QPdfWriter
├── VideoExporter.{h,cpp}    Codificador MP4 QProcess + ffmpeg
├── MainWindow.{h,cpp}       Orquestación principal de UI + coordinación de exportaciones
└── AppTheme.h               Sistema de diseño CSS global (Deep Space Laboratory)
```

**Detalles Técnicos Clave**
- Las trayectorias se pre-cocinan en buffers GPU al cargar
- Las actualizaciones de frame solo cambian conteos de dibujo (sin re-subir)
- El parser usa estadísticas vectorizadas — maneja 2000+ partículas × 300 timesteps en < 2 segundos
- OpenGL 3.3 Core Profile + GLSL 330

---

## Rendimiento

- Tiempo de carga: ~1.5–2.5 segundos para archivos típicos de 1500 partículas × 250 timesteps
- Animación: Suave 25 fps incluso en gráficos integrados
- Memoria: ~80–120 MB para datasets grandes (trayectorias almacenadas como arrays de float)

---

## Historia y Relación con BeamLabStudio

MuonSimViewer fue el **primer prototipo** — construido en una semana para demostrar el concepto.  
A medida que los requerimientos crecieron (soporte Geant4, archivos ROOT, distribución multiplataforma, reproducibilidad bit-exacta), se reescribió desde cero como **BeamLabStudio**.

MuonSimViewer sigue siendo valioso como:
- Una herramienta liviana y enfocada para flujos de trabajo 100% COMSOL
- Una implementación de referencia hermosa de Qt6 + OpenGL + QtCharts
- El ancestro espiritual del proyecto más grande

Si necesitás soporte multi-formato (Geant4, COMSOL, CERN ROOT), CLI headless o builds portables de Windows, usá **BeamLabStudio** en su lugar.

---

## Licencia + Cláusula No Lucro

Licencia MIT — **con una cláusula extra importante**:

**El uso comercial, venta o cualquier forma de monetización está estrictamente prohibida** sin mi permiso expreso por escrito.  
Esto aplica a MuonSimViewer y toda obra derivada (incluyendo BeamLabStudio).

Construí la versión original sin cobrar, durante época de exámenes, mientras aprendía COMSOL desde cero.  
Mantengámoslo para la ciencia.

El texto completo de la licencia está en el archivo `LICENSE` (el mismo que BeamLabStudio).

---

## Créditos

- **Autor y Concepto Original**: José Labarca (Universidad Santa María, CCTVal — terapia con haces de muones contra el cáncer)
- **Desarrollo Mejorado v4.0**: José Labarca + Claude (Anthropic) para arquitectura y módulos de exportación
- **Librerías**: Qt 6, OpenGL 3.3, QtCharts, QPdfWriter, ffmpeg
- **Inspiración**: El grupo de muones del CCTVal que necesitaba algo mejor que las herramientas nativas de COMSOL
- **Agradecimientos Personales**: A mis profesores que me dejaron trabajar en esto en vez de obligarme a usar solo el post-procesador oficial de COMSOL

---

**Si usás MuonSimViewer en tu investigación, por favor citame y mandame un mail.**  
Me encantaría saber que este prototipo que construí en una semana sigue ayudando a ciencia real.

**Nos vemos en Karlsruhe, octubre 2026.**

— José

================================================================================

Este README fue hecho el 27 de abril de 2026 a las 00:29 (hora de Chile).
Versión: 1.0 - para MuonSimViewer v4.0

---

<sub>Parte del **[Pharos Project](https://kegouro.github.io)** — infraestructura científica y educativa sin barreras de entrada. · José Labarca Baeza</sub>
