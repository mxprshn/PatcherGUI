#pragma once

#include "PatchList.h"

#include <QString>
#include <QDir>

// Class operating with patch files
class FileHandler
{
public:
	FileHandler() = delete;
	static QDir makePatchDir(const QString &path, bool &isSuccessful);
	static bool makePatchList(const QString &path, const PatchList &patchList);
	static PatchList parseObjectList(const QString &path, bool &isSuccessful);
	static PatchList parseDependencyList(const QString &path, bool &isSuccessful);
	static QString getPatchListName();
	static QString getDependencyListName();
	static QString getObjectListName();
private:
	// Name of patch list file, which is created from gui
	static const QString patchListName;
	// Name of dependency list file which is created by Builder module
	static const QString dependencyListName;
	// Name of patch object list file which is created by Builder module
	static const QString objectListName;
	static QString getParametersString(const QStringList &parameters);
};