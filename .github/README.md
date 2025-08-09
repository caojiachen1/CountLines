# GitHub Actions Workflows

This repository uses GitHub Actions for automated building, testing, and releasing.

## Workflows

### 1. CI Build (`ci.yml`)
- **Triggers**: Pull requests to main/master branches
- **Purpose**: Validates that the code builds successfully on all platforms
- **Platforms**: Ubuntu, Windows, macOS
- **Actions**: Build and basic test (version check)

### 2. Build and Release (`release.yml`)
- **Triggers**: 
  - Push to main/master branches (creates releases)
  - Pull requests (builds only, no release)
  - Manual workflow dispatch
- **Purpose**: Builds and releases the CountLines tool with automatic versioning
- **Platforms**: Ubuntu, Windows, macOS

## Versioning Strategy

The workflow implements automatic version increment using the format `v<major>.<minor>`:

- **Initial version**: If no tags exist, starts with `v1.0`
- **Version increment**: Increments the minor version (e.g., `v1.0` → `v1.1` → `v1.2`)
- **Tag format validation**: Only processes tags matching `v<major>.<minor>` format
- **Fallback**: If invalid tags exist, starts fresh with `v1.0`

### Examples:
- First release: `v1.0`
- Second release: `v1.1`
- Third release: `v1.2`
- etc.

## Release Process

When code is pushed to the main branch:

1. **Version Detection**: Determines the next version tag
2. **Multi-Platform Build**: Builds native executables for:
   - Linux: `countlines-linux`
   - Windows: `countlines-windows.exe`
   - macOS: `countlines-macos`
3. **Testing**: Validates that executables run and display version
4. **Release Creation**: Creates a GitHub release with:
   - Automatic tag creation
   - Release notes
   - Binary attachments for all platforms

## Manual Release

You can manually trigger a release by:
1. Going to the "Actions" tab in the GitHub repository
2. Selecting "Build and Release" workflow
3. Clicking "Run workflow" 
4. Choosing the branch (usually main/master)

## Release Assets

Each release includes:
- **countlines-linux**: Native Linux executable
- **countlines-windows.exe**: Windows executable  
- **countlines-macos**: macOS executable

All executables are built with optimizations (`-O3`/`/O2`) for maximum performance.