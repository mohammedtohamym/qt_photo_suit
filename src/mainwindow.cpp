#include "mainwindow.h"

#include <QAction>
#include <QDesktopServices>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QListWidgetItem>
#include <QMenuBar>
#include <QMessageBox>
#include <QPixmap>
#include <QShortcut>
#include <QSplitter>
#include <QStandardPaths>
#include <QStatusBar>
#include <QStyle>
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
}

void MainWindow::setupUi()
{
    setWindowTitle("Photo Organizer (Qt)");
    resize(1200, 760);
    setStyleSheet(
        "QMainWindow { background: #111827; }"
        "QWidget { color: #e5e7eb; font-size: 13px; }"
        "QLabel { color: #d1d5db; }"
        "QLineEdit, QSpinBox, QListWidget {"
        "  background: #1f2937; border: 1px solid #374151; border-radius: 8px;"
        "  padding: 6px; color: #f9fafb;"
        "}"
        "QLineEdit:focus, QSpinBox:focus { border: 1px solid #60a5fa; }"
        "QPushButton {"
        "  background: #2563eb; border: none; border-radius: 8px;"
        "  padding: 8px 12px; color: white; font-weight: 600;"
        "}"
        "QPushButton:hover { background: #3b82f6; }"
        "QPushButton:disabled { background: #374151; color: #9ca3af; }"
        "QCheckBox::indicator { width: 16px; height: 16px; }"
        "QMenuBar, QMenu { background: #111827; color: #f9fafb; }"
        "QMenu::item:selected { background: #1d4ed8; }"
        "QStatusBar { background: #0b1220; color: #9ca3af; }"
    );

    auto *central = new QWidget(this);
    auto *rootLayout = new QVBoxLayout(central);
    rootLayout->setContentsMargins(14, 14, 14, 14);
    rootLayout->setSpacing(10);

    auto *filtersLayout = new QHBoxLayout();
    m_nameFilter = new QLineEdit(this);
    m_nameFilter->setPlaceholderText("Search by file name...");

    m_tagFilter = new QLineEdit(this);
    m_tagFilter->setPlaceholderText("Filter by tag...");

    m_favoritesOnly = new QCheckBox("Favorites only", this);
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

    filtersLayout->addWidget(new QLabel("Name:", this));
    filtersLayout->addWidget(m_nameFilter, 2);
    filtersLayout->addWidget(new QLabel("Tag:", this));
    filtersLayout->addWidget(m_tagFilter, 2);
    filtersLayout->addWidget(m_favoritesOnly);
    filtersLayout->addWidget(new QLabel("View:", this));
    filtersLayout->addWidget(m_viewModeCombo);
    filtersLayout->addWidget(new QLabel("Sort:", this));
    filtersLayout->addWidget(m_sortCombo);
    filtersLayout->addWidget(new QLabel("Rating:", this));
    filtersLayout->addWidget(m_minRatingFilter);
    filtersLayout->addWidget(new QLabel("Thumb:", this));
    filtersLayout->addWidget(m_thumbSizeSlider);

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
    detailsLayout->setContentsMargins(8, 8, 8, 8);
    detailsLayout->setSpacing(8);

    m_previewLabel = new QLabel(this);
    m_previewLabel->setMinimumSize(400, 300);
    m_previewLabel->setAlignment(Qt::AlignCenter);
    m_previewLabel->setText("No photo selected");
    m_previewLabel->setFrameStyle(QFrame::StyledPanel);

    m_pathLabel = new QLabel(this);
    m_pathLabel->setWordWrap(true);

    m_tagsEdit = new QLineEdit(this);
    m_tagsEdit->setPlaceholderText("comma,separated,tags");
    m_bulkTagsEdit = new QLineEdit(this);
    m_bulkTagsEdit->setPlaceholderText("bulk tags to add to selected photos");

    m_favoriteCheck = new QCheckBox("Favorite", this);

    m_ratingSpin = new QSpinBox(this);
    m_ratingSpin->setRange(0, 5);
    m_ratingSpin->setPrefix("Rating: ");

    m_saveButton = new QPushButton("Save metadata", this);
    m_bulkAddTagsButton = new QPushButton("Bulk add tags to selected", this);
    m_bulkFavoriteButton = new QPushButton("Apply favorite to selected", this);
    m_bulkRatingButton = new QPushButton("Apply rating to selected", this);
    m_openPhotoButton = new QPushButton("Open selected photo", this);
    auto *openInExplorerButton = new QPushButton("Open photo location", this);

    detailsLayout->addWidget(m_previewLabel, 1);
    detailsLayout->addWidget(new QLabel("Path:", this));
    detailsLayout->addWidget(m_pathLabel);
    detailsLayout->addWidget(new QLabel("Tags:", this));
    detailsLayout->addWidget(m_tagsEdit);
    detailsLayout->addWidget(new QLabel("Bulk Tags:", this));
    detailsLayout->addWidget(m_bulkTagsEdit);
    detailsLayout->addWidget(m_bulkAddTagsButton);
    detailsLayout->addWidget(m_favoriteCheck);
    detailsLayout->addWidget(m_bulkFavoriteButton);
    detailsLayout->addWidget(m_ratingSpin);
    detailsLayout->addWidget(m_bulkRatingButton);
    detailsLayout->addWidget(m_openPhotoButton);
    detailsLayout->addWidget(m_saveButton);
    detailsLayout->addWidget(openInExplorerButton);
    detailsLayout->addStretch();

    splitter->addWidget(m_photoList);
    splitter->addWidget(detailsPanel);
    splitter->setStretchFactor(0, 2);
    splitter->setStretchFactor(1, 1);

    rootLayout->addLayout(filtersLayout);
    rootLayout->addWidget(splitter, 1);
    setCentralWidget(central);

    connect(openInExplorerButton, &QPushButton::clicked, this, [this]() {
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
    fileMenu->addSeparator();
    fileMenu->addAction(quitAction);
}

void MainWindow::setupToolbar()
{
    auto *toolbar = addToolBar("Quick Actions");
    toolbar->setMovable(false);

    auto *openAction = toolbar->addAction(style()->standardIcon(QStyle::SP_DirOpenIcon), "Open");
    auto *refreshAction = toolbar->addAction(style()->standardIcon(QStyle::SP_BrowserReload), "Refresh");
    auto *clearFiltersAction = toolbar->addAction(style()->standardIcon(QStyle::SP_DialogResetButton), "Clear Filters");

    connect(openAction, &QAction::triggered, this, &MainWindow::openFolder);
    connect(refreshAction, &QAction::triggered, this, &MainWindow::refreshList);
    connect(clearFiltersAction, &QAction::triggered, this, [this]() {
        m_nameFilter->clear();
        m_tagFilter->clear();
        m_favoritesOnly->setChecked(false);
        refreshList();
    });
}

void MainWindow::setupConnections()
{
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

    connect(m_photoList, &QListWidget::itemDoubleClicked, this, [this](QListWidgetItem *item) {
        if (!item) {
            return;
        }

        const QString path = item->data(Qt::UserRole).toString();
        QDesktopServices::openUrl(QUrl::fromLocalFile(path));
    });

    connect(m_saveButton, &QPushButton::clicked, this, [this]() {
        const QString path = currentPhotoPath();
        if (path.isEmpty()) {
            return;
        }

        m_library.updateTags(path, parseTags(m_tagsEdit->text()));
        m_library.updateFavorite(path, m_favoriteCheck->isChecked());
        m_library.updateRating(path, m_ratingSpin->value());

        refreshList();
        statusBar()->showMessage("Photo metadata saved.", 2500);
    });

    connect(m_bulkAddTagsButton, &QPushButton::clicked, this, [this]() {
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

    connect(m_bulkFavoriteButton, &QPushButton::clicked, this, [this]() {
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

    connect(m_bulkRatingButton, &QPushButton::clicked, this, [this]() {
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

    if (!m_library.loadFolder(selected)) {
        QMessageBox::warning(this, "Failed", "Could not open the selected folder.");
        return;
    }

    m_nameFilter->clear();
    m_tagFilter->clear();
    m_favoritesOnly->setChecked(false);

    refreshList();
    statusBar()->showMessage(QString("Loaded %1 photos from %2")
                                 .arg(m_library.allItems().size())
                                 .arg(selected),
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
    m_saveButton->setEnabled(true);
}

void MainWindow::clearDetails()
{
    m_previewLabel->setText("No photo selected");
    m_previewLabel->setPixmap(QPixmap());
    m_pathLabel->setText("-");
    m_tagsEdit->clear();
    m_favoriteCheck->setChecked(false);
    m_ratingSpin->setValue(0);
    m_saveButton->setEnabled(false);
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
