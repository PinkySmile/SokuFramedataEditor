BUILD_DIR="$1"
OUTPUT="$2"

if [ -z "$BUILD_DIR" ]; then
	BUILD_DIR=cmake-build-release
fi
if [ -z "$OUTPUT" ]; then
	OUTPUT=appimage_folder
fi

cmake --build "$BUILD_DIR" --target FrameDataEditor -- -j 14
mkdir -p "$OUTPUT/usr/bin" "$OUTPUT/usr/lib" "$OUTPUT/usr/share/th123fde" "$OUTPUT/usr/share/applications" "$OUTPUT/usr/share/icons/hicolor/64x64/apps"
cp "$BUILD_DIR/FrameDataEditor" "$OUTPUT/usr/bin"
cp -r assets "$OUTPUT/usr/share/th123fde"
cp assets/editorIcon.png "$OUTPUT/usr/share/icons/hicolor/64x64/apps/th123fde.png"
echo "[Desktop Entry]
Type=Application
Name=Touhou 12.3 - FrameData Editor
Exec=FrameDataEditor
Comment=Framedata editor for the 12.3th Touhou game
Icon=th123fde
Categories=Development" > "$OUTPUT/usr/share/applications/th123fde.desktop"
echo '#!/bin/sh
HERE="$(dirname -- "$(readlink -f -- "$0")")"
export LD_LIBRARY_PATH="$LD_LIBRARY_PATH:${HERE}/usr/lib"
cd "$HERE"/usr/share/th123fde
exec "$HERE"/usr/bin/FrameDataEditor "$@"' > "$OUTPUT/AppRun"
chmod +x "$OUTPUT/AppRun"
ln -s usr/share/icons/hicolor/64x64/apps/th123fde.png "$OUTPUT/.DirIcon"
ln -s usr/share/icons/hicolor/64x64/apps/th123fde.png "$OUTPUT/th123fde.png"
ln -s usr/share/applications/th123fde.desktop "$OUTPUT/th123fde.desktop"
ldd "$OUTPUT/usr/bin/FrameDataEditor" | tail -n +2 | sort | tail -n +2 | cut -d '>' -f 2 | cut -d '(' -f 1 | cut -d ' ' -f 2 | while read line; do
	if printf %s "$line" | grep -v "libc.so" | grep -v "libstdc++.so" | grep -v "libcrypto.so" | grep -v "libm.so"; then
		cat "$line" > "$OUTPUT/usr/lib/$(basename "$line")"
	fi
done
ARCH=x86_64 appimagetool appimage_folder/ th123fde.AppImage
rm -rf "$OUTPUT"