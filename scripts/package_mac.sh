#!/usr/bin/env bash
# Builds the Release app on macOS, runs the tests and assembles the submission files:
#   output/Team-Nazal-Final-Deliverables.zip   (Desktop Application + Source Code + notices)
#   output/Nazal-macOS.zip                     (the .app on its own)
# Usage, from the project folder:   bash scripts/package_mac.sh
set -euo pipefail
cd "$(dirname "$0")/.."

if [[ "$(uname)" != "Darwin" ]]; then
    echo "This script packages the macOS app, so it must be run on a Mac."
    exit 1
fi
command -v cmake >/dev/null 2>&1 || {
    echo "CMake was not found. Install it first:  brew install cmake"
    exit 1
}

echo "==> Configuring and building (Release)"
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release -j 4

echo "==> Running the automated tests"
ctest --test-dir build -C Release --output-on-failure

APP="build/NazalEditor_artefacts/Release/Nazal Audio Editor.app"
if [[ ! -d "$APP" ]]; then
    echo "The build finished but '$APP' was not found."
    exit 1
fi

OUT="output/Team-Nazal-Deliverables"
rm -rf "$OUT" output/Team-Nazal-Final-Deliverables.zip output/Nazal-macOS.zip
mkdir -p "$OUT/Desktop Application" "$OUT/Source Code" "$OUT/Third Party Notices"

echo "==> Copying the app (ditto keeps the code signature intact)"
ditto "$APP" "$OUT/Desktop Application/Nazal Audio Editor.app"
codesign --verify --deep --strict "$OUT/Desktop Application/Nazal Audio Editor.app"

echo "==> Copying the source"
rsync -a --exclude 'build*' --exclude output --exclude tmp --exclude test-output \
    --exclude .git --exclude .DS_Store --exclude AGENTS.md ./ "$OUT/Source Code/"

JUCE_LICENSE="build/_deps/juce-src/LICENSE.md"
if [[ ! -f "$JUCE_LICENSE" && -n "${JUCE_DIR:-}" ]]; then
    JUCE_LICENSE="$JUCE_DIR/LICENSE.md"
fi
if [[ -f "$JUCE_LICENSE" ]]; then
    cp "$JUCE_LICENSE" "$OUT/Third Party Notices/JUCE-LICENSE.md"
else
    echo "Warning: JUCE LICENSE.md not found; copy it into 'Third Party Notices' by hand."
fi

case "$(uname -m)" in
arm64) CPU="Apple Silicon" ;;
*) CPU="Intel" ;;
esac
cat > "$OUT/README.txt" <<README
TEAM NAZAL - FINAL PROJECT DELIVERABLES
Interactive Multi-Track Audio Editor and Mixer

Folders
1. Source Code/
   Complete modular C++17/JUCE application source, CMake configuration,
   tests, documentation, original assignments and demo project/audio.
   Modules: src/model, src/core, src/algorithms, src/system, src/ui.
   See Source Code/README.md for setup, build, architecture and limitations.
   CMake fetches the pinned JUCE 8.0.12 dependency on first configuration.

2. Desktop Application/
   Nazal Audio Editor.app - compiled Release application for ${CPU} macOS
   (built on macOS $(sw_vers -productVersion)).
   Extract the ZIP, then open this application on a compatible Mac. If macOS
   blocks it, right-click the app, choose Open, and confirm.
   In the app, choose Project > Open Project and select
   Source Code/examples/Demo.nazal to explore the included three-track composition.

3. Third Party Notices/
   JUCE license supplied with the framework used to build this application.

Validation
Release build succeeded and the automated test suite passed (see
Source Code/docs/Testing.md for coverage, measurements and the manual
acceptance record). The application is locally signed (ad hoc), not notarized.
Windows and Linux executables are not included; source is provided for building.
README

echo "==> Creating ZIP files"
(cd output && ditto -c -k --keepParent Team-Nazal-Deliverables Team-Nazal-Final-Deliverables.zip)
ditto -c -k --keepParent "$APP" output/Nazal-macOS.zip

echo
echo "Done. Submission files:"
echo "  $(pwd)/output/Team-Nazal-Final-Deliverables.zip"
echo "  $(pwd)/output/Nazal-macOS.zip"
