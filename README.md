# MultiRayTriangulator (Nuke 16 NDK Plugin)

Interactive Multi-ray Least Squares Triangulator node for Foundry Nuke 16, built with the **Nuke Development Kit (NDK)**.

This plugin solves the inverse computer vision problem: it takes an animated 2D tracker position on screen across multiple frames, combines it with camera transformations, projects 3D rays into world space, and computes the **optimal 3D intersection point** using a closed-form Least Squares mathematical model.

## 🚀 Key Features

* **Analytic Least Squares Core**: Computes the exact 3D point in a single pass without expensive iterative optimization solvers.
* **Smart UI Layout**: Vertical parameter stack starting directly with `screen_pos` for swift artist interaction.
* **Keyframe-Only Mode (Default)**: Dynamically filters empty timeline tracks to process only useful frames marked with animation keys.
* **Polygonal Canvas Overlay**: Custom interactive crosshair drawn using true OpenGL polygon rects—guaranteeing stable thickness proportions regardless of the Viewer Zoom level.
* **Automated 3D Sphere Group**: Generates a pre-linked Nuke 3D Group Container with a red sphere aligned to the calculated coordinates via Python script.

## 🛠️ Mathematical Model

The intersection of multiple 3D rays is modeled by minimizing the sum of squared distances from the target 3D point \(X\) to each camera ray defined by origin \(C_i\) and normalized direction vector \(d_i\):

\[M \cdot X = b\]

Where the projection matrix \(P_i\) for each ray is:
\[P_i = I - d_i \cdot d_i^T\]

The cumulative system matrices are computed as:
\[M = \sum P_i, \quad b = \sum P_i \cdot C_i\]

The exact 3D coordinates are found using Cramer's rule to invert the \(3\times3\) matrix \(M\), ensuring maximum performance and zero memory allocations.

## 📦 Project Structure

* `MultiRayTriangulator.h` — Main Iop node class definition.
* `MultiRayTriangulator.cpp` — Node constructor, stream validation (`_validate`), and pixel engine wrappers.
* `knobs.cpp` — Interface definitions (`knobs()`) with specific `STARTLINE` stacking and automated TCL frame expressions.
* `knob_changed.cpp` — Interactive event listener controlling timeline vector iteration and triggering the math solver.
* `build_handles.cpp` — Viewer interactive overlay logic featuring camera NDC-to-pixel projection and full alpha opacity override.
* `ls_triangulation.h` — Header-only analytic matrix math solver.
* `Makefile` — Production-ready GCC compilation script.

## ⚙️ Compilation & Installation (Linux GCC 11+)

The plugin requires **C++17** ABI compatibility to lock with `libDDImage.so` inside Nuke 16.0v8.

### 1. Build and Deploy
Open your terminal inside the project directory and run:
```bash
make all
```
This target automatically triggers `clean`, compiles the shared library object (`build/MultiRayTriangulator.so`), and deploys it straight to your pipeline plugins directory.

### 2. Nuke Environment Registration
Add the following command to your studio `menu.py` file to hook the plugin into the Nuke UI node toolbar:
```python
import nuke
nuke.menu('Nodes').addCommand('Custom/MultiRayTriangulator', 'nuke.createNode("MultiRayTriangulator")')
```

## 📜 License
Internal Studio Tool / Production Ready.
