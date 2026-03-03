#include "mainwindow.h"

#include <QAction>
#include <QApplication>
#include <QClipboard>
#include <QColor>
#include <QDesktopServices>
#include <QDir>
#include <QDateTime>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFileSystemModel>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QListWidgetItem>
#include <QMap>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QPixmap>
#include <QRandomGenerator>
#include <QSettings>
#include <QShortcut>
#include <QSplitter>
#include <QStandardPaths>
#include <QStatusBar>
#include <QStyle>
#include <QTabWidget>
#include <QTextStream>
#include <QTimer>
#include <QToolBar>
#include <QToolButton>
#include <QTreeView>
#include <QTransform>
#include <QUrl>
#include <QVBoxLayout>

#include <QHash>

#include <algorithm>
#include <cmath>

MainWindow::MainWindow()
{
    setupUi();
    setupMenu();
    setupToolbar();
    setupConnections();
    statusBar()->showMessage("Open a folder to start building your photo universe.");

    const QString lastFolder = QSettings("PhotoOrganizerQt", "PhotoOrganizerQt").value("lastFolder").toString();
    if (!lastFolder.isEmpty() && QFileInfo(lastFolder).isDir()) {
        openFolderPath(lastFolder);
    }
}

void MainWindow::setupUi()
{
    setWindowTitle("PhotoSuite (Qt)[*]");
    resize(1440, 860);

    setStyleSheet(
        "QMainWindow {"
        "  background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #0a1020, stop:1 #111827);"
        "}"
        "QWidget { color: #e6edf6; font-size: 13px; background: transparent; }"
        "QWidget#FilterCard, QWidget#DetailsCard, QWidget#AlbumsCard, QWidget#FilesCard, QWidget#EditorCard {"
        "  background: rgba(15, 23, 42, 0.92);"
        "  border: 1px solid #23324a;"
        "  border-radius: 14px;"
        "}"
        "QLabel { color: #c9d6ea; }"
        "QTabWidget::pane { border: 1px solid #23324a; border-radius: 12px; background: rgba(15, 23, 42, 0.85); }"
        "QTabBar::tab {"
        "  background: rgba(17, 24, 39, 0.8);"
        "  border: 1px solid #23324a;"
        "  border-top-left-radius: 10px;"
        "  border-top-right-radius: 10px;"
        "  padding: 8px 12px;"
        "  margin-right: 4px;"
        "  color: #9fb0ca;"
        "}"
        "QTabBar::tab:selected { background: #1d4ed8; color: #ffffff; border-color: #3b82f6; }"
        "QLineEdit, QSpinBox, QComboBox, QListWidget, QTreeView {"
        "  background: rgba(17, 24, 39, 0.94);"
        "  border: 1px solid #2f3f5c;"
        "  border-radius: 10px;"
        "  padding: 7px;"
        "  color: #f8fbff;"
        "}"
        "QLineEdit:focus, QSpinBox:focus, QComboBox:focus { border: 1px solid #60a5fa; }"
        "QPushButton {"
        "  background: #1d4ed8; border: none; border-radius: 10px;"
        "  padding: 8px 12px; color: #ffffff; font-weight: 600;"
        "}"
        "QPushButton:hover { background: #2563eb; }"
        "QPushButton:disabled { background: #334155; color: #94a3b8; }"
        "QToolButton {"
        "  background: rgba(17, 24, 39, 0.85); border: 1px solid #2f3f5c;"
        "  border-radius: 10px; padding: 6px 10px; color: #e2e8f0;"
        "}"
        "QToolButton:hover { background: rgba(30, 41, 59, 0.92); }"
        "QToolButton::menu-indicator { image: none; width: 0px; }"
        "QCheckBox::indicator { width: 16px; height: 16px; }"
        "QCheckBox::indicator:unchecked { border: 1px solid #3b4c6a; background: #111827; border-radius: 4px; }"
        "QCheckBox::indicator:checked { border: 1px solid #60a5fa; background: #1d4ed8; border-radius: 4px; }"
        "QListWidget::item { background: transparent; border: 1px solid transparent; border-radius: 8px; padding: 4px; }"
        "QListWidget::item:hover, QTreeView::item:hover { background: rgba(59, 130, 246, 0.16); }"
        "QListWidget::item:selected, QTreeView::item:selected {"
        "  background: rgba(37, 99, 235, 0.28); border-color: #3b82f6; color: #f8fbff;"
        "}"
        "QToolBar { background: rgba(15, 23, 42, 0.9); border: 1px solid #23324a; border-radius: 10px; spacing: 4px; }"
        "QToolBar::separator { width: 1px; background: #23324a; margin: 4px; }"
        "QSplitter::handle { background: #23324a; margin: 0 3px; border-radius: 2px; }"
        "QMenuBar, QMenu { background: #0f172a; color: #f8fafc; }"
        "QMenu::item:selected { background: #1e3a8a; }"
        "QStatusBar { background: #0b1220; color: #94a3b8; }"
    );

    auto *central = new QWidget(this);
    auto *rootLayout = new QVBoxLayout(central);
    rootLayout->setContentsMargins(18, 18, 18, 18);
    rootLayout->setSpacing(12);

    auto *filterCard = new QWidget(this);
    filterCard->setObjectName("FilterCard");
    auto *filterCardLayout = new QVBoxLayout(filterCard);
    filterCardLayout->setContentsMargins(12, 12, 12, 12);
    filterCardLayout->setSpacing(8);

    auto *searchRow = new QHBoxLayout();
    searchRow->setSpacing(8);

    m_nameFilter = new QLineEdit(this);
    m_nameFilter->setPlaceholderText("Search photos by name...");

    m_tagFilter = new QLineEdit(this);
    m_tagFilter->setPlaceholderText("Filter by tag...");

    m_savedSearchCombo = new QComboBox(this);
    m_savedSearchCombo->setMinimumWidth(200);
    m_savedSearchCombo->addItem("Saved searches");
    m_saveSearchButton = new QPushButton("Save Search", this);
    m_deleteSearchButton = new QPushButton("Delete", this);
    m_deleteSearchButton->setMaximumWidth(90);

    m_favoritesOnly = new QCheckBox("Favorites only", this);
    searchRow->addWidget(m_nameFilter, 3);
    searchRow->addWidget(m_tagFilter, 2);
    searchRow->addWidget(m_savedSearchCombo);
    searchRow->addWidget(m_saveSearchButton);
    searchRow->addWidget(m_deleteSearchButton);
    searchRow->addWidget(m_favoritesOnly);

    auto *controlsRow = new QHBoxLayout();
    controlsRow->setSpacing(8);

    m_sortCombo = new QComboBox(this);
    m_sortCombo->addItems({"Name (A-Z)", "Name (Z-A)", "Rating (High-Low)", "Path"});
    m_viewModeCombo = new QComboBox(this);
    m_viewModeCombo->addItems({"Grid", "List"});

    m_minRatingFilter = new QSpinBox(this);
    m_minRatingFilter->setRange(0, 5);
    m_minRatingFilter->setPrefix("Min ");

    m_thumbSizeSlider = new QSlider(Qt::Horizontal, this);
    m_thumbSizeSlider->setRange(80, 240);
    m_thumbSizeSlider->setValue(150);
    m_thumbSizeSlider->setMaximumWidth(180);

    controlsRow->addWidget(new QLabel("View", this));
    controlsRow->addWidget(m_viewModeCombo);
    controlsRow->addWidget(new QLabel("Sort", this));
    controlsRow->addWidget(m_sortCombo, 2);
    controlsRow->addWidget(new QLabel("Min rating", this));
    controlsRow->addWidget(m_minRatingFilter);
    controlsRow->addWidget(new QLabel("Thumb", this));
    controlsRow->addWidget(m_thumbSizeSlider);
    controlsRow->addStretch(1);

    filterCardLayout->addLayout(searchRow);
    filterCardLayout->addLayout(controlsRow);

    auto *splitter = new QSplitter(Qt::Horizontal, this);

    m_photoList = new QListWidget(this);
    m_photoList->setViewMode(QListView::IconMode);
    m_photoList->setIconSize(QSize(150, 150));
    m_photoList->setResizeMode(QListWidget::Adjust);
    m_photoList->setMovement(QListView::Static);
    m_photoList->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_photoList->setSpacing(10);
    m_photoList->setWordWrap(true);

    splitter->addWidget(m_photoList);
    setupSuiteTabs(splitter);
    splitter->setStretchFactor(0, 3);
    splitter->setStretchFactor(1, 2);

    rootLayout->addWidget(filterCard);
    rootLayout->addWidget(splitter, 1);
    setCentralWidget(central);

    clearDetails();
}

void MainWindow::setupSuiteTabs(QSplitter *splitter)
{
    m_suiteTabs = new QTabWidget(this);

    m_organizeTab = new QWidget(this);
    m_organizeTab->setObjectName("DetailsCard");
    auto *detailsLayout = new QVBoxLayout(m_organizeTab);
    detailsLayout->setContentsMargins(12, 12, 12, 12);
    detailsLayout->setSpacing(10);

    m_previewLabel = new QLabel(this);
    m_previewLabel->setMinimumSize(420, 300);
    m_previewLabel->setAlignment(Qt::AlignCenter);
    m_previewLabel->setText("No photo selected");
    m_previewLabel->setStyleSheet("background: rgba(15,23,42,0.7); border: 1px solid #2f3f5c; border-radius: 12px;");

    m_pathLabel = new QLabel(this);
    m_pathLabel->setWordWrap(true);
    m_pathLabel->setStyleSheet("color: #93a4bf;");

    m_tagsEdit = new QLineEdit(this);
    m_tagsEdit->setPlaceholderText("comma,separated,tags");

    m_bulkTagsEdit = new QLineEdit(this);
    m_bulkTagsEdit->setPlaceholderText("bulk tags to add to selected photos");

    m_autoSaveCheck = new QCheckBox("Auto-save metadata", this);
    m_autoSaveCheck->setChecked(QSettings("PhotoOrganizerQt", "PhotoOrganizerQt").value("autoSave", false).toBool());

    m_favoriteCheck = new QCheckBox("Favorite", this);

    m_ratingSpin = new QSpinBox(this);
    m_ratingSpin->setRange(0, 5);
    m_ratingSpin->setPrefix("Rating: ");

    m_saveButton = new QPushButton("Save Changes", this);
    m_openPhotoButton = new QPushButton("Open photo", this);
    m_openLocationButton = new QPushButton("Open location", this);
    m_copyPathButton = new QPushButton("Copy path", this);
    m_slideshowButton = new QPushButton("Slideshow", this);

    m_bulkActionsButton = new QToolButton(this);
    m_bulkActionsButton->setText("Bulk Actions");
    m_bulkActionsButton->setPopupMode(QToolButton::InstantPopup);
    auto *bulkMenu = new QMenu(m_bulkActionsButton);
    m_bulkAddTagsAction = bulkMenu->addAction("Add tags to selected");
    m_bulkFavoriteAction = bulkMenu->addAction("Apply favorite to selected");
    m_bulkRatingAction = bulkMenu->addAction("Apply rating to selected");
    m_bulkActionsButton->setMenu(bulkMenu);

    m_slideshowTimer = new QTimer(this);
    m_slideshowTimer->setInterval(2000);

    auto *quickActionsRow = new QHBoxLayout();
    quickActionsRow->setSpacing(8);
    quickActionsRow->addWidget(m_openPhotoButton);
    quickActionsRow->addWidget(m_openLocationButton);
    quickActionsRow->addWidget(m_copyPathButton);
    quickActionsRow->addWidget(m_slideshowButton);

    auto *saveRow = new QHBoxLayout();
    saveRow->setSpacing(8);
    saveRow->addWidget(m_bulkActionsButton);
    saveRow->addStretch(1);
    saveRow->addWidget(m_saveButton);

    detailsLayout->addWidget(m_previewLabel, 1);
    detailsLayout->addWidget(new QLabel("Path:", this));
    detailsLayout->addWidget(m_pathLabel);
    detailsLayout->addWidget(new QLabel("Tags:", this));
    detailsLayout->addWidget(m_tagsEdit);
    detailsLayout->addWidget(m_autoSaveCheck);
    detailsLayout->addWidget(new QLabel("Bulk Tags:", this));
    detailsLayout->addWidget(m_bulkTagsEdit);
    detailsLayout->addWidget(m_favoriteCheck);
    detailsLayout->addWidget(m_ratingSpin);
    detailsLayout->addLayout(quickActionsRow);
    detailsLayout->addLayout(saveRow);
    detailsLayout->addStretch();

    m_albumsTab = new QWidget(this);
    m_albumsTab->setObjectName("AlbumsCard");
    auto *albumsLayout = new QVBoxLayout(m_albumsTab);
    albumsLayout->setContentsMargins(12, 12, 12, 12);
    albumsLayout->setSpacing(10);

    auto *newAlbumRow = new QHBoxLayout();
    m_newAlbumEdit = new QLineEdit(this);
    m_newAlbumEdit->setPlaceholderText("New album name");
    m_createAlbumButton = new QPushButton("Create", this);
    m_deleteAlbumButton = new QPushButton("Delete", this);
    newAlbumRow->addWidget(m_newAlbumEdit, 1);
    newAlbumRow->addWidget(m_createAlbumButton);
    newAlbumRow->addWidget(m_deleteAlbumButton);

    auto *smartRuleRow = new QHBoxLayout();
    m_smartRuleNameEdit = new QLineEdit(this);
    m_smartRuleNameEdit->setPlaceholderText("Smart rule name");
    m_smartRuleTagEdit = new QLineEdit(this);
    m_smartRuleTagEdit->setPlaceholderText("Tag contains (optional)");
    m_smartRuleMinRatingSpin = new QSpinBox(this);
    m_smartRuleMinRatingSpin->setRange(0, 5);
    m_smartRuleMinRatingSpin->setPrefix("Min ");
    m_smartRuleFavoriteOnlyCheck = new QCheckBox("Fav only", this);
    m_createSmartRuleButton = new QPushButton("Create Smart Rule", this);
    m_deleteSmartRuleButton = new QPushButton("Delete Smart Rule", this);
    smartRuleRow->addWidget(m_smartRuleNameEdit, 1);
    smartRuleRow->addWidget(m_smartRuleTagEdit, 1);
    smartRuleRow->addWidget(m_smartRuleMinRatingSpin);
    smartRuleRow->addWidget(m_smartRuleFavoriteOnlyCheck);
    smartRuleRow->addWidget(m_createSmartRuleButton);
    smartRuleRow->addWidget(m_deleteSmartRuleButton);

    m_albumList = new QListWidget(this);
    m_albumPhotoList = new QListWidget(this);
    m_albumPhotoList->setSelectionMode(QAbstractItemView::ExtendedSelection);

    auto *albumActionsRow = new QHBoxLayout();
    m_addToAlbumButton = new QPushButton("Add selected photos", this);
    m_removeFromAlbumButton = new QPushButton("Remove selected photos", this);
    albumActionsRow->addWidget(m_addToAlbumButton);
    albumActionsRow->addWidget(m_removeFromAlbumButton);

    albumsLayout->addLayout(newAlbumRow);
    albumsLayout->addLayout(smartRuleRow);
    albumsLayout->addWidget(new QLabel("Albums", this));
    albumsLayout->addWidget(m_albumList, 1);
    albumsLayout->addLayout(albumActionsRow);
    albumsLayout->addWidget(new QLabel("Album Photos", this));
    albumsLayout->addWidget(m_albumPhotoList, 1);

    m_filesTab = new QWidget(this);
    m_filesTab->setObjectName("FilesCard");
    auto *filesLayout = new QVBoxLayout(m_filesTab);
    filesLayout->setContentsMargins(12, 12, 12, 12);
    filesLayout->setSpacing(10);

    m_fileModel = new QFileSystemModel(this);
    m_fileModel->setFilter(QDir::AllDirs | QDir::Files | QDir::NoDotAndDotDot);
    m_fileModel->setNameFilters({"*.jpg", "*.jpeg", "*.png", "*.bmp", "*.gif", "*.webp", "*.tif", "*.tiff", "*.heic"});
    m_fileModel->setNameFilterDisables(false);

    m_filesTree = new QTreeView(this);
    m_filesTree->setModel(m_fileModel);
    m_filesTree->setSortingEnabled(true);
    m_filesTree->sortByColumn(0, Qt::AscendingOrder);
    m_renameFileButton = new QPushButton("Rename selected file", this);
    m_scanDuplicatesButton = new QPushButton("Scan duplicates", this);
    m_openContainingFolderButton = new QPushButton("Open containing folder", this);
    m_duplicatesList = new QListWidget(this);
    m_duplicatesList->setMinimumHeight(140);
    m_duplicatesList->setSelectionMode(QAbstractItemView::SingleSelection);

    filesLayout->addWidget(new QLabel("File Explorer", this));
    filesLayout->addWidget(m_filesTree, 1);
    filesLayout->addWidget(m_renameFileButton);
    filesLayout->addWidget(m_openContainingFolderButton);
    filesLayout->addWidget(m_scanDuplicatesButton);
    filesLayout->addWidget(new QLabel("Likely duplicates", this));
    filesLayout->addWidget(m_duplicatesList);

    m_timelineTab = new QWidget(this);
    m_timelineTab->setObjectName("FilesCard");
    auto *timelineLayout = new QVBoxLayout(m_timelineTab);
    timelineLayout->setContentsMargins(12, 12, 12, 12);
    timelineLayout->setSpacing(10);
    m_timelineYearFilter = new QComboBox(this);
    m_timelineYearFilter->addItem("All years", QString());
    m_timelineList = new QListWidget(this);
    m_timelineList->setSelectionMode(QAbstractItemView::SingleSelection);
    timelineLayout->addWidget(new QLabel("Timeline", this));
    timelineLayout->addWidget(m_timelineYearFilter);
    timelineLayout->addWidget(m_timelineList, 1);

    m_editorTab = new QWidget(this);
    m_editorTab->setObjectName("EditorCard");
    auto *editorLayout = new QVBoxLayout(m_editorTab);
    editorLayout->setContentsMargins(12, 12, 12, 12);
    editorLayout->setSpacing(10);

    m_editorPreviewLabel = new QLabel(this);
    m_editorPreviewLabel->setMinimumSize(420, 300);
    m_editorPreviewLabel->setAlignment(Qt::AlignCenter);
    m_editorPreviewLabel->setText("Editor preview (select a photo)");
    m_editorPreviewLabel->setStyleSheet("background: rgba(15,23,42,0.7); border: 1px solid #2f3f5c; border-radius: 12px;");

    m_brightnessSlider = new QSlider(Qt::Horizontal, this);
    m_brightnessSlider->setRange(-100, 100);
    m_brightnessSlider->setValue(0);

    m_contrastSlider = new QSlider(Qt::Horizontal, this);
    m_contrastSlider->setRange(-100, 100);
    m_contrastSlider->setValue(0);

    m_saturationSlider = new QSlider(Qt::Horizontal, this);
    m_saturationSlider->setRange(-100, 100);
    m_saturationSlider->setValue(0);

    m_temperatureSlider = new QSlider(Qt::Horizontal, this);
    m_temperatureSlider->setRange(-100, 100);
    m_temperatureSlider->setValue(0);

    m_vignetteSlider = new QSlider(Qt::Horizontal, this);
    m_vignetteSlider->setRange(0, 100);
    m_vignetteSlider->setValue(0);

    m_grayscaleCheck = new QCheckBox("Grayscale", this);
    m_sepiaCheck = new QCheckBox("Sepia", this);
    m_editorRotateLeftButton = new QPushButton("Rotate Left", this);
    m_editorRotateRightButton = new QPushButton("Rotate Right", this);
    m_editorUndoButton = new QPushButton("Undo", this);
    m_editorRedoButton = new QPushButton("Redo", this);
    m_editorAutoEnhanceButton = new QPushButton("Auto Enhance", this);
    m_editorBeforeAfterButton = new QPushButton("Before/After", this);
    m_editorBeforeAfterButton->setCheckable(true);
    m_editorResetButton = new QPushButton("Reset", this);
    m_editorSaveCopyButton = new QPushButton("Save edited copy", this);
    m_editorSaveSnapshotButton = new QPushButton("Save Snapshot", this);
    m_editorSnapshotsList = new QListWidget(this);
    m_editorSnapshotsList->setMinimumHeight(120);

    auto *editorActions = new QHBoxLayout();
    editorActions->addWidget(m_editorRotateLeftButton);
    editorActions->addWidget(m_editorRotateRightButton);
    editorActions->addWidget(m_editorUndoButton);
    editorActions->addWidget(m_editorRedoButton);
    editorActions->addWidget(m_editorAutoEnhanceButton);
    editorActions->addWidget(m_editorBeforeAfterButton);
    editorActions->addWidget(m_editorResetButton);
    editorActions->addStretch(1);
    editorActions->addWidget(m_editorSaveSnapshotButton);
    editorActions->addWidget(m_editorSaveCopyButton);

    editorLayout->addWidget(m_editorPreviewLabel, 1);
    editorLayout->addWidget(new QLabel("Brightness", this));
    editorLayout->addWidget(m_brightnessSlider);
    editorLayout->addWidget(new QLabel("Contrast", this));
    editorLayout->addWidget(m_contrastSlider);
    editorLayout->addWidget(new QLabel("Saturation", this));
    editorLayout->addWidget(m_saturationSlider);
    editorLayout->addWidget(new QLabel("Temperature", this));
    editorLayout->addWidget(m_temperatureSlider);
    editorLayout->addWidget(new QLabel("Vignette", this));
    editorLayout->addWidget(m_vignetteSlider);
    editorLayout->addWidget(m_grayscaleCheck);
    editorLayout->addWidget(m_sepiaCheck);
    editorLayout->addLayout(editorActions);
    editorLayout->addWidget(new QLabel("Snapshots", this));
    editorLayout->addWidget(m_editorSnapshotsList);

    m_suiteTabs->addTab(m_organizeTab, "Organize");
    m_suiteTabs->addTab(m_albumsTab, "Albums");
    m_suiteTabs->addTab(m_filesTab, "Files");
    m_suiteTabs->addTab(m_timelineTab, "Timeline");
    m_suiteTabs->addTab(m_editorTab, "Editor");

    splitter->addWidget(m_suiteTabs);
}

void MainWindow::setupMenu()
{
    auto *fileMenu = menuBar()->addMenu("&File");

    auto *openAction = new QAction("Open Photo Folder...", this);
    openAction->setShortcut(QKeySequence::Open);
    connect(openAction, &QAction::triggered, this, &MainWindow::openFolder);

    auto *quitAction = new QAction("Quit", this);
    quitAction->setShortcut(QKeySequence::Quit);
    connect(quitAction, &QAction::triggered, this, &QWidget::close);

    fileMenu->addAction(openAction);
    m_recentFoldersMenu = fileMenu->addMenu("Recent Folders");
    m_recentFolders = QSettings("PhotoOrganizerQt", "PhotoOrganizerQt").value("recentFolders").toStringList();
    updateRecentFoldersMenu();
    fileMenu->addSeparator();
    fileMenu->addAction(quitAction);
}

void MainWindow::updateRecentFoldersMenu()
{
    if (!m_recentFoldersMenu) {
        return;
    }

    m_recentFoldersMenu->clear();
    if (m_recentFolders.isEmpty()) {
        auto *emptyAction = m_recentFoldersMenu->addAction("(No recent folders)");
        emptyAction->setEnabled(false);
        return;
    }

    for (const auto &folderPath : m_recentFolders) {
        auto *action = m_recentFoldersMenu->addAction(folderPath);
        connect(action, &QAction::triggered, this, [this, folderPath]() {
            openFolderPath(folderPath);
        });
    }
}

void MainWindow::setupToolbar()
{
    auto *toolbar = addToolBar("Quick Actions");
    toolbar->setMovable(false);
    toolbar->setToolButtonStyle(Qt::ToolButtonIconOnly);
    toolbar->setIconSize(QSize(18, 18));

    auto *openAction = toolbar->addAction(style()->standardIcon(QStyle::SP_DirOpenIcon), "Open");
    auto *refreshAction = toolbar->addAction(style()->standardIcon(QStyle::SP_BrowserReload), "Refresh");
    auto *randomAction = toolbar->addAction(style()->standardIcon(QStyle::SP_BrowserStop), "Random");
    auto *exportCsvAction = toolbar->addAction(style()->standardIcon(QStyle::SP_DialogSaveButton), "Export CSV");
    auto *clearFiltersAction = toolbar->addAction(style()->standardIcon(QStyle::SP_DialogResetButton), "Clear Filters");

    openAction->setToolTip("Open folder");
    refreshAction->setToolTip("Refresh library");
    randomAction->setToolTip("Jump to random photo");
    exportCsvAction->setToolTip("Export visible photos to CSV");
    clearFiltersAction->setToolTip("Clear all filters");

    connect(openAction, &QAction::triggered, this, &MainWindow::openFolder);
    connect(refreshAction, &QAction::triggered, this, &MainWindow::refreshList);
    connect(randomAction, &QAction::triggered, this, [this]() {
        if (m_photoList->count() == 0) {
            return;
        }
        const int row = QRandomGenerator::global()->bounded(m_photoList->count());
        m_photoList->setCurrentRow(row);
    });
    connect(exportCsvAction, &QAction::triggered, this, &MainWindow::exportVisibleToCsv);
    connect(clearFiltersAction, &QAction::triggered, this, [this]() {
        m_nameFilter->clear();
        m_tagFilter->clear();
        m_favoritesOnly->setChecked(false);
        refreshList();
    });
}

void MainWindow::setupConnections()
{
    auto loadSavedSearches = [this]() {
        const QString current = m_savedSearchCombo->currentData().toString();
        m_savedSearchCombo->blockSignals(true);
        m_savedSearchCombo->clear();
        m_savedSearchCombo->addItem("Saved searches", QString());

        const QStringList rawEntries = QSettings("PhotoOrganizerQt", "PhotoOrganizerQt").value("savedSearches").toStringList();
        for (const auto &entry : rawEntries) {
            const QStringList parts = entry.split("||");
            if (parts.size() < 7 || parts[0].trimmed().isEmpty()) {
                continue;
            }
            m_savedSearchCombo->addItem(parts[0], entry);
        }

        int restoreIndex = 0;
        for (int i = 0; i < m_savedSearchCombo->count(); ++i) {
            if (m_savedSearchCombo->itemData(i).toString() == current) {
                restoreIndex = i;
                break;
            }
        }
        m_savedSearchCombo->setCurrentIndex(restoreIndex);
        m_savedSearchCombo->blockSignals(false);
    };

    loadSavedSearches();

    auto saveCurrentMetadata = [this]() {
        const QString path = currentPhotoPath();
        if (path.isEmpty()) {
            return;
        }

        m_library.updateTags(path, parseTags(m_tagsEdit->text()));
        m_library.updateFavorite(path, m_favoriteCheck->isChecked());
        m_library.updateRating(path, m_ratingSpin->value());
        setUnsavedChanges(false);
        refreshList();
    };

    auto selectAdjacent = [this](int delta) {
        if (m_photoList->count() == 0) {
            return;
        }
        const int current = m_photoList->currentRow();
        const int next = qBound(0, current + delta, m_photoList->count() - 1);
        m_photoList->setCurrentRow(next);
    };

    auto *nextShortcut = new QShortcut(QKeySequence(Qt::Key_Right), this);
    auto *prevShortcut = new QShortcut(QKeySequence(Qt::Key_Left), this);
    auto *nextAltShortcut = new QShortcut(QKeySequence("Ctrl+J"), this);
    auto *prevAltShortcut = new QShortcut(QKeySequence("Ctrl+K"), this);
    connect(nextShortcut, &QShortcut::activated, this, [selectAdjacent]() { selectAdjacent(1); });
    connect(prevShortcut, &QShortcut::activated, this, [selectAdjacent]() { selectAdjacent(-1); });
    connect(nextAltShortcut, &QShortcut::activated, this, [selectAdjacent]() { selectAdjacent(1); });
    connect(prevAltShortcut, &QShortcut::activated, this, [selectAdjacent]() { selectAdjacent(-1); });

    connect(m_nameFilter, &QLineEdit::textChanged, this, &MainWindow::refreshList);
    connect(m_tagFilter, &QLineEdit::textChanged, this, &MainWindow::refreshList);

    connect(m_saveSearchButton, &QPushButton::clicked, this, [this, loadSavedSearches]() {
        const QString name = QInputDialog::getText(this, "Save Search", "Preset name:").trimmed();
        if (name.isEmpty()) {
            return;
        }

        QString serialized = QString("%1||%2||%3||%4||%5||%6||%7")
            .arg(name)
            .arg(m_nameFilter->text())
            .arg(m_tagFilter->text())
            .arg(m_favoritesOnly->isChecked() ? "1" : "0")
            .arg(m_minRatingFilter->value())
            .arg(m_sortCombo->currentText())
            .arg(m_viewModeCombo->currentText());

        QStringList entries = QSettings("PhotoOrganizerQt", "PhotoOrganizerQt").value("savedSearches").toStringList();
        for (int i = entries.size() - 1; i >= 0; --i) {
            if (entries[i].startsWith(name + "||")) {
                entries.removeAt(i);
            }
        }
        entries.prepend(serialized);
        QSettings("PhotoOrganizerQt", "PhotoOrganizerQt").setValue("savedSearches", entries);
        loadSavedSearches();
        statusBar()->showMessage("Saved search preset added.", 2000);
    });

    connect(m_savedSearchCombo, &QComboBox::currentIndexChanged, this, [this](int index) {
        if (index <= 0) {
            return;
        }

        const QString data = m_savedSearchCombo->itemData(index).toString();
        const QStringList parts = data.split("||");
        if (parts.size() < 7) {
            return;
        }

        m_nameFilter->setText(parts[1]);
        m_tagFilter->setText(parts[2]);
        m_favoritesOnly->setChecked(parts[3] == "1");
        m_minRatingFilter->setValue(parts[4].toInt());
        m_sortCombo->setCurrentText(parts[5]);
        m_viewModeCombo->setCurrentText(parts[6]);
        refreshList();
        statusBar()->showMessage(QString("Applied search preset: %1").arg(parts[0]), 2000);
    });

    connect(m_deleteSearchButton, &QPushButton::clicked, this, [this, loadSavedSearches]() {
        const int index = m_savedSearchCombo->currentIndex();
        if (index <= 0) {
            return;
        }

        const QString data = m_savedSearchCombo->itemData(index).toString();
        QStringList entries = QSettings("PhotoOrganizerQt", "PhotoOrganizerQt").value("savedSearches").toStringList();
        entries.removeAll(data);
        QSettings("PhotoOrganizerQt", "PhotoOrganizerQt").setValue("savedSearches", entries);
        loadSavedSearches();
        statusBar()->showMessage("Saved search preset deleted.", 2000);
    });

    connect(m_tagsEdit, &QLineEdit::textChanged, this, [this](const QString &) {
        if (!m_isLoadingSelection && !currentPhotoPath().isEmpty()) {
            setUnsavedChanges(true);
            if (m_autoSaveCheck->isChecked()) {
                const QString path = currentPhotoPath();
                m_library.updateTags(path, parseTags(m_tagsEdit->text()));
                setUnsavedChanges(false);
                refreshList();
            }
        }
    });

    connect(m_favoriteCheck, &QCheckBox::toggled, this, [this](bool) {
        if (!m_isLoadingSelection && !currentPhotoPath().isEmpty()) {
            setUnsavedChanges(true);
            if (m_autoSaveCheck->isChecked()) {
                const QString path = currentPhotoPath();
                m_library.updateFavorite(path, m_favoriteCheck->isChecked());
                setUnsavedChanges(false);
                refreshList();
            }
        }
    });

    connect(m_ratingSpin, &QSpinBox::valueChanged, this, [this](int) {
        if (!m_isLoadingSelection && !currentPhotoPath().isEmpty()) {
            setUnsavedChanges(true);
            if (m_autoSaveCheck->isChecked()) {
                const QString path = currentPhotoPath();
                m_library.updateRating(path, m_ratingSpin->value());
                setUnsavedChanges(false);
                refreshList();
            }
        }
    });

    connect(m_autoSaveCheck, &QCheckBox::toggled, this, [](bool checked) {
        QSettings("PhotoOrganizerQt", "PhotoOrganizerQt").setValue("autoSave", checked);
    });

    connect(m_sortCombo, &QComboBox::currentTextChanged, this, [this]() { refreshList(); });
    connect(m_viewModeCombo, &QComboBox::currentTextChanged, this, [this]() { refreshList(); });
    connect(m_minRatingFilter, &QSpinBox::valueChanged, this, [this](int) { refreshList(); });
    connect(m_thumbSizeSlider, &QSlider::valueChanged, this, [this](int value) {
        m_photoList->setIconSize(QSize(value, value));
        refreshList();
    });
    connect(m_favoritesOnly, &QCheckBox::toggled, this, [this]() { refreshList(); });

    connect(m_photoList, &QListWidget::currentRowChanged, this, [this]() {
        loadSelectionDetails();
    });

    connect(m_photoList, &QListWidget::itemSelectionChanged, this, [this]() {
        m_bulkActionsButton->setEnabled(!selectedPhotoPaths().isEmpty());
    });

    connect(m_photoList, &QListWidget::itemDoubleClicked, this, [this](QListWidgetItem *item) {
        if (!item) {
            return;
        }
        QDesktopServices::openUrl(QUrl::fromLocalFile(item->data(Qt::UserRole).toString()));
    });

    connect(m_saveButton, &QPushButton::clicked, this, [this, saveCurrentMetadata]() {
        saveCurrentMetadata();
        statusBar()->showMessage("Photo metadata saved.", 2500);
    });

    connect(m_bulkAddTagsAction, &QAction::triggered, this, [this]() {
        const QStringList selected = selectedPhotoPaths();
        const QStringList newTags = parseTags(m_bulkTagsEdit->text());

        if (selected.isEmpty() || newTags.isEmpty()) {
            return;
        }

        for (const auto &path : selected) {
            const PhotoItem item = m_library.byAbsolutePath(path);
            QStringList merged = item.tags;
            merged.append(newTags);
            m_library.updateTags(path, merged);
        }

        refreshList();
        loadSelectionDetails();
        statusBar()->showMessage("Bulk tags applied.", 2500);
    });

    connect(m_bulkFavoriteAction, &QAction::triggered, this, [this]() {
        const auto selected = selectedPhotoPaths();
        if (selected.isEmpty()) {
            return;
        }

        for (const auto &path : selected) {
            m_library.updateFavorite(path, m_favoriteCheck->isChecked());
        }

        refreshList();
        loadSelectionDetails();
        statusBar()->showMessage("Bulk favorite applied.", 2500);
    });

    connect(m_bulkRatingAction, &QAction::triggered, this, [this]() {
        const auto selected = selectedPhotoPaths();
        if (selected.isEmpty()) {
            return;
        }

        for (const auto &path : selected) {
            m_library.updateRating(path, m_ratingSpin->value());
        }

        refreshList();
        loadSelectionDetails();
        statusBar()->showMessage("Bulk rating applied.", 2500);
    });

    connect(m_openPhotoButton, &QPushButton::clicked, this, [this]() {
        const QString path = currentPhotoPath();
        if (!path.isEmpty()) {
            QDesktopServices::openUrl(QUrl::fromLocalFile(path));
        }
    });

    connect(m_openLocationButton, &QPushButton::clicked, this, [this]() {
        const QString path = currentPhotoPath();
        if (!path.isEmpty()) {
            QDesktopServices::openUrl(QUrl::fromLocalFile(QFileInfo(path).absolutePath()));
        }
    });

    connect(m_copyPathButton, &QPushButton::clicked, this, [this]() {
        const QString path = currentPhotoPath();
        if (!path.isEmpty()) {
            QApplication::clipboard()->setText(path);
            statusBar()->showMessage("Path copied to clipboard.", 2000);
        }
    });

    connect(m_slideshowButton, &QPushButton::clicked, this, [this]() {
        if (m_slideshowTimer->isActive()) {
            m_slideshowTimer->stop();
            m_slideshowButton->setText("Slideshow");
            return;
        }
        if (m_photoList->count() == 0) {
            return;
        }
        m_slideshowTimer->start();
        m_slideshowButton->setText("Stop");
    });

    connect(m_slideshowTimer, &QTimer::timeout, this, [this]() {
        if (m_photoList->count() == 0) {
            return;
        }
        const int next = (m_photoList->currentRow() + 1) % m_photoList->count();
        m_photoList->setCurrentRow(next);
    });

    connect(m_createAlbumButton, &QPushButton::clicked, this, [this]() {
        if (m_library.createAlbum(m_newAlbumEdit->text())) {
            m_newAlbumEdit->clear();
            refreshAlbumsWorkspace();
            statusBar()->showMessage("Album created.", 2000);
        }
    });

    connect(m_deleteAlbumButton, &QPushButton::clicked, this, [this]() {
        const auto *item = m_albumList->currentItem();
        if (!item) {
            return;
        }
        if (item->data(Qt::UserRole).toString().startsWith("smart:")) {
            statusBar()->showMessage("Smart albums are dynamic and cannot be deleted.", 2200);
            return;
        }
        m_library.deleteAlbum(item->data(Qt::UserRole).toString());
        refreshAlbumsWorkspace();
        statusBar()->showMessage("Album deleted.", 2000);
    });

    connect(m_createSmartRuleButton, &QPushButton::clicked, this, [this]() {
        const QString name = m_smartRuleNameEdit->text().trimmed();
        if (name.isEmpty()) {
            return;
        }

        const QString serialized = QString("%1||%2||%3||%4")
            .arg(name)
            .arg(m_smartRuleFavoriteOnlyCheck->isChecked() ? "1" : "0")
            .arg(m_smartRuleMinRatingSpin->value())
            .arg(m_smartRuleTagEdit->text().trimmed());

        QStringList rules = QSettings("PhotoOrganizerQt", "PhotoOrganizerQt").value("customSmartAlbumRules").toStringList();
        for (int i = rules.size() - 1; i >= 0; --i) {
            if (rules[i].startsWith(name + "||")) {
                rules.removeAt(i);
            }
        }
        rules.prepend(serialized);
        QSettings("PhotoOrganizerQt", "PhotoOrganizerQt").setValue("customSmartAlbumRules", rules);

        m_smartRuleNameEdit->clear();
        m_smartRuleTagEdit->clear();
        m_smartRuleMinRatingSpin->setValue(0);
        m_smartRuleFavoriteOnlyCheck->setChecked(false);
        refreshAlbumsWorkspace();
        statusBar()->showMessage("Custom smart rule created.", 2200);
    });

    connect(m_deleteSmartRuleButton, &QPushButton::clicked, this, [this]() {
        const auto *item = m_albumList->currentItem();
        if (!item) {
            return;
        }

        const QString key = item->data(Qt::UserRole).toString();
        if (!key.startsWith("customsmart:")) {
            statusBar()->showMessage("Select a custom smart album to delete.", 2200);
            return;
        }

        const QString name = key.mid(QString("customsmart:").size());
        QStringList rules = QSettings("PhotoOrganizerQt", "PhotoOrganizerQt").value("customSmartAlbumRules").toStringList();
        for (int i = rules.size() - 1; i >= 0; --i) {
            if (rules[i].startsWith(name + "||")) {
                rules.removeAt(i);
            }
        }
        QSettings("PhotoOrganizerQt", "PhotoOrganizerQt").setValue("customSmartAlbumRules", rules);
        refreshAlbumsWorkspace();
        statusBar()->showMessage("Custom smart rule deleted.", 2200);
    });

    connect(m_addToAlbumButton, &QPushButton::clicked, this, [this]() {
        const auto *albumItem = m_albumList->currentItem();
        if (!albumItem) {
            return;
        }
        const QString albumKey = albumItem->data(Qt::UserRole).toString();
        if (albumKey.startsWith("smart:")) {
            statusBar()->showMessage("Cannot manually add photos to smart albums.", 2200);
            return;
        }
        const auto selected = selectedPhotoPaths();
        if (selected.isEmpty()) {
            return;
        }
        m_library.addPhotosToAlbum(albumKey, selected);
        refreshAlbumsWorkspace();
        statusBar()->showMessage("Photos added to album.", 2000);
    });

    connect(m_removeFromAlbumButton, &QPushButton::clicked, this, [this]() {
        const auto *albumItem = m_albumList->currentItem();
        if (!albumItem) {
            return;
        }
        const QString albumKey = albumItem->data(Qt::UserRole).toString();
        if (albumKey.startsWith("smart:")) {
            statusBar()->showMessage("Cannot manually remove photos from smart albums.", 2200);
            return;
        }
        QStringList removePaths;
        for (auto *item : m_albumPhotoList->selectedItems()) {
            removePaths.push_back(item->data(Qt::UserRole).toString());
        }
        if (removePaths.isEmpty()) {
            return;
        }
        m_library.removePhotosFromAlbum(albumKey, removePaths);
        refreshAlbumsWorkspace();
        statusBar()->showMessage("Photos removed from album.", 2000);
    });

    connect(m_albumList, &QListWidget::currentRowChanged, this, [this]() {
        refreshAlbumsWorkspace();
    });

    connect(m_albumPhotoList, &QListWidget::itemDoubleClicked, this, [this](QListWidgetItem *item) {
        if (!item) {
            return;
        }
        const QString path = item->data(Qt::UserRole).toString();
        QDesktopServices::openUrl(QUrl::fromLocalFile(path));
    });

    connect(m_filesTree, &QTreeView::doubleClicked, this, [this](const QModelIndex &index) {
        const QString path = m_fileModel->filePath(index);
        if (!QFileInfo(path).isFile()) {
            return;
        }
        QDesktopServices::openUrl(QUrl::fromLocalFile(path));
    });

    connect(m_renameFileButton, &QPushButton::clicked, this, [this]() {
        const QModelIndex index = m_filesTree->currentIndex();
        if (!index.isValid()) {
            return;
        }

        const QString path = m_fileModel->filePath(index);
        const QFileInfo info(path);
        if (!info.exists() || !info.isFile()) {
            return;
        }

        const QString proposedName = QInputDialog::getText(this, "Rename File", "New file name:", QLineEdit::Normal, info.fileName()).trimmed();
        if (proposedName.isEmpty() || proposedName == info.fileName()) {
            return;
        }

        const QString newPath = info.dir().filePath(proposedName);
        if (QFile::exists(newPath)) {
            QMessageBox::warning(this, "Rename failed", "A file with that name already exists.");
            return;
        }

        if (!QFile::rename(path, newPath)) {
            QMessageBox::warning(this, "Rename failed", "Could not rename file.");
            return;
        }

        refreshFilesWorkspace();
        refreshList();
        statusBar()->showMessage("File renamed.", 2200);
    });

    connect(m_openContainingFolderButton, &QPushButton::clicked, this, [this]() {
        const QModelIndex index = m_filesTree->currentIndex();
        if (!index.isValid()) {
            return;
        }

        const QString path = m_fileModel->filePath(index);
        const QFileInfo info(path);
        const QString dirPath = info.isDir() ? info.absoluteFilePath() : info.absolutePath();
        if (!dirPath.isEmpty()) {
            QDesktopServices::openUrl(QUrl::fromLocalFile(dirPath));
        }
    });

    connect(m_timelineList, &QListWidget::itemDoubleClicked, this, [this](QListWidgetItem *item) {
        if (!item) {
            return;
        }
        const QString path = item->data(Qt::UserRole).toString();
        if (path.isEmpty()) {
            return;
        }

        for (int i = 0; i < m_photoList->count(); ++i) {
            auto *listItem = m_photoList->item(i);
            if (listItem->data(Qt::UserRole).toString() == path) {
                m_photoList->setCurrentRow(i);
                m_suiteTabs->setCurrentIndex(0);
                break;
            }
        }
    });

    connect(m_timelineYearFilter, &QComboBox::currentTextChanged, this, [this]() {
        refreshTimelineWorkspace();
    });

    connect(m_scanDuplicatesButton, &QPushButton::clicked, this, [this]() {
        m_duplicatesList->clear();

        QHash<QString, QStringList> grouped;
        for (const auto &photo : m_library.allItems()) {
            const QFileInfo info(photo.absolutePath);
            const QString key = QString("%1|%2").arg(info.size()).arg(info.fileName().toLower());
            grouped[key].push_back(photo.absolutePath);
        }

        int groupCount = 0;
        for (auto it = grouped.begin(); it != grouped.end(); ++it) {
            if (it.value().size() < 2) {
                continue;
            }
            ++groupCount;
            auto *header = new QListWidgetItem(QString("Group %1 (%2 files)").arg(groupCount).arg(it.value().size()), m_duplicatesList);
            header->setFlags(Qt::NoItemFlags);
            for (const auto &path : it.value()) {
                auto *item = new QListWidgetItem(QString("  %1").arg(QFileInfo(path).fileName()), m_duplicatesList);
                item->setData(Qt::UserRole, path);
                item->setToolTip(path);
            }
        }

        if (groupCount == 0) {
            m_duplicatesList->addItem("No duplicate groups found.");
        }

        statusBar()->showMessage(QString("Duplicate scan complete: %1 groups").arg(groupCount), 2500);
    });

    connect(m_duplicatesList, &QListWidget::itemDoubleClicked, this, [this](QListWidgetItem *item) {
        if (!item) {
            return;
        }
        const QString path = item->data(Qt::UserRole).toString();
        if (!path.isEmpty()) {
            QDesktopServices::openUrl(QUrl::fromLocalFile(path));
        }
    });

    connect(m_brightnessSlider, &QSlider::valueChanged, this, [this](int) {
        applyEditorAdjustments();
    });
    connect(m_contrastSlider, &QSlider::valueChanged, this, [this](int) {
        applyEditorAdjustments();
    });
    connect(m_saturationSlider, &QSlider::valueChanged, this, [this](int) {
        applyEditorAdjustments();
    });
    connect(m_temperatureSlider, &QSlider::valueChanged, this, [this](int) {
        applyEditorAdjustments();
    });
    connect(m_vignetteSlider, &QSlider::valueChanged, this, [this](int) {
        applyEditorAdjustments();
    });
    connect(m_grayscaleCheck, &QCheckBox::toggled, this, [this](bool) {
        applyEditorAdjustments();
    });
    connect(m_sepiaCheck, &QCheckBox::toggled, this, [this](bool) {
        applyEditorAdjustments();
    });

    connect(m_editorResetButton, &QPushButton::clicked, this, [this]() {
        m_brightnessSlider->setValue(0);
        m_contrastSlider->setValue(0);
        m_saturationSlider->setValue(0);
        m_temperatureSlider->setValue(0);
        m_vignetteSlider->setValue(0);
        m_grayscaleCheck->setChecked(false);
        m_sepiaCheck->setChecked(false);
        applyEditorAdjustments();
    });

    connect(m_editorBeforeAfterButton, &QPushButton::toggled, this, [this](bool checked) {
        m_editorShowOriginal = checked;
        updateEditorPreview();
    });

    connect(m_editorRotateLeftButton, &QPushButton::clicked, this, [this]() {
        if (m_editorOriginalImage.isNull()) {
            return;
        }
        m_editorUndoStack.push_back(m_editorOriginalImage);
        m_editorRedoStack.clear();
        m_editorOriginalImage = m_editorOriginalImage.transformed(QTransform().rotate(-90), Qt::SmoothTransformation);
        applyEditorAdjustments();
    });

    connect(m_editorRotateRightButton, &QPushButton::clicked, this, [this]() {
        if (m_editorOriginalImage.isNull()) {
            return;
        }
        m_editorUndoStack.push_back(m_editorOriginalImage);
        m_editorRedoStack.clear();
        m_editorOriginalImage = m_editorOriginalImage.transformed(QTransform().rotate(90), Qt::SmoothTransformation);
        applyEditorAdjustments();
    });

    connect(m_editorUndoButton, &QPushButton::clicked, this, [this]() {
        if (m_editorUndoStack.isEmpty()) {
            return;
        }
        m_editorRedoStack.push_back(m_editorOriginalImage);
        m_editorOriginalImage = m_editorUndoStack.takeLast();
        applyEditorAdjustments();
    });

    connect(m_editorRedoButton, &QPushButton::clicked, this, [this]() {
        if (m_editorRedoStack.isEmpty()) {
            return;
        }
        m_editorUndoStack.push_back(m_editorOriginalImage);
        m_editorOriginalImage = m_editorRedoStack.takeLast();
        applyEditorAdjustments();
    });

    connect(m_editorAutoEnhanceButton, &QPushButton::clicked, this, [this]() {
        if (m_editorOriginalImage.isNull()) {
            return;
        }

        m_editorUndoStack.push_back(m_editorOriginalImage);
        m_editorRedoStack.clear();

        double sumLuma = 0.0;
        const int pixelCount = m_editorOriginalImage.width() * m_editorOriginalImage.height();
        for (int y = 0; y < m_editorOriginalImage.height(); ++y) {
            const QRgb *line = reinterpret_cast<const QRgb *>(m_editorOriginalImage.constScanLine(y));
            for (int x = 0; x < m_editorOriginalImage.width(); ++x) {
                sumLuma += qGray(line[x]);
            }
        }

        const double avg = pixelCount > 0 ? sumLuma / pixelCount : 127.0;
        const int targetBrightness = qBound(-30, static_cast<int>(std::round((127.0 - avg) * 0.35)), 30);

        m_brightnessSlider->setValue(targetBrightness);
        m_contrastSlider->setValue(12);
        m_saturationSlider->setValue(10);
        m_temperatureSlider->setValue(4);
        statusBar()->showMessage("Auto-enhance applied.", 2200);
    });

    connect(m_editorSaveCopyButton, &QPushButton::clicked, this, [this]() {
        if (m_editorPreviewImage.isNull()) {
            return;
        }
        const QString currentPath = currentPhotoPath();
        const QString baseDir = currentPath.isEmpty() ? QDir::homePath() : QFileInfo(currentPath).absolutePath();
        const QString savePath = QFileDialog::getSaveFileName(
            this,
            "Save edited copy",
            QDir(baseDir).filePath("edited_copy.jpg"),
            "Images (*.jpg *.jpeg *.png *.bmp *.webp)");

        if (savePath.isEmpty()) {
            return;
        }

        if (!m_editorPreviewImage.save(savePath)) {
            QMessageBox::warning(this, "Save failed", "Could not save edited image.");
            return;
        }

        statusBar()->showMessage("Edited copy saved.", 2500);
    });

    connect(m_editorSaveSnapshotButton, &QPushButton::clicked, this, [this]() {
        if (m_editorPreviewImage.isNull()) {
            return;
        }

        const QString name = QInputDialog::getText(this, "Save Snapshot", "Snapshot name:").trimmed();
        if (name.isEmpty()) {
            return;
        }

        m_editorSnapshots.insert(name, m_editorPreviewImage);
        m_editorSnapshotsList->clear();
        for (const auto &snapshotName : m_editorSnapshots.keys()) {
            m_editorSnapshotsList->addItem(snapshotName);
        }
        statusBar()->showMessage("Snapshot saved.", 2000);
    });

    connect(m_editorSnapshotsList, &QListWidget::itemDoubleClicked, this, [this](QListWidgetItem *item) {
        if (!item) {
            return;
        }
        const QString key = item->text();
        if (!m_editorSnapshots.contains(key)) {
            return;
        }

        m_editorUndoStack.push_back(m_editorOriginalImage);
        m_editorRedoStack.clear();
        m_editorOriginalImage = m_editorSnapshots.value(key).convertToFormat(QImage::Format_RGB32);
        m_brightnessSlider->setValue(0);
        m_contrastSlider->setValue(0);
        m_saturationSlider->setValue(0);
        m_temperatureSlider->setValue(0);
        m_vignetteSlider->setValue(0);
        m_grayscaleCheck->setChecked(false);
        m_sepiaCheck->setChecked(false);
        applyEditorAdjustments();
        statusBar()->showMessage("Snapshot applied.", 2000);
    });
}

void MainWindow::openFolder()
{
    const QString defaultPath = m_library.rootFolder().isEmpty()
        ? QStandardPaths::writableLocation(QStandardPaths::PicturesLocation)
        : m_library.rootFolder();

    const QString selected = QFileDialog::getExistingDirectory(
        this,
        "Select photo folder",
        defaultPath,
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if (!selected.isEmpty()) {
        openFolderPath(selected);
    }
}

void MainWindow::openFolderPath(const QString &folderPath)
{
    if (folderPath.isEmpty()) {
        return;
    }

    if (!m_library.loadFolder(folderPath)) {
        QMessageBox::warning(this, "Failed", "Could not open the selected folder.");
        return;
    }

    m_nameFilter->clear();
    m_tagFilter->clear();
    m_favoritesOnly->setChecked(false);

    refreshList();
    refreshAlbumsWorkspace();
    refreshFilesWorkspace();

    m_recentFolders.removeAll(folderPath);
    m_recentFolders.prepend(folderPath);
    while (m_recentFolders.size() > 10) {
        m_recentFolders.removeLast();
    }
    QSettings("PhotoOrganizerQt", "PhotoOrganizerQt").setValue("recentFolders", m_recentFolders);
    QSettings("PhotoOrganizerQt", "PhotoOrganizerQt").setValue("lastFolder", folderPath);
    updateRecentFoldersMenu();

    statusBar()->showMessage(QString("Loaded %1 photos from %2")
                                 .arg(m_library.allItems().size())
                                 .arg(folderPath),
                             4000);
}

void MainWindow::refreshList()
{
    const QString previousPath = currentPhotoPath();

    const bool listMode = m_viewModeCombo->currentText() == "List";
    m_photoList->setViewMode(listMode ? QListView::ListMode : QListView::IconMode);
    m_photoList->setWordWrap(!listMode);
    m_photoList->setSpacing(listMode ? 4 : 10);
    m_photoList->setUniformItemSizes(listMode);

    m_photoList->clear();

    auto items = m_library.filteredItems(
        m_nameFilter->text(),
        m_tagFilter->text(),
        m_favoritesOnly->isChecked());

    const QString sortMode = m_sortCombo->currentText();
    std::sort(items.begin(), items.end(), [sortMode](const PhotoItem &left, const PhotoItem &right) {
        const QString leftName = QFileInfo(left.absolutePath).fileName().toLower();
        const QString rightName = QFileInfo(right.absolutePath).fileName().toLower();

        if (sortMode == "Name (Z-A)") {
            return leftName > rightName;
        }
        if (sortMode == "Rating (High-Low)") {
            if (left.rating != right.rating) {
                return left.rating > right.rating;
            }
            return leftName < rightName;
        }
        if (sortMode == "Path") {
            return left.relativePath.toLower() < right.relativePath.toLower();
        }
        return leftName < rightName;
    });

    const int thumbSize = m_thumbSizeSlider->value();
    m_photoList->setIconSize(QSize(thumbSize, thumbSize));

    const int minimumRating = m_minRatingFilter->value();
    for (const auto &item : items) {
        if (item.rating < minimumRating) {
            continue;
        }

        auto *listItem = new QListWidgetItem(m_photoList);

        const QPixmap pix(item.absolutePath);
        const QPixmap thumb = pix.isNull()
            ? style()->standardIcon(QStyle::SP_FileIcon).pixmap(thumbSize, thumbSize)
            : pix.scaled(thumbSize, thumbSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        listItem->setIcon(QIcon(thumb));

        QString title = QFileInfo(item.absolutePath).fileName();
        if (item.favorite) {
            title.prepend("★ ");
        }
        if (item.rating > 0) {
            title.append(QString("  (%1/5)").arg(item.rating));
        }

        listItem->setText(title);
        listItem->setData(Qt::UserRole, item.absolutePath);
        listItem->setToolTip(item.relativePath);
    }

    if (!previousPath.isEmpty()) {
        for (int row = 0; row < m_photoList->count(); ++row) {
            auto *item = m_photoList->item(row);
            if (item->data(Qt::UserRole).toString() == previousPath) {
                m_photoList->setCurrentRow(row);
                break;
            }
        }
    }

    if (m_photoList->currentItem() == nullptr && m_photoList->count() > 0) {
        m_photoList->setCurrentRow(0);
    }

    if (m_photoList->count() == 0) {
        clearDetails();
    }

    refreshAlbumsWorkspace();
    refreshTimelineWorkspace();
    refreshTimelineWorkspace();
}

void MainWindow::refreshTimelineWorkspace()
{
    m_timelineList->clear();

    QStringList availableYears;
    for (const auto &photo : m_library.allItems()) {
        const QString year = QFileInfo(photo.absolutePath).lastModified().toString("yyyy");
        if (!availableYears.contains(year)) {
            availableYears.push_back(year);
        }
    }
    std::sort(availableYears.begin(), availableYears.end(), std::greater<QString>());

    const QString previousYear = m_timelineYearFilter->currentData().toString();
    m_timelineYearFilter->blockSignals(true);
    m_timelineYearFilter->clear();
    m_timelineYearFilter->addItem("All years", QString());
    for (const auto &year : availableYears) {
        m_timelineYearFilter->addItem(year, year);
    }
    int restoreYearIndex = 0;
    for (int i = 0; i < m_timelineYearFilter->count(); ++i) {
        if (m_timelineYearFilter->itemData(i).toString() == previousYear) {
            restoreYearIndex = i;
            break;
        }
    }
    m_timelineYearFilter->setCurrentIndex(restoreYearIndex);
    m_timelineYearFilter->blockSignals(false);

    const QString selectedYear = m_timelineYearFilter->currentData().toString();

    QMap<QString, QStringList> grouped;
    for (const auto &photo : m_library.allItems()) {
        const QFileInfo info(photo.absolutePath);
        if (!selectedYear.isEmpty() && info.lastModified().toString("yyyy") != selectedYear) {
            continue;
        }
        const QString monthKey = info.lastModified().toString("yyyy MMMM");
        grouped[monthKey].push_back(photo.absolutePath);
    }

    auto keys = grouped.keys();
    std::sort(keys.begin(), keys.end(), std::greater<QString>());
    for (const auto &key : keys) {
        auto *header = new QListWidgetItem(QString("%1 • %2 photos").arg(key).arg(grouped[key].size()), m_timelineList);
        header->setFlags(Qt::NoItemFlags);
        header->setForeground(QColor("#93a4bf"));

        const auto paths = grouped[key];
        for (const auto &path : paths) {
            auto *item = new QListWidgetItem(QString("  %1").arg(QFileInfo(path).fileName()), m_timelineList);
            item->setData(Qt::UserRole, path);
            item->setToolTip(path);
        }
    }
}

void MainWindow::refreshAlbumsWorkspace()
{
    const QString currentAlbum = m_albumList->currentItem()
        ? m_albumList->currentItem()->data(Qt::UserRole).toString()
        : QString();

    m_albumList->blockSignals(true);
    m_albumList->clear();
    int favoritesCount = 0;
    int topRatedCount = 0;
    int untaggedCount = 0;
    for (const auto &photo : m_library.allItems()) {
        if (photo.favorite) {
            ++favoritesCount;
        }
        if (photo.rating >= 4) {
            ++topRatedCount;
        }
        if (photo.tags.isEmpty()) {
            ++untaggedCount;
        }
    }

    auto *favoritesSmart = new QListWidgetItem(QString("Smart: Favorites (%1)").arg(favoritesCount), m_albumList);
    favoritesSmart->setData(Qt::UserRole, "smart:favorites");
    auto *topRatedSmart = new QListWidgetItem(QString("Smart: Top Rated (%1)").arg(topRatedCount), m_albumList);
    topRatedSmart->setData(Qt::UserRole, "smart:toprated");
    auto *untaggedSmart = new QListWidgetItem(QString("Smart: Untagged (%1)").arg(untaggedCount), m_albumList);
    untaggedSmart->setData(Qt::UserRole, "smart:untagged");

    const QStringList customRules = QSettings("PhotoOrganizerQt", "PhotoOrganizerQt").value("customSmartAlbumRules").toStringList();
    for (const auto &rule : customRules) {
        const QStringList parts = rule.split("||");
        if (parts.size() < 4 || parts[0].trimmed().isEmpty()) {
            continue;
        }

        const QString name = parts[0];
        const bool favOnly = parts[1] == "1";
        const int minRating = parts[2].toInt();
        const QString tagNeedle = parts[3].trimmed().toLower();

        int count = 0;
        for (const auto &photo : m_library.allItems()) {
            if (favOnly && !photo.favorite) {
                continue;
            }
            if (photo.rating < minRating) {
                continue;
            }
            if (!tagNeedle.isEmpty()) {
                bool match = false;
                for (const auto &tag : photo.tags) {
                    if (tag.toLower().contains(tagNeedle)) {
                        match = true;
                        break;
                    }
                }
                if (!match) {
                    continue;
                }
            }
            ++count;
        }

        auto *customItem = new QListWidgetItem(QString("Smart: %1 (%2)").arg(name).arg(count), m_albumList);
        customItem->setData(Qt::UserRole, "customsmart:" + name);
        customItem->setForeground(QColor("#bfd0ff"));
    }

    const auto albums = m_library.albumNames();
    for (const auto &album : albums) {
        const int count = m_library.photosForAlbum(album).size();
        auto *albumItem = new QListWidgetItem(QString("%1 (%2)").arg(album).arg(count), m_albumList);
        albumItem->setData(Qt::UserRole, album);
    }

    if (!currentAlbum.isEmpty()) {
        for (int i = 0; i < m_albumList->count(); ++i) {
            if (m_albumList->item(i)->data(Qt::UserRole).toString() == currentAlbum) {
                m_albumList->setCurrentRow(i);
                break;
            }
        }
    }

    if (m_albumList->currentRow() < 0 && m_albumList->count() > 0) {
        m_albumList->setCurrentRow(0);
    }
    m_albumList->blockSignals(false);

    m_albumPhotoList->clear();
    auto *albumItem = m_albumList->currentItem();
    if (!albumItem) {
        return;
    }

    QStringList displayPaths;
    const QString selected = albumItem->data(Qt::UserRole).toString();
    if (selected == "smart:favorites") {
        for (const auto &photo : m_library.allItems()) {
            if (photo.favorite) {
                displayPaths.push_back(photo.absolutePath);
            }
        }
    } else if (selected == "smart:toprated") {
        for (const auto &photo : m_library.allItems()) {
            if (photo.rating >= 4) {
                displayPaths.push_back(photo.absolutePath);
            }
        }
    } else if (selected == "smart:untagged") {
        for (const auto &photo : m_library.allItems()) {
            if (photo.tags.isEmpty()) {
                displayPaths.push_back(photo.absolutePath);
            }
        }
    } else if (selected.startsWith("customsmart:")) {
        const QString name = selected.mid(QString("customsmart:").size());
        const QStringList customRules = QSettings("PhotoOrganizerQt", "PhotoOrganizerQt").value("customSmartAlbumRules").toStringList();
        QStringList found;
        for (const auto &rule : customRules) {
            const QStringList parts = rule.split("||");
            if (parts.size() < 4 || parts[0] != name) {
                continue;
            }

            const bool favOnly = parts[1] == "1";
            const int minRating = parts[2].toInt();
            const QString tagNeedle = parts[3].trimmed().toLower();

            for (const auto &photo : m_library.allItems()) {
                if (favOnly && !photo.favorite) {
                    continue;
                }
                if (photo.rating < minRating) {
                    continue;
                }
                if (!tagNeedle.isEmpty()) {
                    bool match = false;
                    for (const auto &tag : photo.tags) {
                        if (tag.toLower().contains(tagNeedle)) {
                            match = true;
                            break;
                        }
                    }
                    if (!match) {
                        continue;
                    }
                }
                found.push_back(photo.absolutePath);
            }
            break;
        }
        displayPaths = found;
    } else {
        displayPaths = m_library.photosForAlbum(selected);
    }

    for (const auto &path : displayPaths) {
        if (!m_library.contains(path)) {
            continue;
        }
        auto *listItem = new QListWidgetItem(QFileInfo(path).fileName(), m_albumPhotoList);
        listItem->setData(Qt::UserRole, path);
        listItem->setToolTip(path);
    }
}

void MainWindow::refreshFilesWorkspace()
{
    QString root = m_library.rootFolder();
    if (root.isEmpty()) {
        root = QDir::homePath();
    }

    const QModelIndex idx = m_fileModel->setRootPath(root);
    m_filesTree->setRootIndex(idx);
    m_filesTree->setColumnHidden(1, true);
    m_filesTree->setColumnHidden(2, true);
    m_filesTree->setColumnHidden(3, false);
}

void MainWindow::exportVisibleToCsv()
{
    if (m_photoList->count() == 0) {
        statusBar()->showMessage("No visible photos to export.", 2500);
        return;
    }

    const QString defaultPath = m_library.rootFolder().isEmpty()
        ? QDir::homePath()
        : m_library.rootFolder();

    const QString filePath = QFileDialog::getSaveFileName(
        this,
        "Export visible photos to CSV",
        QDir(defaultPath).filePath("photo_report.csv"),
        "CSV Files (*.csv)");

    if (filePath.isEmpty()) {
        return;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        QMessageBox::warning(this, "Export failed", "Could not write CSV file.");
        return;
    }

    QTextStream out(&file);
    out << "file_name,relative_path,favorite,rating,tags\n";

    for (int row = 0; row < m_photoList->count(); ++row) {
        auto *itemWidget = m_photoList->item(row);
        const QString path = itemWidget->data(Qt::UserRole).toString();
        const PhotoItem item = m_library.byAbsolutePath(path);

        QString escapedFileName = QFileInfo(item.absolutePath).fileName();
        QString escapedRelative = item.relativePath;
        QString escapedTags = item.tags.join("|");

        escapedFileName.replace('"', "\"\"");
        escapedRelative.replace('"', "\"\"");
        escapedTags.replace('"', "\"\"");

        out << '"' << escapedFileName << "\",";
        out << '"' << escapedRelative << "\",";
        out << (item.favorite ? "true" : "false") << ',';
        out << item.rating << ',';
        out << '"' << escapedTags << "\"\n";
    }

    statusBar()->showMessage("CSV export complete.", 2500);
}

void MainWindow::loadSelectionDetails()
{
    const QString path = currentPhotoPath();
    if (path.isEmpty() || !m_library.contains(path)) {
        clearDetails();
        return;
    }

    const PhotoItem photo = m_library.byAbsolutePath(path);
    m_isLoadingSelection = true;

    const QPixmap pix(photo.absolutePath);
    if (pix.isNull()) {
        m_previewLabel->setText("Preview unavailable");
        m_previewLabel->setPixmap(QPixmap());
    } else {
        m_previewLabel->setPixmap(pix.scaled(
            m_previewLabel->size(),
            Qt::KeepAspectRatio,
            Qt::SmoothTransformation));
    }

    m_pathLabel->setText(photo.absolutePath);
    m_tagsEdit->setText(photo.tags.join(", "));
    m_favoriteCheck->setChecked(photo.favorite);
    m_ratingSpin->setValue(photo.rating);
    m_isLoadingSelection = false;

    setUnsavedChanges(false);
    m_saveButton->setEnabled(true);
    m_openPhotoButton->setEnabled(true);
    m_openLocationButton->setEnabled(true);
    m_copyPathButton->setEnabled(true);
    m_slideshowButton->setEnabled(true);
    m_bulkActionsButton->setEnabled(!selectedPhotoPaths().isEmpty());

    loadEditorPhoto(photo.absolutePath);
}

void MainWindow::clearDetails()
{
    m_isLoadingSelection = true;
    m_previewLabel->setText("No photo selected");
    m_previewLabel->setPixmap(QPixmap());
    m_pathLabel->setText("-");
    m_tagsEdit->clear();
    m_favoriteCheck->setChecked(false);
    m_ratingSpin->setValue(0);
    m_isLoadingSelection = false;

    setUnsavedChanges(false);
    m_saveButton->setEnabled(false);
    m_openPhotoButton->setEnabled(false);
    m_openLocationButton->setEnabled(false);
    m_copyPathButton->setEnabled(false);
    m_slideshowButton->setEnabled(m_photoList->count() > 0);
    m_bulkActionsButton->setEnabled(false);

    m_editorOriginalImage = QImage();
    m_editorPreviewImage = QImage();
    m_editorUndoStack.clear();
    m_editorRedoStack.clear();
    m_editorSnapshots.clear();
    m_editorSnapshotsList->clear();
    m_editorBeforeAfterButton->setChecked(false);
    m_editorShowOriginal = false;
    m_editorPreviewLabel->setText("Editor preview (select a photo)");
    m_editorPreviewLabel->setPixmap(QPixmap());
}

void MainWindow::setUnsavedChanges(bool value)
{
    m_hasUnsavedChanges = value;
    setWindowModified(value);
    if (value) {
        statusBar()->showMessage("Unsaved metadata changes.");
    }
}

void MainWindow::loadEditorPhoto(const QString &path)
{
    if (path.isEmpty()) {
        m_editorOriginalImage = QImage();
        m_editorPreviewImage = QImage();
        updateEditorPreview();
        return;
    }

    QImage loaded(path);
    if (loaded.isNull()) {
        return;
    }

    m_editorOriginalImage = loaded.convertToFormat(QImage::Format_RGB32);
    m_editorUndoStack.clear();
    m_editorRedoStack.clear();
    m_editorSnapshots.clear();
    m_editorSnapshotsList->clear();
    m_brightnessSlider->blockSignals(true);
    m_contrastSlider->blockSignals(true);
    m_saturationSlider->blockSignals(true);
    m_temperatureSlider->blockSignals(true);
    m_vignetteSlider->blockSignals(true);
    m_grayscaleCheck->blockSignals(true);
    m_sepiaCheck->blockSignals(true);
    m_brightnessSlider->setValue(0);
    m_contrastSlider->setValue(0);
    m_saturationSlider->setValue(0);
    m_temperatureSlider->setValue(0);
    m_vignetteSlider->setValue(0);
    m_grayscaleCheck->setChecked(false);
    m_sepiaCheck->setChecked(false);
    m_editorBeforeAfterButton->setChecked(false);
    m_editorShowOriginal = false;
    m_brightnessSlider->blockSignals(false);
    m_contrastSlider->blockSignals(false);
    m_saturationSlider->blockSignals(false);
    m_temperatureSlider->blockSignals(false);
    m_vignetteSlider->blockSignals(false);
    m_grayscaleCheck->blockSignals(false);
    m_sepiaCheck->blockSignals(false);

    applyEditorAdjustments();
}

void MainWindow::applyEditorAdjustments()
{
    m_editorPreviewImage = makeEditedImage();
    updateEditorPreview();
}

QImage MainWindow::makeEditedImage() const
{
    if (m_editorOriginalImage.isNull()) {
        return {};
    }

    QImage output = m_editorOriginalImage;
    const int brightness = m_brightnessSlider->value();
    const int contrast = m_contrastSlider->value();
    const int saturation = m_saturationSlider->value();
    const int temperature = m_temperatureSlider->value();
    const int vignette = m_vignetteSlider->value();
    const bool grayscale = m_grayscaleCheck->isChecked();
    const bool sepia = m_sepiaCheck->isChecked();

    const double contrastFactor = (100.0 + contrast) / 100.0;
    const double centerX = output.width() / 2.0;
    const double centerY = output.height() / 2.0;
    const double maxDistance = std::sqrt(centerX * centerX + centerY * centerY);

    for (int y = 0; y < output.height(); ++y) {
        QRgb *line = reinterpret_cast<QRgb *>(output.scanLine(y));
        for (int x = 0; x < output.width(); ++x) {
            QColor color = QColor::fromRgb(line[x]);
            int r = color.red();
            int g = color.green();
            int b = color.blue();

            r = qBound(0, static_cast<int>(std::round((r - 127) * contrastFactor + 127 + brightness)), 255);
            g = qBound(0, static_cast<int>(std::round((g - 127) * contrastFactor + 127 + brightness)), 255);
            b = qBound(0, static_cast<int>(std::round((b - 127) * contrastFactor + 127 + brightness)), 255);

            if (grayscale) {
                const int gray = qGray(r, g, b);
                r = gray;
                g = gray;
                b = gray;
            } else if (saturation != 0) {
                QColor adjusted(r, g, b);
                qreal h = adjusted.hslHueF();
                qreal s = adjusted.hslSaturationF();
                qreal l = adjusted.lightnessF();
                if (h >= 0.0) {
                    s = qBound(0.0, s * (1.0 + (saturation / 100.0)), 1.0);
                    adjusted.setHslF(h, s, l);
                    r = adjusted.red();
                    g = adjusted.green();
                    b = adjusted.blue();
                }
            }

            if (temperature != 0) {
                const int delta = static_cast<int>(std::round(temperature * 0.9));
                r = qBound(0, r + delta, 255);
                b = qBound(0, b - delta, 255);
            }

            if (sepia) {
                const int tr = qBound(0, static_cast<int>(0.393 * r + 0.769 * g + 0.189 * b), 255);
                const int tg = qBound(0, static_cast<int>(0.349 * r + 0.686 * g + 0.168 * b), 255);
                const int tb = qBound(0, static_cast<int>(0.272 * r + 0.534 * g + 0.131 * b), 255);
                r = tr;
                g = tg;
                b = tb;
            }

            if (vignette > 0) {
                const double dx = x - centerX;
                const double dy = y - centerY;
                const double distance = std::sqrt(dx * dx + dy * dy);
                const double norm = qBound(0.0, distance / maxDistance, 1.0);
                const double attenuation = 1.0 - (vignette / 100.0) * norm * norm;
                r = qBound(0, static_cast<int>(std::round(r * attenuation)), 255);
                g = qBound(0, static_cast<int>(std::round(g * attenuation)), 255);
                b = qBound(0, static_cast<int>(std::round(b * attenuation)), 255);
            }

            line[x] = qRgb(r, g, b);
        }
    }

    return output;
}

void MainWindow::updateEditorPreview()
{
    if (m_editorPreviewImage.isNull()) {
        m_editorPreviewLabel->setText("Editor preview (select a photo)");
        m_editorPreviewLabel->setPixmap(QPixmap());
        return;
    }

    const QImage displayImage = (m_editorShowOriginal && !m_editorOriginalImage.isNull())
        ? m_editorOriginalImage
        : m_editorPreviewImage;

    m_editorPreviewLabel->setPixmap(QPixmap::fromImage(displayImage).scaled(
        m_editorPreviewLabel->size(),
        Qt::KeepAspectRatio,
        Qt::SmoothTransformation));
}

QString MainWindow::currentPhotoPath() const
{
    auto *item = m_photoList->currentItem();
    return item ? item->data(Qt::UserRole).toString() : QString();
}

QStringList MainWindow::selectedPhotoPaths() const
{
    QStringList paths;
    for (auto *item : m_photoList->selectedItems()) {
        paths.push_back(item->data(Qt::UserRole).toString());
    }
    return paths;
}

QStringList MainWindow::parseTags(const QString &tagText) const
{
    QStringList raw = tagText.split(',', Qt::SkipEmptyParts);
    for (auto &tag : raw) {
        tag = tag.trimmed();
    }
    return raw;
}
