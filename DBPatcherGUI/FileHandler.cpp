#include "FileHandler.h"
#include "DatabaseProvider.h"
#include "PatchListElement.h"
#include "ObjectTypes.h"

#include <QDir>
#include <QDateTime>
#include <QTextStream>

const QString FileHandler::patchListName = "PatchList.txt";
const QString FileHandler::dependencyListName = "DependencyList.dpn";
const QString FileHandler::objectListName = "ObjectList.txt";

// Makes directory for patch files
QDir FileHandler::makePatchDir(const QString &path, bool &isSuccessful)
{
	QDir patchDir(path);
	// Can database have a name with dots?
	const auto patchDirName = DatabaseProvider::database() + "_build_" + QDateTime::currentDateTime().toString("yyyy-MM-dd_HH-mm-ss");

	if (!patchDir.mkdir(patchDirName) || !patchDir.cd(patchDirName))
	{
		isSuccessful = false;
		return QString();
	}

	isSuccessful = true;
	return patchDir;
}

// Makes patch list file from PatchList object
bool FileHandler::makePatchList(const QString &path, const PatchList &patchList)
{
	const QDir patchDir(path);
	QFile file(patchDir.absoluteFilePath(patchListName));

	if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::NewOnly))
	{
		return false;
	}

	QTextStream patchFileStream(&file);

	for (const auto current : patchList)
	{
		if (current->getType() == ObjectTypes::script)
		{
			patchFileStream << ObjectTypes::typeNames.value(current->getType()) << " " << current->getName();
		}
		else
		{
			patchFileStream << current->getSchema() << " " << current->getName() << " " << ObjectTypes::typeNames.value(current->getType());

			if (current->getType() == ObjectTypes::function)
			{
				patchFileStream << " " << getParametersString(current->getParameters());
			}
		}

		patchFileStream << endl;
	}

	file.close();
	return true;
}

// Returns PatchList object parsed from object list file
PatchList FileHandler::parseObjectList(const QString &path, bool &isSuccessful)
{
	const QDir patchDir(path);
	QFile file(patchDir.absoluteFilePath(objectListName));

	if (!file.open(QIODevice::ReadOnly))
	{
		isSuccessful = false;
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
			type = ObjectTypes::typeNames.key(splitResult.at(2));
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
			isSuccessful = false;
			return PatchList();
		}

		objectList.add(type, schemaName, name, parameters);
	}

	file.close();
	isSuccessful = true;
	return objectList;
}

// Returns PatchList object parsed from dependency list file
PatchList FileHandler::parseDependencyList(const QString &path, bool &isSuccessful)
{
	const QDir patchDir(path);
	QFile file(patchDir.absoluteFilePath(dependencyListName));

	if (!file.open(QIODevice::ReadOnly))
	{
		isSuccessful = false;
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
			type = ObjectTypes::typeNames.key(splitResult.at(2));
		}
		else
		{
			file.close();
			isSuccessful = false;
			return PatchList();
		}

		dependencyList.add(type, schemaName, name, QStringList());
	}

	file.close();
	isSuccessful = true;
	return dependencyList;
}

// Getter for patchListName
QString FileHandler::getPatchListName()
{
	return patchListName;
}

// Getter for dependencyListName
QString FileHandler::getDependencyListName()
{
	return dependencyListName;
}

// Getter for objectListName
QString FileHandler::getObjectListName()
{
	return objectListName;
}

// Returns formatted parameters string made from list of parameters
QString FileHandler::getParametersString(const QStringList &parameters)
{
	QString result = "( ";

	for (const auto &current : parameters)
	{
		result += current + " ";
	}

	return result + ")";
}