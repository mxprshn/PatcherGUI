#pragma once

#include "PatchList.h"

#include <QString>
#include <QDir>

// Class operating with patch files
class FileHandler
{
public:
	FileHandler() = delete;
	static QDir MakePatchDir(const QString &path, bool &is_successful);
	static bool MakePatchList(const QString &path, const PatchList &patch_list);
	static bool MakeDependencyList(const QString &path, const PatchList &dependency_list);
	static PatchList ParseObjectList(const QString &path, bool &is_successful);
	static PatchList ParseDependencyList(const QString &path, bool &is_successful);
	static QString GetPatchListName();
	static QString GetDependencyListName();
	static QString GetObjectListName();
private:
	// Name of patch list file, which is created from gui
	static const QString patch_list_name;
	// Name of dependency list file which is created by Builder module
	static const QString dependency_list_name;
	// Name of patch object list file which is created by Builder module
	static const QString object_list_name;
	static QString GetParametersString(const QStringList &parameters);
};