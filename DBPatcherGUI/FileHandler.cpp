#include "FileHandler.h"
#include "DatabaseProvider.h"
#include "PatchListElement.h"
#include "ObjectTypes.h"

#include <QDir>
#include <QDateTime>
#include <QTextStream>

const QString FileHandler::patch_list_name = "PatchList.txt";
const QString FileHandler::dependency_list_name = "DependencyList.dpn";
const QString FileHandler::object_list_name = "ObjectList.txt";

// Makes directory for patch files
QDir FileHandler::MakePatchDir(const QString &path, bool &is_successful)
{
	QDir patchDir(path);
	// Can database have a name with dots?
	const auto patchDirName = DatabaseProvider::Database() + "_build_" + QDateTime::currentDateTime().toString("yyyy-MM-dd_HH-mm-ss");

	if (!patchDir.mkdir(patchDirName) || !patchDir.cd(patchDirName))
	{
		is_successful = false;
		return QString();
	}

	is_successful = true;
	return patchDir;
}

// Makes patch list file from PatchList object
bool FileHandler::MakePatchList(const QString &path, const PatchList &patch_list)
{
	const QDir patchDir(path);
	QFile file(patchDir.absoluteFilePath(patch_list_name));

	if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::NewOnly))
	{
		return false;
	}

	QTextStream patchFileStream(&file);

	for (const auto current : patch_list)
	{
		if (current->getType() == ObjectTypes::script)
		{
			patchFileStream << ObjectTypes::type_names.value(current->getType()) << " " << current->getName();
		}
		else
		{
			patchFileStream << current->getSchema() << " " << current->getName() << " " << ObjectTypes::type_names.value(current->getType());

			if (current->getType() == ObjectTypes::function)
			{
				patchFileStream << " " << GetParametersString(current->getParameters());
			}
		}

		patchFileStream << endl;
	}

	file.close();
	return true;
}

bool FileHandler::MakeDependencyList(const QString &path, const PatchList &dependency_list)
{
	const QDir patchDir(path);
	QFile tempFile(patchDir.absoluteFilePath("temp.dpn"));
	QFile file(patchDir.absoluteFilePath(dependency_list_name));

	if (!tempFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::NewOnly))
	{
		return false;
	}

	QTextStream dependencyFileStream(&tempFile);

	for (const auto current : dependency_list)
	{
		dependencyFileStream << current->getSchema() << " " << current->getName() << " "
			<< ObjectTypes::type_names.value(current->getType()) << endl;
	}

	tempFile.close();

	if (!file.remove())
	{
		tempFile.remove();
		return false;
	}

	auto temp = tempFile.rename(patchDir.absoluteFilePath(dependency_list_name));
	return true;
}

// Returns PatchList object parsed from object list file
PatchList FileHandler::ParseObjectList(const QString &path, bool &is_successful)
{
	const QDir patchDir(path);
	QFile file(patchDir.absoluteFilePath(object_list_name));

	if (!file.open(QIODevice::ReadOnly))
	{
		is_successful = false;
		return PatchList();
	}

	QTextStream input(&file);
	PatchList objectList;

	while (!input.atEnd())
	{
		const QString readString = input.readLine();

		if (readString.isEmpty())
		{
			continue;
		}

		int type = ObjectTypes::typeCount;
		QString schemaName = "";
		QString name;
		QStringList parameters = QStringList("");

		if (QRegExp("([^ ])+ ([^ ])+ (table|sequence|view|trigger|index)( )*").exactMatch(readString))
		{
			const auto splitResult = readString.split(" ", QString::SkipEmptyParts);
			schemaName = splitResult.at(0);
			name = splitResult.at(1);
			type = ObjectTypes::type_names.key(splitResult.at(2));
		}
		else if (QRegExp("script ([^ ])+( )*").exactMatch(readString))
		{
			const auto splitResult = readString.split(" ", QString::SkipEmptyParts);
			name = splitResult.at(1);
			type = ObjectTypes::script;
		}
		else if (QRegExp("([^ ])+ ([^ ])+ function \\( (([^,() ])+ )*\\)( )*").exactMatch(readString))
		{
			auto splitResult = readString.split(QRegExp("(\\ |\\(|\\))"), QString::SkipEmptyParts);
			schemaName = splitResult.first();
			splitResult.pop_front();
			name = splitResult.first();
			splitResult.pop_front();
			type = ObjectTypes::function;
			splitResult.pop_front();

			if (!splitResult.isEmpty())
			{
				parameters = splitResult;
			}
		}
		else
		{
			file.close();
			is_successful = false;
			return PatchList();
		}

		objectList.Add(type, schemaName, name, parameters);
	}

	file.close();
	is_successful = true;
	return objectList;
}

// Returns PatchList object parsed from dependency list file
PatchList FileHandler::ParseDependencyList(const QString &path, bool &is_successful)
{
	const QDir patchDir(path);
	QFile file(patchDir.absoluteFilePath(dependency_list_name));

	if (!file.open(QIODevice::ReadOnly))
	{
		is_successful = false;
		return PatchList();
	}

	QTextStream input(&file);
	PatchList dependencyList;

	while (!input.atEnd())
	{
		const QString readString = input.readLine();

		if (readString.isEmpty())
		{
			continue;
		}

		int type = ObjectTypes::typeCount;
		QString schemaName = "";
		QString name;

		if (QRegExp("([^ ])+ ([^ ])+ (table|sequence|view|trigger|index|function)( )*").exactMatch(readString))
		{
			const auto splitResult = readString.split(" ", QString::SkipEmptyParts);
			schemaName = splitResult.at(0);
			name = splitResult.at(1);
			type = ObjectTypes::type_names.key(splitResult.at(2));
		}
		else
		{
			file.close();
			is_successful = false;
			return PatchList();
		}

		dependencyList.Add(type, schemaName, name, QStringList());
	}

	file.close();
	is_successful = true;
	return dependencyList;
}

// Getter for patchListName
QString FileHandler::GetPatchListName()
{
	return patch_list_name;
}

// Getter for dependencyListName
QString FileHandler::GetDependencyListName()
{
	return dependency_list_name;
}

// Getter for objectListName
QString FileHandler::GetObjectListName()
{
	return object_list_name;
}

// Returns formatted parameters string made from list of parameters
QString FileHandler::GetParametersString(const QStringList &parameters)
{
	QString result = "( ";

	for (const auto &current : parameters)
	{
		result += current + " ";
	}

	return result + ")";
}