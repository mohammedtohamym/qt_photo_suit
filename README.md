# PhotoSuite (Qt)

A larger Qt Widgets photo software suite foundation inspired by Google Photos-style workflows.

## Implemented Workspaces

- **Library / Organize**
	- Thumbnail browsing (grid/list)
	- Name/tag/favorite/rating filters
	- Sorting and adjustable thumbnail sizes
	- Metadata editing (tags/favorite/rating)
	- Bulk actions via compact menu
- **Albums**
	- Create/delete albums
	- Add selected photos to albums
	- Remove selected photos from albums
	- Album data persisted in `.photo_organizer.json`
- **Files**
	- Folder file explorer with image filtering
	- Open files directly from tree view
- **Editor (Mini Photoshop baseline)**
	- Brightness and contrast sliders
	- Grayscale toggle
	- Rotate left / rotate right
	- Save edited copy

## Product Planning

See the complete large-project feature backlog in [PRODUCT_SUITE_FEATURES.md](PRODUCT_SUITE_FEATURES.md).

## Requirements

- Qt 6 (Widgets)
- CMake 3.16+
- A C++17 compiler

## Build (Windows, PowerShell)

```powershell
cmake -S . -B build -G "Visual Studio 16 2019" -A x64 -DCMAKE_PREFIX_PATH="./deps/Qt/6.5.3/msvc2019_64" -DCMAKE_DISABLE_FIND_PACKAGE_WrapVulkanHeaders=TRUE
cmake --build build --config Release
```

If you are using a different Qt installation, change `CMAKE_PREFIX_PATH` accordingly.

## Run

```powershell
./build/Release/PhotoOrganizerQt.exe
```

## Usage

1. Open the app.
2. Go to **File -> Open Photo Folder...**
3. Select a folder that contains photos.
4. Use top filters to find photos quickly.
5. Use **Organize** tab to edit metadata and run bulk actions.
6. Use **Albums** tab to group selected photos.
7. Use **Files** tab to navigate source files.
8. Use **Editor** tab to preview edits and save edited copies.

## Notes

- Supported image extensions include: `jpg`, `jpeg`, `png`, `bmp`, `gif`, `webp`, `tiff`, `tif`, `heic`.
- Metadata and albums are stored by relative paths in `.photo_organizer.json`, so moving the whole folder preserves organization.
