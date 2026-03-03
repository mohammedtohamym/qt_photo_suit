#include "mainwindow.h"

#include <QAction>
#include <QApplication>
#include <QClipboard>
#include <QDesktopServices>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QFile>
#include <QHBoxLayout>
#include <QListWidgetItem>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QPixmap>
#include <QRandomGenerator>
#include <QShortcut>
#include <QSplitter>
#include <QSettings>
#include <QStandardPaths>
#include <QStatusBar>
#include <QStyle>
#include <QTimer>
#include <QTextStream>
#include <QToolBar>
#include <QToolButton>
#include <QUrl>
#include <QVBoxLayout>

#include <algorithm>
#include <QSet>

MainWindow::MainWindow()
{
    setupUi();
    setupMenu();
    setupToolbar();
    setupConnections();
    statusBar()->showMessage("Open a folder to start organizing photos.");

    const QString lastFolder = QSettings("PhotoOrganizerQt", "PhotoOrganizerQt").value("lastFolder").toString();
    if (!lastFolder.isEmpty() && QFileInfo(lastFolder).isDir()) {
        openFolderPath(lastFolder);
    }
}

void MainWindow::setupUi()
{
    setWindowTitle("Photo Organizer (Qt)[*]");
    resize(1200, 760);
    setStyleSheet(
        "QMainWindow {"
        "  background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #0b1020, stop:1 #111827);"
        "}"
        "QWidget { color: #e6edf6; font-size: 13px; }"
        "QLabel { color: #c9d6ea; }"
        "QLineEdit, QSpinBox, QComboBox, QListWidget {"
        "  background: rgba(17, 24, 39, 0.92);"
        "  border: 1px solid #2f3f5c;"
        "  border-radius: 10px;"
        "  padding: 7px;"
        "  color: #f8fbff;"
        "}"
        "QLineEdit:focus, QSpinBox:focus, QComboBox:focus { border: 1px solid #60a5fa; }"
        "QPushButton {"
        "  background: #1d4ed8;"
        "  border: none;"
        "  border-radius: 10px;"
        "  padding: 8px 12px;"
        "  color: #ffffff;"
        "  font-weight: 600;"
        "}"
        "QPushButton:hover { background: #2563eb; }"
        "QPushButton:disabled { background: #334155; color: #94a3b8; }"
        "QToolButton {"
        "  background: rgba(17, 24, 39, 0.8);"
        "  border: 1px solid #2f3f5c;"
        "  border-radius: 10px;"
        "  padding: 6px 10px;"
        "  color: #e2e8f0;"
        "}"
        "QToolButton::menu-indicator { image: none; width: 0px; }"
        "QCheckBox::indicator { width: 16px; height: 16px; }"
        "QMenuBar, QMenu { background: #0f172a; color: #f8fafc; }"
        "QMenu::item:selected { background: #1e3a8a; }"
        "QStatusBar { background: #0b1220; color: #94a3b8; }"
    );

    auto *central = new QWidget(this);
    auto *rootLayout = new QVBoxLayout(central);
    rootLayout->setContentsMargins(18, 18, 18, 18);
    rootLayout->setSpacing(12);

    auto *filterCard = new QWidget(this);
    auto *filterCardLayout = new QVBoxLayout(filterCard);
    filterCardLayout->setContentsMargins(12, 12, 12, 12);
    filterCardLayout->setSpacing(8);
    filterCard->setStyleSheet("background: rgba(15, 23, 42, 0.82); border: 1px solid #23324a; border-radius: 14px;");

    auto *searchRow = new QHBoxLayout();
    searchRow->setSpacing(8);
    m_nameFilter = new QLineEdit(this);
    m_nameFilter->setPlaceholderText("Search photos by name...");

    m_tagFilter = new QLineEdit(this);
    m_tagFilter->setPlaceholderText("Filter by tag...");

    m_favoritesOnly = new QCheckBox("Favorites only", this);
    searchRow->addWidget(m_nameFilter, 3);
    searchRow->addWidget(m_tagFilter, 2);
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
    m_thumbSizeSlider->setRange(80, 220);
    m_thumbSizeSlider->setValue(140);
    m_thumbSizeSlider->setMaximumWidth(140);

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
    m_photoList->setIconSize(QSize(140, 140));
    m_photoList->setResizeMode(QListWidget::Adjust);
    m_photoList->setMovement(QListView::Static);
    m_photoList->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_photoList->setSpacing(10);
    m_photoList->setWordWrap(true);

    auto *detailsPanel = new QWidget(this);
    auto *detailsLayout = new QVBoxLayout(detailsPanel);
    detailsLayout->setContentsMargins(12, 12, 12, 12);
    detailsLayout->setSpacing(10);
    detailsPanel->setStyleSheet("background: rgba(15, 23, 42, 0.82); border: 1px solid #23324a; border-radius: 14px;");

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

    m_saveButton = new QPushButton("Save metadata", this);
    m_saveButton->setText("Save Changes");
    m_openPhotoButton = new QPushButton("Open selected photo", this);
    m_openLocationButton = new QPushButton("Open location", this);
    m_copyPathButton = new QPushButton("Copy selected path", this);
    m_slideshowButton = new QPushButton("Slideshow");
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

    splitter->addWidget(m_photoList);
    splitter->addWidget(detailsPanel);
    splitter->setStretchFactor(0, 2);
    splitter->setStretchFactor(1, 1);

    rootLayout->addWidget(filterCard);
    rootLayout->addWidget(splitter, 1);
    setCentralWidget(central);

    connect(m_openLocationButton, &QPushButton::clicked, this, [this]() {
        const QString path = currentPhotoPath();
        if (path.isEmpty()) {
            return;
        }

        QDesktopServices::openUrl(QUrl::fromLocalFile(QFileInfo(path).absolutePath()));
    });

    clearDetails();
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
    connect(m_sortCombo, &QComboBox::currentTextChanged, this, [this]() {
        refreshList();
    });
    connect(m_viewModeCombo, &QComboBox::currentTextChanged, this, [this]() {
        refreshList();
    });
    connect(m_minRatingFilter, &QSpinBox::valueChanged, this, [this](int) {
        refreshList();
    });
    connect(m_thumbSizeSlider, &QSlider::valueChanged, this, [this](int value) {
        m_photoList->setIconSize(QSize(value, value));
        refreshList();
    });
    connect(m_favoritesOnly, &QCheckBox::toggled, this, [this]() {
        refreshList();
    });

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

        const QString path = item->data(Qt::UserRole).toString();
        QDesktopServices::openUrl(QUrl::fromLocalFile(path));
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

            QSet<QString> dedupe;
            QStringList normalized;
            for (const auto &tag : merged) {
                const QString clean = tag.trimmed();
                if (clean.isEmpty()) {
                    continue;
                }
                const QString key = clean.toLower();
                if (dedupe.contains(key)) {
                    continue;
                }
                dedupe.insert(key);
                normalized.push_back(clean);
            }

            m_library.updateTags(path, normalized);
        }

        refreshList();
        loadSelectionDetails();
        statusBar()->showMessage("Bulk tags applied.", 2500);
    });

    connect(m_bulkFavoriteAction, &QAction::triggered, this, [this]() {
        const QStringList selected = selectedPhotoPaths();
        if (selected.isEmpty()) {
            return;
        }

        const bool favoriteValue = m_favoriteCheck->isChecked();
        for (const auto &path : selected) {
            m_library.updateFavorite(path, favoriteValue);
        }

        refreshList();
        loadSelectionDetails();
        statusBar()->showMessage("Bulk favorite applied.", 2500);
    });

    connect(m_bulkRatingAction, &QAction::triggered, this, [this]() {
        const QStringList selected = selectedPhotoPaths();
        if (selected.isEmpty()) {
            return;
        }

        const int ratingValue = m_ratingSpin->value();
        for (const auto &path : selected) {
            m_library.updateRating(path, ratingValue);
        }

        refreshList();
        loadSelectionDetails();
        statusBar()->showMessage("Bulk rating applied.", 2500);
    });

    connect(m_openPhotoButton, &QPushButton::clicked, this, [this]() {
        const QString path = currentPhotoPath();
        if (path.isEmpty()) {
            return;
        }
        QDesktopServices::openUrl(QUrl::fromLocalFile(path));
    });

    connect(m_copyPathButton, &QPushButton::clicked, this, [this]() {
        const QString path = currentPhotoPath();
        if (path.isEmpty()) {
            return;
        }
        QApplication::clipboard()->setText(path);
        statusBar()->showMessage("Path copied to clipboard.", 2000);
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
        const int current = m_photoList->currentRow();
        const int next = (current + 1) % m_photoList->count();
        m_photoList->setCurrentRow(next);
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

    if (selected.isEmpty()) {
        return;
    }

    openFolderPath(selected);
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
        const QString fileName = QFileInfo(item.absolutePath).fileName();
        const QString tags = item.tags.join("|");
        QString escapedFileName = fileName;
        QString escapedRelative = item.relativePath;
        QString escapedTags = tags;
        escapedFileName.replace('"', "\"\"");
        escapedRelative.replace('"', "\"\"");
        escapedTags.replace('"', "\"\"");

        out << '"' << escapedFileName << "\",";
        out << '"' << escapedRelative << "\",";
        out << (item.favorite ? "true" : "false") << ',';
        out << item.rating << ',';
        out << '"' << escapedTags << "\"\n";
    }

    file.close();
    statusBar()->showMessage("CSV export complete.", 2500);
}

void MainWindow::loadSelectionDetails()
{
    const QString path = currentPhotoPath();
    if (path.isEmpty()) {
        clearDetails();
        return;
    }

    if (!m_library.contains(path)) {
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
}

void MainWindow::setUnsavedChanges(bool value)
{
    m_hasUnsavedChanges = value;
    setWindowModified(value);
    if (value) {
        statusBar()->showMessage("Unsaved metadata changes.");
    }
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
