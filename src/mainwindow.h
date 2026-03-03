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
#include <QVector>
#include <QMap>

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
    QLineEdit *m_smartRuleNameEdit = nullptr;
    QLineEdit *m_smartRuleTagEdit = nullptr;
    QSpinBox *m_smartRuleMinRatingSpin = nullptr;
    QCheckBox *m_smartRuleFavoriteOnlyCheck = nullptr;
    QPushButton *m_createSmartRuleButton = nullptr;
    QPushButton *m_deleteSmartRuleButton = nullptr;

    QWidget *m_filesTab = nullptr;
    QTreeView *m_filesTree = nullptr;
    QFileSystemModel *m_fileModel = nullptr;
    QPushButton *m_renameFileButton = nullptr;
    QLineEdit *m_bulkRenamePatternEdit = nullptr;
    QPushButton *m_bulkRenameButton = nullptr;
    QPushButton *m_scanDuplicatesButton = nullptr;
    QPushButton *m_openContainingFolderButton = nullptr;
    QPushButton *m_deleteDuplicateEntryButton = nullptr;
    QListWidget *m_duplicatesList = nullptr;

    QWidget *m_timelineTab = nullptr;
    QListWidget *m_timelineList = nullptr;
    QComboBox *m_timelineYearFilter = nullptr;

    QWidget *m_editorTab = nullptr;
    QLabel *m_editorPreviewLabel = nullptr;
    QSlider *m_brightnessSlider = nullptr;
    QSlider *m_contrastSlider = nullptr;
    QSlider *m_saturationSlider = nullptr;
    QSlider *m_temperatureSlider = nullptr;
    QSlider *m_vignetteSlider = nullptr;
    QCheckBox *m_grayscaleCheck = nullptr;
    QCheckBox *m_sepiaCheck = nullptr;
    QPushButton *m_editorRotateLeftButton = nullptr;
    QPushButton *m_editorRotateRightButton = nullptr;
    QPushButton *m_editorUndoButton = nullptr;
    QPushButton *m_editorRedoButton = nullptr;
    QPushButton *m_editorAutoEnhanceButton = nullptr;
    QPushButton *m_editorBeforeAfterButton = nullptr;
    QPushButton *m_editorResetButton = nullptr;
    QPushButton *m_editorSaveCopyButton = nullptr;
    QPushButton *m_editorSaveSnapshotButton = nullptr;
    QListWidget *m_editorSnapshotsList = nullptr;
    QImage m_editorOriginalImage;
    QImage m_editorPreviewImage;
    QVector<QImage> m_editorUndoStack;
    QVector<QImage> m_editorRedoStack;
    QMap<QString, QImage> m_editorSnapshots;

    QMenu *m_recentFoldersMenu = nullptr;
    QStringList m_recentFolders;
    bool m_hasUnsavedChanges = false;
    bool m_isLoadingSelection = false;
    bool m_editorShowOriginal = false;
    QTimer *m_slideshowTimer = nullptr;
};
