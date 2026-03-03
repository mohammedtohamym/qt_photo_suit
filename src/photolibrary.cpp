#include "photolibrary.h"

#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QSet>

#include <algorithm>

bool PhotoLibrary::loadFolder(const QString &folderPath)
{
    if (folderPath.isEmpty()) {
        return false;
    }

    const QFileInfo info(folderPath);
    if (!info.exists() || !info.isDir()) {
        return false;
    }

    m_rootFolder = info.absoluteFilePath();
    scanImages();
    loadMetadata();
    rebuildTagIndex();
    saveTagIndexCache();
    return true;
}

QString PhotoLibrary::rootFolder() const
{
    return m_rootFolder;
}

QVector<PhotoItem> PhotoLibrary::allItems() const
{
    return m_items;
}

QVector<PhotoItem> PhotoLibrary::filteredItems(const QString &nameQuery,
                                               const QString &tagQuery,
                                               bool favoritesOnly) const
{
    const QString nameNeedle = nameQuery.trimmed().toLower();
    const QString tagNeedle = tagQuery.trimmed().toLower();

    QVector<PhotoItem> filtered;
    filtered.reserve(m_items.size());

    for (const auto &item : m_items) {
        if (favoritesOnly && !item.favorite) {
            continue;
        }

        if (!nameNeedle.isEmpty()) {
            const QString fileName = QFileInfo(item.absolutePath).fileName().toLower();
            if (!fileName.contains(nameNeedle)) {
                continue;
            }
        }

        if (!tagNeedle.isEmpty()) {
            bool hasTag = false;
            for (const auto &tag : item.tags) {
                if (tag.toLower().contains(tagNeedle)) {
                    hasTag = true;
                    break;
                }
            }
            if (!hasTag) {
                continue;
            }
        }

        filtered.push_back(item);
    }

    return filtered;
}

bool PhotoLibrary::contains(const QString &absolutePath) const
{
    return m_indexByPath.contains(absolutePath);
}

PhotoItem PhotoLibrary::byAbsolutePath(const QString &absolutePath) const
{
    const auto indexIt = m_indexByPath.constFind(absolutePath);
    if (indexIt == m_indexByPath.constEnd()) {
        return {};
    }
    return m_items.at(indexIt.value());
}

void PhotoLibrary::updateTags(const QString &absolutePath, const QStringList &tags)
{
    const auto indexIt = m_indexByPath.constFind(absolutePath);
    if (indexIt == m_indexByPath.constEnd()) {
        return;
    }

    m_items[indexIt.value()].tags = normalizeTags(tags);
    rebuildTagIndex();
    saveTagIndexCache();
    saveMetadata();
}

void PhotoLibrary::updateFavorite(const QString &absolutePath, bool favorite)
{
    const auto indexIt = m_indexByPath.constFind(absolutePath);
    if (indexIt == m_indexByPath.constEnd()) {
        return;
    }

    m_items[indexIt.value()].favorite = favorite;
    saveMetadata();
}

void PhotoLibrary::updateRating(const QString &absolutePath, int rating)
{
    const auto indexIt = m_indexByPath.constFind(absolutePath);
    if (indexIt == m_indexByPath.constEnd()) {
        return;
    }

    m_items[indexIt.value()].rating = qBound(0, rating, 5);
    saveMetadata();
}

QVector<PhotoItem> PhotoLibrary::indexedSearchByTagExact(const QString &tag) const
{
    QVector<PhotoItem> result;
    const QString key = tag.trimmed().toLower();
    if (key.isEmpty() || !m_tagIndex.contains(key)) {
        return result;
    }

    const QStringList paths = m_tagIndex.value(key);
    result.reserve(paths.size());
    for (const auto &path : paths) {
        const auto indexIt = m_indexByPath.constFind(path);
        if (indexIt != m_indexByPath.constEnd()) {
            result.push_back(m_items.at(indexIt.value()));
        }
    }
    return result;
}

void PhotoLibrary::setEditRecipe(const QString &absolutePath, const QJsonObject &recipe)
{
    const QString cleanPath = QDir::cleanPath(absolutePath);
    if (!contains(cleanPath)) {
        return;
    }

    if (recipe.isEmpty()) {
        m_editRecipes.remove(cleanPath);
    } else {
        m_editRecipes.insert(cleanPath, recipe);
    }
    saveMetadata();
}

QJsonObject PhotoLibrary::editRecipeForPhoto(const QString &absolutePath) const
{
    return m_editRecipes.value(QDir::cleanPath(absolutePath));
}

QStringList PhotoLibrary::albumNames() const
{
    auto names = m_albums.keys();
    names.sort(Qt::CaseInsensitive);
    return names;
}

QStringList PhotoLibrary::photosForAlbum(const QString &albumName) const
{
    return m_albums.value(albumName);
}

bool PhotoLibrary::createAlbum(const QString &albumName)
{
    const QString trimmed = albumName.trimmed();
    if (trimmed.isEmpty() || m_albums.contains(trimmed)) {
        return false;
    }

    m_albums.insert(trimmed, {});
    saveMetadata();
    return true;
}

void PhotoLibrary::deleteAlbum(const QString &albumName)
{
    if (!m_albums.remove(albumName)) {
        return;
    }
    saveMetadata();
}

void PhotoLibrary::addPhotosToAlbum(const QString &albumName, const QStringList &absolutePaths)
{
    if (!m_albums.contains(albumName)) {
        return;
    }

    QStringList entries = m_albums.value(albumName);
    QSet<QString> dedupe(entries.begin(), entries.end());
    for (const auto &path : absolutePaths) {
        if (!contains(path)) {
            continue;
        }
        if (dedupe.contains(path)) {
            continue;
        }
        dedupe.insert(path);
        entries.push_back(path);
    }

    m_albums[albumName] = entries;
    saveMetadata();
}

void PhotoLibrary::removePhotosFromAlbum(const QString &albumName, const QStringList &absolutePaths)
{
    if (!m_albums.contains(albumName)) {
        return;
    }

    QSet<QString> removeSet(absolutePaths.begin(), absolutePaths.end());
    auto entries = m_albums.value(albumName);
    entries.erase(std::remove_if(entries.begin(), entries.end(), [&removeSet](const QString &path) {
        return removeSet.contains(path);
    }), entries.end());

    m_albums[albumName] = entries;
    saveMetadata();
}

QString PhotoLibrary::metadataFilePath() const
{
    return QDir(m_rootFolder).filePath(".photo_organizer.json");
}

QString PhotoLibrary::tagIndexFilePath() const
{
    return QDir(m_rootFolder).filePath(".photo_tag_index.json");
}

void PhotoLibrary::rebuildTagIndex()
{
    m_tagIndex.clear();
    for (const auto &item : m_items) {
        for (const auto &tag : item.tags) {
            const QString key = tag.trimmed().toLower();
            if (key.isEmpty()) {
                continue;
            }
            m_tagIndex[key].push_back(item.absolutePath);
        }
    }
}

void PhotoLibrary::saveTagIndexCache() const
{
    if (m_rootFolder.isEmpty()) {
        return;
    }

    QJsonObject root;
    QJsonObject buckets;
    for (auto it = m_tagIndex.begin(); it != m_tagIndex.end(); ++it) {
        QJsonArray arr;
        for (const auto &path : it.value()) {
            arr.push_back(QDir(m_rootFolder).relativeFilePath(path));
        }
        buckets.insert(it.key(), arr);
    }
    root.insert("tags", buckets);
    root.insert("tagCount", buckets.keys().size());
    root.insert("photoCount", m_items.size());

    QFile file(tagIndexFilePath());
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        return;
    }
    file.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    file.close();
}

void PhotoLibrary::scanImages()
{
    m_items.clear();
    m_indexByPath.clear();

    QDirIterator it(m_rootFolder,
                    QDir::Files,
                    QDirIterator::Subdirectories);

    while (it.hasNext()) {
        const QString absolutePath = QDir::cleanPath(it.next());
        const QFileInfo fileInfo(absolutePath);

        if (!isImageFile(fileInfo.fileName())) {
            continue;
        }

        PhotoItem item;
        item.absolutePath = absolutePath;
        item.relativePath = QDir(m_rootFolder).relativeFilePath(absolutePath);
        m_indexByPath.insert(item.absolutePath, m_items.size());
        m_items.push_back(item);
    }
}

void PhotoLibrary::loadMetadata()
{
    m_albums.clear();
    m_editRecipes.clear();

    QFile file(metadataFilePath());
    if (!file.exists()) {
        return;
    }

    if (!file.open(QIODevice::ReadOnly)) {
        return;
    }

    const auto doc = QJsonDocument::fromJson(file.readAll());
    file.close();

    if (!doc.isObject()) {
        return;
    }

    const auto rootObj = doc.object();
    const auto entriesObj = rootObj.value("items").toObject();

    for (auto it = entriesObj.begin(); it != entriesObj.end(); ++it) {
        const QString relativePath = it.key();
        const QString absolutePath = QDir(m_rootFolder).absoluteFilePath(relativePath);

        const auto indexIt = m_indexByPath.constFind(QDir::cleanPath(absolutePath));
        if (indexIt == m_indexByPath.constEnd()) {
            continue;
        }

        const auto obj = it.value().toObject();
        auto &item = m_items[indexIt.value()];

        QStringList tags;
        for (const auto &value : obj.value("tags").toArray()) {
            tags.push_back(value.toString());
        }

        item.tags = normalizeTags(tags);
        item.favorite = obj.value("favorite").toBool(false);
        item.rating = qBound(0, obj.value("rating").toInt(0), 5);

        if (obj.value("recipe").isObject()) {
            m_editRecipes.insert(item.absolutePath, obj.value("recipe").toObject());
        }
    }

    const auto albumsObj = rootObj.value("albums").toObject();
    for (auto it = albumsObj.begin(); it != albumsObj.end(); ++it) {
        const QString albumName = it.key().trimmed();
        if (albumName.isEmpty()) {
            continue;
        }

        QStringList albumPaths;
        QSet<QString> dedupe;
        const auto array = it.value().toArray();
        for (const auto &value : array) {
            const QString relativePath = value.toString();
            const QString absolutePath = QDir::cleanPath(QDir(m_rootFolder).absoluteFilePath(relativePath));
            if (!contains(absolutePath) || dedupe.contains(absolutePath)) {
                continue;
            }
            dedupe.insert(absolutePath);
            albumPaths.push_back(absolutePath);
        }

        m_albums.insert(albumName, albumPaths);
    }
}

void PhotoLibrary::saveMetadata() const
{
    if (m_rootFolder.isEmpty()) {
        return;
    }

    QJsonObject entries;
    QJsonObject albums;

    for (const auto &item : m_items) {
        if (item.tags.isEmpty() && !item.favorite && item.rating == 0) {
            continue;
        }

        QJsonObject obj;
        QJsonArray tagsArray;
        for (const auto &tag : item.tags) {
            tagsArray.push_back(tag);
        }

        obj.insert("tags", tagsArray);
        obj.insert("favorite", item.favorite);
        obj.insert("rating", item.rating);

        const auto recipeIt = m_editRecipes.constFind(item.absolutePath);
        if (recipeIt != m_editRecipes.constEnd() && !recipeIt.value().isEmpty()) {
            obj.insert("recipe", recipeIt.value());
        }
        entries.insert(item.relativePath, obj);
    }

    for (auto it = m_albums.begin(); it != m_albums.end(); ++it) {
        const QString albumName = it.key().trimmed();
        if (albumName.isEmpty()) {
            continue;
        }

        QJsonArray albumEntries;
        QSet<QString> dedupe;
        for (const auto &absolutePath : it.value()) {
            const QString cleanPath = QDir::cleanPath(absolutePath);
            if (!contains(cleanPath) || dedupe.contains(cleanPath)) {
                continue;
            }
            dedupe.insert(cleanPath);
            albumEntries.push_back(QDir(m_rootFolder).relativeFilePath(cleanPath));
        }

        albums.insert(albumName, albumEntries);
    }

    QJsonObject root;
    root.insert("items", entries);
    root.insert("albums", albums);

    QFile file(metadataFilePath());
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        return;
    }

    file.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    file.close();
}

bool PhotoLibrary::isImageFile(const QString &fileName)
{
    static const QSet<QString> extensions = {
        "jpg", "jpeg", "png", "bmp", "gif", "webp", "tiff", "tif", "heic"
    };

    const auto suffix = QFileInfo(fileName).suffix().toLower();
    return extensions.contains(suffix);
}

QStringList PhotoLibrary::normalizeTags(const QStringList &rawTags)
{
    QSet<QString> dedupe;
    QStringList normalized;

    for (const auto &tag : rawTags) {
        const QString cleaned = tag.trimmed();
        if (cleaned.isEmpty()) {
            continue;
        }

        const QString key = cleaned.toLower();
        if (dedupe.contains(key)) {
            continue;
        }

        dedupe.insert(key);
        normalized.push_back(cleaned);
    }

    normalized.sort(Qt::CaseInsensitive);
    return normalized;
}
