#pragma once

#include <QHash>
#include <QJsonObject>
#include <QString>
#include <QStringList>
#include <QVector>

struct PhotoItem {
    QString absolutePath;
    QString relativePath;
    QStringList tags;
    bool favorite = false;
    int rating = 0;
};

class PhotoLibrary {
public:
    bool loadFolder(const QString &folderPath);

    QString rootFolder() const;
    QVector<PhotoItem> allItems() const;
    QVector<PhotoItem> filteredItems(const QString &nameQuery,
                                     const QString &tagQuery,
                                     bool favoritesOnly) const;

    bool contains(const QString &absolutePath) const;
    PhotoItem byAbsolutePath(const QString &absolutePath) const;

    void updateTags(const QString &absolutePath, const QStringList &tags);
    void updateFavorite(const QString &absolutePath, bool favorite);
    void updateRating(const QString &absolutePath, int rating);

private:
    QString metadataFilePath() const;
    void scanImages();
    void loadMetadata();
    void saveMetadata() const;

    static bool isImageFile(const QString &fileName);
    static QStringList normalizeTags(const QStringList &rawTags);

private:
    QString m_rootFolder;
    QVector<PhotoItem> m_items;
    QHash<QString, int> m_indexByPath;
};
