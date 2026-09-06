/** @fileoverview Screenshot destination and filename pattern from the user's
 *  INI config. Every key is optional; defaults match the built-in behavior. */
#pragma once

#include <QDateTime>
#include <QString>
#include <QVector>

/// One saved image the select overlay's open-image picker can reopen.
struct EditableImage {
  QString path;
  /// File name, for the picker rows.
  QString name;
  /// Last modified, ms since the epoch; 0 when unknown.
  qint64 stampMs = 0;
};

struct OutputConfig {
  /** Screenshot directory; empty means `~/Pictures/Screenshots`. */
  QString directory;
  /** Filename pattern without extension. Tokens: `{date}` (yyyy-MM-dd),
   *  `{time}` (HH-mm-ss), `{app}` (slug of the app under the selection). */
  QString filename = QStringLiteral("screenshot-{date}_{time}-{app}");
};

/** Reads [output] directory and `output` filename. A missing file or key
 *  leaves the default untouched; `~` in directory expands to $HOME. */
[[nodiscard]] OutputConfig loadOutputConfig(const QString &filePath);

/// Where screenshots are saved, without creating it: `OMASNAP_SCREENSHOT_DIR`,
/// then `[output] directory` in the config, then `~/Pictures/Screenshots`.
/// The open-image picker lists this same directory.
[[nodiscard]] QString screenshotDirectory();

/// Every readable image in `directory`, newest first (mtime descending, name
/// ascending on ties). Empty when the directory is missing or holds none.
[[nodiscard]] QVector<EditableImage> listEditableImages(const QString &directory);

/** Expands `pattern` for `when` and `appSlug` into a safe `.png` filename.
 *  An empty `appSlug` removes `{app}` together with one separator before it
 *  so `screenshot-{date}-{app}` yields `screenshot-<date>.png`, not a
 *  trailing dash. Never returns an empty or path-like name. */
[[nodiscard]] QString formatScreenshotFilename(const QString &pattern,
                                               const QDateTime &when,
                                               const QString &appSlug);

/** ~/.config/omasnap/omasnap.conf (XDG config location). */
[[nodiscard]] QString defaultConfigPath();
