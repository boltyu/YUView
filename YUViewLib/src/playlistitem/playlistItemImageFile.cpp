/*  This file is part of YUView - The YUV player with advanced analytics toolset
 *   <https://github.com/IENT/YUView>
 *   Copyright (C) 2015  Institut für Nachrichtentechnik, RWTH Aachen University, GERMANY
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   In addition, as a special exception, the copyright holders give
 *   permission to link the code of portions of this program with the
 *   OpenSSL library under certain conditions as described in each
 *   individual source file, and distribute linked combinations including
 *   the two.
 *
 *   You must obey the GNU General Public License in all respects for all
 *   of the code used other than OpenSSL. If you modify file(s) with this
 *   exception, you may extend this exception to your version of the
 *   file(s), but you are not obligated to do so. If you do not wish to do
 *   so, delete this exception statement from your version. If you delete
 *   this exception statement from all source files in the program, then
 *   also delete it here.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "playlistItemImageFile.h"

#include <QImageReader>
#include <QPainter>
#include <QSettings>
#include <QUrl>

#include <common/Formatting.h>
#include <common/FunctionsGui.h>
#include <filesource/FileSource.h>

#define IMAGEFILE_ERROR_TEXT "The given image file could not be loaded."

playlistItemImageFile::playlistItemImageFile(const QString &filePath)
    : playlistItem(filePath, Type::Static)
{
  // Set the properties of the playlistItem
  this->setIcon(0, functionsGui::convertIcon(":img_television.png"));
  // Nothing can be dropped onto an image file
  this->setFlags(flags() & ~Qt::ItemIsDropEnabled);

  this->prop.isFileSource          = true;
  this->prop.propertiesWidgetTitle = "Image Properties";

  QFileInfo fileInfo(filePath);
  if (!fileInfo.exists() || !fileInfo.isFile())
    return;

  connect(&fileWatcher,
          &QFileSystemWatcher::fileChanged,
          this,
          &playlistItemImageFile::fileSystemWatcherFileChanged);

  this->updateSettings();
}

void playlistItemImageFile::loadFrame(int, bool, bool, bool emitSignals)
{
  this->imageLoading = true;
  this->frame.loadCurrentImageFromFile(this->properties().name);
  this->imageLoading    = false;
  this->needToLoadImage = false;

  if (emitSignals)
    emit SignalItemChanged(true, RECACHE_NONE);
}

void playlistItemImageFile::savePlaylist(QDomElement &root, const QDir &playlistDir) const
{
  const auto filename = this->properties().name;

  // Determine the relative path to the raw file. We save both in the playlist.
  QUrl fileURL(filename);
  fileURL.setScheme("file");
  auto relativePath = playlistDir.relativeFilePath(filename);

  auto d = YUViewDomElement(root.ownerDocument().createElement("playlistItemImageFile"));

  // Append the properties of the playlistItem
  playlistItem::appendPropertiesToPlaylist(d);

  // Append all the properties of the raw file (the path to the file. Relative and absolute)
  d.appendProperiteChild("absolutePath", fileURL.toString());
  d.appendProperiteChild("relativePath", relativePath);

  root.appendChild(d);
}

/* Parse the playlist and return a new playlistItemImageFile.
 */
playlistItemImageFile *
playlistItemImageFile::newplaylistItemImageFile(const YUViewDomElement &root,
                                                const QString          &playlistFilePath)
{
  // Parse the DOM element. It should have all values of a playlistItemImageFile
  auto absolutePath = root.findChildValue("absolutePath");
  auto relativePath = root.findChildValue("relativePath");

  // check if file with absolute path exists, otherwise check relative path
  auto filePath = FileSource::getAbsPathFromAbsAndRel(playlistFilePath, absolutePath, relativePath);
  if (filePath.isEmpty())
    return nullptr;

  auto newImage = new playlistItemImageFile(filePath);

  // Load the propertied of the playlistItemIndexed
  playlistItem::loadPropertiesFromPlaylist(root, newImage);

  return newImage;
}

void playlistItemImageFile::drawItem(QPainter *painter, int, double zoomFactor, bool drawRawData)
{
  if (!frame.isFormatValid())
  {
    // The image could not be loaded. Draw a text instead.
    // Get the size of the text and create a QRect of that size which is centered at (0,0)
    auto displayFont = painter->font();
    displayFont.setPointSizeF(painter->font().pointSizeF() * zoomFactor);
    painter->setFont(displayFont);
    auto  textSize = painter->fontMetrics().size(0, IMAGEFILE_ERROR_TEXT);
    QRect textRect;
    textRect.setSize(textSize);
    textRect.moveCenter(QPoint(0, 0));

    painter->drawText(textRect, IMAGEFILE_ERROR_TEXT);
  }
  else if (!this->imageLoading)
    frame.drawFrame(painter, zoomFactor, drawRawData);
}

ItemLoadingState playlistItemImageFile::needsLoading(int, bool)
{
  return needToLoadImage ? ItemLoadingState::LoadingNeeded : ItemLoadingState::LoadingNotNeeded;
}

void playlistItemImageFile::getSupportedFileExtensions(QStringList &allExtensions,
                                                       QStringList &filters)
{
  const QList<QByteArray> formats = QImageReader::supportedImageFormats();

  QString filter = "Static Image (";
  for (auto &fmt : formats)
  {
    auto formatString = QString(fmt);
    allExtensions.append(formatString);
    filter += "*." + formatString + " ";
  }

  // Append Targa/TGA extensions
  for (auto fmt : {"tga", "icb", "vda", "vst"})
  {
    auto formatString = QString(fmt);
    allExtensions.append(formatString);
    filter += "*." + formatString + " ";
  }

  if (filter.endsWith(' '))
    filter.chop(1);

  filter += ")";

  filters.append(filter);
}

ValuePairListSets playlistItemImageFile::getPixelValues(const QPoint &pixelPos, int)
{
  ValuePairListSets newSet;
  newSet.append("RGB", frame.getPixelValues(pixelPos, -1));
  return newSet;
}

InfoData playlistItemImageFile::getInfo() const
{
  InfoData info("Image Info");

  info.items.append(InfoItem("File", this->properties().name.toStdString()));
  if (frame.isFormatValid())
  {
    info.items.append(InfoItem("Resolution",
                               to_string(frame.getFrameSize()),
                               "The video resolution in pixel (width x height)"));
    info.items.append(InfoItem(
        "Bit depth", std::to_string(frame.getImageBitDepth()), "The bit depth of the image."));
  }
  else if (isLoading())
    info.items.append(
        InfoItem("Status"sv, "Loading...", "The image is being loaded. Please wait."));
  else
    info.items.append(InfoItem("Status"sv, "Error", "There was an error loading the image."));

  return info;
}

QSize playlistItemImageFile::getSize() const
{
  auto s = frame.getFrameSize();
  return QSize(s.width, s.height);
}

void playlistItemImageFile::updateSettings()
{
  // Install a file watcher if file watching is active in the settings.
  // The addPath/removePath functions will do nothing if called twice for the same file.
  QSettings settings;
  if (settings.value("WatchFiles", true).toBool())
    fileWatcher.addPath(this->properties().name);
  else
    fileWatcher.removePath(this->properties().name);
}

void playlistItemImageFile::fileSystemWatcherFileChanged(const QString &)
{
  this->needToLoadImage = true;
  emit SignalItemChanged(true, RECACHE_CLEAR);
  this->updateSettings();
}
