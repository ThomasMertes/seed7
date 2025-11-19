# GitHub Actions Workflows for Seed7

This directory contains GitHub Actions workflows for automatically building Seed7 binaries across multiple platforms and architectures.

## Workflows

### 1. Build Seed7 Binaries (`build.yml`)

This workflow builds the core Seed7 interpreter (s7) and compiler (s7c) for multiple platforms and architectures.

**Triggers:**
- Push to `master` or `main` branches
- Pull requests to `master` or `main` branches
- Release creation
- Manual trigger via workflow_dispatch

**Platforms & Architectures:**
- **Linux**:
  - AMD64 (x86_64) - native build on `ubuntu-latest`
  - ARM64 (aarch64) - native build on `ubuntu-24.04-arm` runner
- **macOS**:
  - AMD64 (Intel) - native build on `macos-15-intel` runner
  - ARM64 (Apple Silicon M1/M2/M3) - native build on `macos-latest` runner
- **Windows**:
  - x64 - using MinGW/MSYS2 on `windows-latest`

**Build Strategy:**
- Uses matrix builds for efficient parallel compilation across all platforms/architectures
- Native builds on appropriate runners for each architecture (no cross-compilation)
- `fail-fast: false` ensures all platform builds complete even if one fails
- Tests run natively on all platforms including ARM64

**Build Steps:**
1. Install platform-specific dependencies (gcc, make, X11, ncurses)
2. Run `make depend` to generate dependencies
3. Run `make` to build the interpreter (s7)
4. Run `make s7c` to build the compiler (s7c)
5. Run `make test` to execute the test suite (continues on error)
6. Verify architecture (macOS only, using `lipo` and `file` commands)
7. Package binaries and libraries into platform-specific archives
8. Upload artifacts for download

**Artifacts:**
- `seed7-linux-amd64.tar.gz` - Linux AMD64 binaries
- `seed7-linux-arm64.tar.gz` - Linux ARM64 binaries
- `seed7-macos-amd64.tar.gz` - macOS Intel binaries
- `seed7-macos-arm64.tar.gz` - macOS Apple Silicon binaries
- `seed7-windows-x64.zip` - Windows x64 binaries

**Release Handling:**
When a GitHub release is created, the workflow automatically attaches all platform and architecture binaries to the release.

### 2. Build Seed7 Utilities (`build-utils.yml`)

This workflow builds additional Seed7 utilities and tools for all supported architectures.

**Triggers:**
- Push to `master` or `main` branches (only when `prg/**` changes)
- Pull requests to `master` or `main` branches (only when `prg/**` changes)
- Manual trigger via workflow_dispatch

**Platforms & Architectures:**
- Linux: AMD64 and ARM64
- macOS: AMD64 (Intel) and ARM64 (Apple Silicon)
- Windows: x64

**Built Utilities:**
- `bas7` - BASIC interpreter
- `calc7` - Calculator
- `db7` - Database tool
- `diff7` - Diff utility
- `find7` - Find utility
- `ftp7` - FTP client
- `ide7` - IDE
- `make7` - Make utility
- `sql7` - SQL tool
- `tar7` - TAR utility
- And others as defined in the Seed7 project

**Artifacts:**
- `seed7-utils-linux-amd64.tar.gz`
- `seed7-utils-linux-arm64.tar.gz`
- `seed7-utils-macos-amd64.tar.gz`
- `seed7-utils-macos-arm64.tar.gz`
- `seed7-utils-windows-x64.zip`

### 3. Nightly Builds (`nightly.yml`)

Scheduled builds for continuous integration across all platforms and architectures.

**Triggers:**
- Scheduled daily at 2:00 AM UTC
- Manual trigger via workflow_dispatch

**Features:**
- Runs daily at 2:00 AM UTC
- Uses matrix strategy for all platform/architecture combinations
- Builds both core binaries and utilities
- Generates build reports with status indicators
- Auto-cleanup of old artifacts (keeps last 3, deletes after 7 days)
- Artifacts retained for 7 days
- Includes architecture verification for macOS builds

**Build Matrix:**
- Linux AMD64
- Linux ARM64
- macOS AMD64 (Intel)
- macOS ARM64 (Apple Silicon)
- Windows x64

**Build Reports:**
Each build generates a report showing:
- Build date and time
- Platform and architecture
- Runner used
- Build status
- Success/failure for interpreter and compiler

## Architecture-Specific Details

### Linux ARM64 Native Builds

The Linux ARM64 builds use GitHub's native ARM runners (public preview):
- Runner: `ubuntu-24.04-arm` (available for public repositories only)
- Native compilation using standard `gcc` toolchain
- All tests run natively on ARM64 hardware
- **Important**: ARM runners are only available for public repositories
- May experience longer queue times during peak hours (public preview)

### macOS Architecture Separation

macOS builds use different runners for different architectures:
- **Intel (AMD64)**: `macos-15-intel` runner - latest macOS 15 with Intel support
- **Apple Silicon (ARM64)**: `macos-latest` runner - automatically tracks the newest Apple Silicon runner

Each build includes architecture verification using:
- `file` command to show binary type
- `lipo -info` to display architecture information

**Note**: `macos-latest` currently points to an ARM64 runner and will automatically update to newer ARM runners as GitHub releases them.

### Windows Build

Windows builds use:
- MSYS2 environment with MINGW64
- Native Windows runners (always x64)
- Uses 7zip for packaging instead of tar

## Usage

### Automatic Builds

The workflows run automatically on:
- Every push to master/main
- Every pull request to master/main
- Every release creation
- Daily at 2:00 AM UTC (nightly builds)

### Manual Builds

To trigger a manual build:
1. Go to the "Actions" tab in your GitHub repository
2. Select the workflow you want to run
3. Click "Run workflow"
4. Select the branch and click "Run workflow"

### Downloading Artifacts

After a successful workflow run:
1. Go to the "Actions" tab
2. Click on the workflow run
3. Scroll down to the "Artifacts" section
4. Download the desired platform/architecture binaries

**Artifact Retention:**
- Regular builds: 30 days
- Nightly builds: 7 days

### Using in Releases

When creating a new release:
1. Create a new release on GitHub
2. The workflow will automatically build binaries for all platforms and architectures
3. The binaries will be attached to the release as downloadable assets

## Dependencies

### Linux (both AMD64 and ARM64)
- gcc
- make
- libx11-dev
- libncurses5-dev

### macOS
- gcc
- make
- ncurses
- XQuartz (optional, for X11 support)

### Windows
- MSYS2 with MINGW64
- mingw-w64-x86_64-gcc
- mingw-w64-x86_64-make
- make

## Troubleshooting

### Test Failures

The test step is configured with `continue-on-error: true`, meaning the workflow will package binaries even if tests fail. This allows you to download and debug failing builds. However, you should always review test results before using the binaries.

**Important**: Core build steps (make depend, make, make s7c) do NOT have `continue-on-error`. If compilation fails, the workflow will fail and no artifacts will be produced. This ensures we never package broken binaries.

### ARM64 Native Runner Limitations

Linux ARM64 builds use GitHub's native ARM runners which have some limitations:
- **Public repositories only**: ARM runners (`ubuntu-24.04-arm`) only work in public repositories
- **Queue times**: May experience longer waits during peak hours (public preview feature)
- **Fallback**: If ARM runners are unavailable, consider switching to cross-compilation or self-hosted ARM runners

### macOS Runner Availability

- `macos-15-intel`: Latest macOS 15 runner with Intel support
- `macos-latest`: Currently points to ARM64 (Apple Silicon) runners, automatically updates to newest version
- Available ARM runners: `macos-latest`, `macos-14`, `macos-15`, `macos-26` (public preview)
- If builds fail, check GitHub's runner availability status or pin to specific versions

### Build Failures

If builds fail:
1. Check the workflow logs in the Actions tab
2. Verify that the appropriate makefile exists (mk_linux.mak, mk_osxcl.mak, mk_msys.mak)
3. Ensure all dependencies are properly installed
4. Check for platform-specific issues in the Seed7 documentation
5. For ARM64 builds, verify cross-compilation toolchain is available

### Missing Artifacts

Some binaries may not be created if:
- The build step failed
- The binary path has changed
- Platform-specific issues prevent compilation
- Cross-compilation dependencies are missing

The packaging steps use `|| true` to continue even if some files are missing.

## Architecture Verification

### Verifying Linux Binaries

```bash
# Check binary architecture
file bin/s7
# Should show: ELF 64-bit LSB executable, x86-64 (for AMD64)
# Should show: ELF 64-bit LSB executable, ARM aarch64 (for ARM64)
```

### Verifying macOS Binaries

```bash
# Check binary architecture
file bin/s7
lipo -info bin/s7
# Should show: x86_64 (for Intel)
# Should show: arm64 (for Apple Silicon)
```

### Verifying Windows Binaries

```bash
# On Windows or using file command
file bin/s7.exe
# Should show: PE32+ executable (console) x86-64
```

## Customization

### Adding New Platforms

To add support for a new platform:
1. Add a new matrix entry in the appropriate workflow
2. Specify the runner, makefile, and build parameters
3. Add platform-specific dependency installation steps
4. Update the packaging and artifact upload steps
5. Add the artifact to the release step

### Adding New Architectures

To add a new architecture (e.g., RISC-V):
1. Check if GitHub provides native runners for the architecture
2. Add a new matrix entry with the architecture and appropriate runner
3. If no native runner exists, set up cross-compilation toolchain
4. Configure makefile for the target architecture (if cross-compiling)
5. Update artifact names and paths
6. Consider test execution (native runners can run tests, cross-compiled cannot)

### Modifying Build Options

To change build options:
1. Edit the makefile copy step to use a different makefile variant
2. Or create a custom makefile configuration
3. Modify the CFLAGS or other build parameters in the makefile
4. Use `sed` commands to patch makefiles as needed

### Changing Artifact Retention

The default retention is:
- 30 days for regular builds
- 7 days for nightly builds

To change this, modify the `retention-days` parameter in the upload-artifact steps.

## Performance Considerations

### Matrix Builds

Using matrix builds allows parallel compilation across different platforms and architectures, significantly reducing total build time.

### Fail-Fast Strategy

`fail-fast: false` at the matrix level means that if one platform/architecture build fails (e.g., Linux ARM64), the other builds (Linux AMD64, macOS, Windows) will continue rather than being cancelled. This allows you to:
- Get binaries for platforms that do build successfully
- Identify which specific platforms have issues in a single workflow run
- Debug platform-specific problems without re-running the entire matrix

**Note**: This does NOT mean builds continue after errors. Critical build steps will still fail the job if they error - we never package broken binaries.

### Artifact Size

Consider the following to reduce artifact size:
- Strip debug symbols using `make strip`
- Compress binaries more aggressively
- Separate debug symbols into separate artifacts
- Only include necessary libraries

## CI/CD Integration

These workflows can be integrated into a larger CI/CD pipeline by:
- Triggering on specific tags or branches
- Adding deployment steps after successful builds
- Integrating with testing frameworks
- Adding code quality checks before building
- Publishing to package repositories
- Creating Docker images with the binaries

## Security Considerations

### Native Builds vs Cross-Compilation

All current builds use native runners (no cross-compilation):
- Linux AMD64: native on ubuntu-latest
- Linux ARM64: native on ubuntu-24.04-arm
- macOS AMD64: native on macos-15-intel
- macOS ARM64: native on macos-latest
- Windows x64: native on windows-latest

Native builds are preferred because:
- Tests can run on the actual target architecture
- Better binary compatibility and performance
- Simpler build configuration

### Dependency Management

- Dependencies are installed from system repositories
- Consider pinning specific versions for reproducible builds
- Regularly update base images and dependencies

### Secrets and Tokens

- `GITHUB_TOKEN` is automatically provided by GitHub Actions
- No additional secrets are required for basic functionality
- Add secrets for deployment or publishing if needed

## License

The workflows follow the same license as the Seed7 project (GPL/LGPL).
