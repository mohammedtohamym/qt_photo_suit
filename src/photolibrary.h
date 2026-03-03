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
    QVector<PhotoItem> indexedSearchByTagExact(const QString &tag) const;

    void setEditRecipe(const QString &absolutePath, const QJsonObject &recipe);
    QJsonObject editRecipeForPhoto(const QString &absolutePath) const;

    QStringList albumNames() const;
    QStringList photosForAlbum(const QString &albumName) const;
    bool createAlbum(const QString &albumName);
    void deleteAlbum(const QString &albumName);
    void addPhotosToAlbum(const QString &albumName, const QStringList &absolutePaths);
    void removePhotosFromAlbum(const QString &albumName, const QStringList &absolutePaths);

private:
    QString metadataFilePath() const;
    QString tagIndexFilePath() const;
    void scanImages();
    void loadMetadata();
    void saveMetadata() const;
    void rebuildTagIndex();
    void saveTagIndexCache() const;

    static bool isImageFile(const QString &fileName);
    static QStringList normalizeTags(const QStringList &rawTags);

private:
    QString m_rootFolder;
    QVector<PhotoItem> m_items;
    QHash<QString, int> m_indexByPath;
    QHash<QString, QStringList> m_albums;
    QHash<QString, QStringList> m_tagIndex;
    QHash<QString, QJsonObject> m_editRecipes;
};
