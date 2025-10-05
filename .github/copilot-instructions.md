# GitHub Copilot Instructions for vvflow CFD Suite

**IMPORTANT: AI agents must reference these instructions first before any additional search or context gathering when working with the vvflow repository.**

## Overview

Vvflow is a Computational Fluid Dynamics (CFD) suite implementing the Viscous Vortex Domains (VVD) method - a mesh-free method for directly solving 2D Navier-Stokes equations in Lagrange coordinates.

## Critical Build System Information

**⚠️ NETWORK DEPENDENCIES WARNING ⚠️**

The build system downloads external dependencies (HDF5, Lua, zlib, libarchive, CppUnit) from the internet during compilation. **Builds WILL FAIL in network-restricted environments** with "Could not resolve host" errors.

**Workaround for network restrictions:**
Install system packages instead of relying on external downloads:
```bash
sudo apt-get install libhdf5-dev liblua5.2-dev libarchive-dev zlib1g-dev libcppunit-dev
```

## System Requirements

- **OS**: Ubuntu 22.04 (jammy) or 24.04 (noble) - officially supported
- **Compiler**: GCC 13.3+ with C++11 support
- **CMake**: 3.0 or higher
- **Python**: 3.12+ (for testing)
- **Memory**: 4GB+ RAM recommended for full builds

## Quick Setup Commands

### System Dependencies
```bash
sudo apt-get update
sudo apt-get install build-essential cmake make git liblapack-dev gnuplot
```

### Build Process (⚠️ NEVER CANCEL - can take 20+ minutes due to external downloads)
```bash
git clone https://github.com/vvflow/vvflow.git
cd vvflow
cmake -Bbuild -S. \
    -DCMAKE_BUILD_TYPE=RelWithDebInfo \
    -DCMAKE_INSTALL_PREFIX=$HOME/.local/vvflow \
    -DCMAKE_INSTALL_RPATH=$HOME/.local/vvflow/lib
make -C build -j$(nproc)
make -C build install
```

### Environment Setup
```bash
echo 'export PATH="$HOME/.local/vvflow/bin:$PATH"' >> ~/.bashrc
source ~/.bashrc
```

## Critical Timeout Information

- **CMake configuration**: ~3-5 seconds
- **Full build**: **20+ minutes** (external downloads are unpredictable)
- **Test suite**: **2 minutes**

**NEVER CANCEL** long-running builds - external dependency downloads can be very slow.

## Testing

### Python Test Suite
```bash
# Code style checks (must pass)
black ./pytest --check --diff
flake8 ./pytest

# Run integration tests (requires built binaries)
cd build && ctest --verbose
```

### Manual Validation Workflow
```bash
# 1. Create test simulation
vvflow example/cyl_re600.lua

# 2. Run simulation (takes several minutes)
vvflow cyl_re600.h5

# 3. Process results
vvxtract stepdata_cyl_re600.h5 time body00/force_hydro | head -5

# 4. Visualize (requires gnuplot)
vvplot results_cyl_re600/000150.h5 ./ -BV -x -1,4 --size 480x360

# 5. Generate time series plot
vvxtract stepdata_cyl_re600.h5 time body00/force_holder | gpquick --lines -u 1:3 -o force_plot.png
```

## Repository Structure

### Core Components
- **`libvvhd/`** - Core CFD library implementing VVD method
- **`utils/vvflow/`** - Main simulation binary with embedded Lua interpreter
- **`utils/vvxtract/`** - Data extraction tool for HDF5 results
- **`utils/vvplot/`** - Visualization tool (requires gnuplot)
- **`utils/scripts/`** - Helper utilities (gpquick, vvawk.* family)

### Key Files
- **`example/cyl_re600.lua`** - Reference simulation script (cylinder at Re=600)
- **`ExternalProjects.cmake`** - External dependency definitions
- **`pytest/`** - Python integration test suite

### Generated During Build/Run
- **`stepdata_*.h5`** - Time series data (forces, positions, etc.)
- **`results_*/`** - Simulation snapshots in HDF5 format

## Available Utilities

After successful build and install:

1. **`vvflow`** - Main simulation engine with Lua scripting
2. **`vvxtract`** - Extract data from HDF5 files
3. **`vvplot`** - Create visualizations (requires gnuplot)
4. **`gpquick`** - Quick plotting utility
5. **`vvawk.*`** - Family of awk-based data processing tools:
   - `vvawk.mavg` - Moving averages
   - `vvawk.avg` - Simple averages
   - `vvawk.drv` - Derivatives
   - `vvawk.ampl` - Amplitude analysis

## Common Workflows

### New Simulation Setup
1. Copy and modify `example/cyl_re600.lua`
2. Run `vvflow your_script.lua` to generate .h5 configuration
3. Run `vvflow your_config.h5` to execute simulation
4. Process results with `vvxtract` and visualize with `vvplot`

### Results Analysis
1. List available data: `vvxtract stepdata_*.h5 --list`
2. Extract time series: `vvxtract stepdata_*.h5 time body00/force_hydro`
3. Create plots: pipe to `vvawk.*` and `gpquick`
4. Visualize fields: `vvplot results_*/snapshot.h5 output_dir -BV`

## Troubleshooting

### Build Issues
- **"Could not resolve host" errors**: Expected in network-restricted environments
- **Long build times**: External downloads can be very slow, **DO NOT CANCEL**
- **Missing dependencies**: Install system packages as workaround

### Test Failures
- **Python tests fail**: Ensure binaries are built and installed correctly
- **Visualization tests fail**: Requires gnuplot installation
- **Integration tests timeout**: Allow 30+ minutes for full test suite

### Runtime Issues
- **"vvflow: command not found"**: Check PATH includes installation directory
- **HDF5 errors**: Verify libhdf5-dev is installed
- **Lua script errors**: Check syntax against example scripts

## Development Guidelines

1. **Always test with example simulations** before submitting changes
2. **Run both Python linting and integration tests**
3. **Validate visualization pipeline** if modifying output formats
4. **Consider network restrictions** when modifying build system
5. **Document any new dependencies** or build requirements

## External Dependencies (Downloaded During Build)

- HDF5 1.10.6 - Data storage format
- Lua 5.2.4 - Embedded scripting engine
- zlib 1.2.11 - Compression library
- libarchive 3.4.3 - Archive handling
- CppUnit 1.15.1 - Unit testing framework

These are automatically downloaded from vvflow.github.io mirrors and upstream sources.
