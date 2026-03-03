#pragma once

#include "photolibrary.h"

#include <QCheckBox>
#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMainWindow>
#include <QPushButton>
#include <QSlider>
#include <QSpinBox>

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow();

private:
    void setupUi();
    void setupMenu();
    void setupToolbar();
    void setupConnections();

    void openFolder();
    void refreshList();
    void loadSelectionDetails();
    void clearDetails();

    QString currentPhotoPath() const;
    QStringList selectedPhotoPaths() const;
    QStringList parseTags(const QString &tagText) const;

private:
    PhotoLibrary m_library;

    QLineEdit *m_nameFilter = nullptr;
    QLineEdit *m_tagFilter = nullptr;
    QCheckBox *m_favoritesOnly = nullptr;
    QComboBox *m_sortCombo = nullptr;
    QComboBox *m_viewModeCombo = nullptr;
    QSlider *m_thumbSizeSlider = nullptr;
    QSpinBox *m_minRatingFilter = nullptr;

    QListWidget *m_photoList = nullptr;

    QLabel *m_previewLabel = nullptr;
    QLabel *m_pathLabel = nullptr;
    QLineEdit *m_tagsEdit = nullptr;
    QLineEdit *m_bulkTagsEdit = nullptr;
    QCheckBox *m_favoriteCheck = nullptr;
    QSpinBox *m_ratingSpin = nullptr;
    QPushButton *m_saveButton = nullptr;
    QPushButton *m_openPhotoButton = nullptr;
    QPushButton *m_copyPathButton = nullptr;
    QPushButton *m_bulkAddTagsButton = nullptr;
    QPushButton *m_bulkFavoriteButton = nullptr;
    QPushButton *m_bulkRatingButton = nullptr;
};
