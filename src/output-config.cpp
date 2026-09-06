/** @fileoverview Screenshot destination and filename pattern from the user's
 *  INI config. */
#include "output-config.hpp"

#include <QDateTime>
#include <QDir>
#include <QFileInfoList>
#include <QImageReader>
#include <QSettings>
#include <QStandardPaths>
#include <QStringList>

#include <algorithm>

OutputConfig loadOutputConfig(const QString &filePath) {
  OutputConfig config;
  QSettings settings(filePath, QSettings::IniFormat);
  QString directory =
      settings.value(QStringLiteral("output/directory")).toString().trimmed();
  if (directory == QStringLiteral("~"))
    directory = QDir::homePath();
  else if (directory.startsWith(QStringLiteral("~/")))
    directory = QDir::homePath() + directory.mid(1);
  if (!directory.isEmpty())
    config.directory = directory;
  const QString filename =
      settings.value(QStringLiteral("output/filename")).toString().trimmed();
  if (!filename.isEmpty())
    config.filename = filename;
  return config;
}

QString formatScreenshotFilename(const QString &pattern, const QDateTime &when,
                                 const QString &appSlug) {
  QString name = pattern;
  if (appSlug.isEmpty()) {
    // Drop the token and the separator that introduced it, so the default
    // pattern does not leave a dangling dash on a capture with no app.
    for (const char *joined : {"-{app}", "_{app}", " {app}", "{app}-",
                               "{app}_", "{app} ", "{app}"})
      name.replace(QLatin1String(joined), QString());
  } else {
    name.replace(QStringLiteral("{app}"), appSlug);
  }
  name.replace(QStringLiteral("{date}"),
               when.toString(QStringLiteral("yyyy-MM-dd")));
  name.replace(QStringLiteral("{time}"),
               when.toString(QStringLiteral("HH-mm-ss")));
  // Filenames only: a slash would silently change the destination, and a
  // leading dot hides the file.
  name.replace(QLatin1Char('/'), QLatin1Char('-'));
  while (!name.isEmpty() &&
         QStringLiteral(". -_").contains(name.front()))
    name.remove(0, 1);
  name = name.trimmed();
  if (name.endsWith(QStringLiteral(".png"), Qt::CaseInsensitive))
    name.chop(4);
  if (name.isEmpty())
    name = QStringLiteral("screenshot-") +
           when.toString(QStringLiteral("yyyy-MM-dd_HH-mm-ss"));
  return name + QStringLiteral(".png");
}

QString defaultConfigPath() {
  return QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) +
         QStringLiteral("/omasnap/omasnap.conf");
}

QString screenshotDirectory() {
  const OutputConfig config = loadOutputConfig(defaultConfigPath());
  QString root = qEnvironmentVariable("OMASNAP_SCREENSHOT_DIR");
  if (root.isEmpty())
    root = config.directory;
  if (root.isEmpty())
    root = QDir(QStandardPaths::writableLocation(QStandardPaths::PicturesLocation))
              .filePath(QStringLiteral("Screenshots"));
  return root;
}

QVector<EditableImage> listEditableImages(const QString &directory) {
  QStringList filters;
  const QList<QByteArray> formats = QImageReader::supportedImageFormats();
  for (const QByteArray &format : formats)
    filters << QStringLiteral("*.") + QString::fromLatin1(format.toLower());
  const QFileInfoList entries =
      QDir(directory).entryInfoList(filters, QDir::Files);
  QVector<EditableImage> images;
  images.reserve(entries.size());
  for (const QFileInfo &entry : entries) {
    EditableImage image;
    image.path = entry.absoluteFilePath();
    image.name = entry.fileName();
    image.stampMs = entry.lastModified().toMSecsSinceEpoch();
    images.append(image);
  }
  std::sort(images.begin(), images.end(),
            [](const EditableImage &a, const EditableImage &b) {
              if (a.stampMs != b.stampMs)
                return a.stampMs > b.stampMs;
              return a.name < b.name;
            });
  return images;
}
