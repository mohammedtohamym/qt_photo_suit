#pragma once

#include "photolibrary.h"

#include <QCheckBox>
#include <QComboBox>
#include <QImage>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMainWindow>
#include <QPushButton>
#include <QSlider>
#include <QSpinBox>

class QMenu;
class QTimer;
class QToolButton;
class QAction;
class QFileSystemModel;
class QTabWidget;
class QTreeView;
class QSplitter;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow();

private:
    void setupUi();
    void setupMenu();
    void setupToolbar();
    void setupConnections();
    void updateRecentFoldersMenu();
    void setupSuiteTabs(QSplitter *splitter);
    void refreshAlbumsWorkspace();
    void refreshFilesWorkspace();
    void refreshTimelineWorkspace();
    void loadEditorPhoto(const QString &path);
    void applyEditorAdjustments();
    void updateEditorPreview();
    QImage makeEditedImage() const;

    void openFolder();
    void openFolderPath(const QString &folderPath);
    void refreshList();
    void exportVisibleToCsv();
    void loadSelectionDetails();
    void clearDetails();
    void setUnsavedChanges(bool value);

    QString currentPhotoPath() const;
    QStringList selectedPhotoPaths() const;
    QStringList parseTags(const QString &tagText) const;

private:
    PhotoLibrary m_library;

    QLineEdit *m_nameFilter = nullptr;
    QLineEdit *m_tagFilter = nullptr;
    QComboBox *m_savedSearchCombo = nullptr;
    QPushButton *m_saveSearchButton = nullptr;
    QPushButton *m_deleteSearchButton = nullptr;
    QCheckBox *m_favoritesOnly = nullptr;
    QComboBox *m_sortCombo = nullptr;
    QComboBox *m_viewModeCombo = nullptr;
    QSlider *m_thumbSizeSlider = nullptr;
    QSpinBox *m_minRatingFilter = nullptr;

    QListWidget *m_photoList = nullptr;
    QTabWidget *m_suiteTabs = nullptr;

    QWidget *m_organizeTab = nullptr;
    QLabel *m_previewLabel = nullptr;
    QLabel *m_pathLabel = nullptr;
    QLineEdit *m_tagsEdit = nullptr;
    QLineEdit *m_bulkTagsEdit = nullptr;
    QCheckBox *m_autoSaveCheck = nullptr;
    QCheckBox *m_favoriteCheck = nullptr;
    QSpinBox *m_ratingSpin = nullptr;
    QPushButton *m_saveButton = nullptr;
    QPushButton *m_openPhotoButton = nullptr;
    QPushButton *m_openLocationButton = nullptr;
    QPushButton *m_copyPathButton = nullptr;
    QPushButton *m_slideshowButton = nullptr;
    QToolButton *m_bulkActionsButton = nullptr;
    QAction *m_bulkAddTagsAction = nullptr;
    QAction *m_bulkFavoriteAction = nullptr;
    QAction *m_bulkRatingAction = nullptr;

    QWidget *m_albumsTab = nullptr;
    QListWidget *m_albumList = nullptr;
    QListWidget *m_albumPhotoList = nullptr;
    QLineEdit *m_newAlbumEdit = nullptr;
    QPushButton *m_createAlbumButton = nullptr;
    QPushButton *m_deleteAlbumButton = nullptr;
    QPushButton *m_addToAlbumButton = nullptr;
    QPushButton *m_removeFromAlbumButton = nullptr;

    QWidget *m_filesTab = nullptr;
    QTreeView *m_filesTree = nullptr;
    QFileSystemModel *m_fileModel = nullptr;

    QWidget *m_timelineTab = nullptr;
    QListWidget *m_timelineList = nullptr;

    QWidget *m_editorTab = nullptr;
    QLabel *m_editorPreviewLabel = nullptr;
    QSlider *m_brightnessSlider = nullptr;
    QSlider *m_contrastSlider = nullptr;
    QCheckBox *m_grayscaleCheck = nullptr;
    QPushButton *m_editorRotateLeftButton = nullptr;
    QPushButton *m_editorRotateRightButton = nullptr;
    QPushButton *m_editorResetButton = nullptr;
    QPushButton *m_editorSaveCopyButton = nullptr;
    QImage m_editorOriginalImage;
    QImage m_editorPreviewImage;

    QMenu *m_recentFoldersMenu = nullptr;
    QStringList m_recentFolders;
    bool m_hasUnsavedChanges = false;
    bool m_isLoadingSelection = false;
    QTimer *m_slideshowTimer = nullptr;
};
