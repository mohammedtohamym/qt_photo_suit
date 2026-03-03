# Photo Organizer (Qt)

A lightweight Qt Widgets desktop app to organize photos by folder with:

- Thumbnail grid browsing
- Name and tag filtering
- Favorites flag
- 0-5 rating
- Custom tags per photo
- Metadata persistence in `.photo_organizer.json` inside the selected folder

## Requirements

- Qt 6 (Widgets)
- CMake 3.16+
- A C++17 compiler

## Build (Windows, PowerShell)

```powershell
cmake -S . -B build -G "Ninja" -DCMAKE_PREFIX_PATH="C:/Qt/6.8.0/msvc2022_64"
cmake --build build
```

If you are using a different Qt installation, change `CMAKE_PREFIX_PATH` accordingly.

## Run

```powershell
./build/PhotoOrganizerQt.exe
```

## Usage

1. Open the app.
2. Go to **File -> Open Photo Folder...**
3. Select a folder that contains photos.
4. Use filters at the top to find photos quickly.
5. Select a photo, edit tags/favorite/rating, then click **Save metadata**.
6. Double-click a thumbnail to open the photo using your system default image viewer.

## Notes

- Supported image extensions include: `jpg`, `jpeg`, `png`, `bmp`, `gif`, `webp`, `tiff`, `tif`, `heic`.
- Metadata is stored by relative photo path, so moving the whole folder preserves your organization.
