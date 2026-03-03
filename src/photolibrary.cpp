#include "photolibrary.h"

#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QSet>

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

QString PhotoLibrary::metadataFilePath() const
{
    return QDir(m_rootFolder).filePath(".photo_organizer.json");
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
    }
}

void PhotoLibrary::saveMetadata() const
{
    if (m_rootFolder.isEmpty()) {
        return;
    }

    QJsonObject entries;

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
        entries.insert(item.relativePath, obj);
    }

    QJsonObject root;
    root.insert("items", entries);

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
