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
	QDir patch_dir(path);
	// Can database have a name with dots?
	const auto patch_dir_name = DatabaseProvider::Database() + "_build_" + QDateTime::currentDateTime().toString("yyyy-MM-dd_HH-mm-ss");

	if (!patch_dir.mkdir(patch_dir_name) || !patch_dir.cd(patch_dir_name))
	{
		is_successful = false;
		return QString();
	}

	is_successful = true;
	return patch_dir;
}

// Makes patch list file from PatchList object
bool FileHandler::MakePatchList(const QString &path, const PatchList &patch_list)
{
	const QDir patch_dir(path);
	QFile file(patch_dir.absoluteFilePath(patch_list_name));

	if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::NewOnly))
	{
		return false;
	}

	QTextStream patch_file_stream(&file);

	for (const auto current : patch_list)
	{
		if (current->GetType() == ObjectTypes::script)
		{
			patch_file_stream << ObjectTypes::type_names.value(current->GetType()) << " " << current->GetName();
		}
		else
		{
			patch_file_stream << current->GetSchema() << " " << current->GetName() << " " << ObjectTypes::type_names.value(current->GetType());

			if (current->GetType() == ObjectTypes::function)
			{
				patch_file_stream << " " << GetParametersString(current->GetParameters());
			}
		}

		patch_file_stream << endl;
	}

	file.close();
	return true;
}

bool FileHandler::MakeDependencyList(const QString &path, const PatchList &dependency_list)
{
	const QDir patch_dir(path);
	QFile temp_file(patch_dir.absoluteFilePath("temp.dpn"));
	QFile file(patch_dir.absoluteFilePath(dependency_list_name));

	if (!temp_file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::NewOnly))
	{
		return false;
	}

	QTextStream dependency_file_stream(&temp_file);

	for (const auto current : dependency_list)
	{
		dependency_file_stream << current->GetSchema() << " " << current->GetName() << " "
			<< ObjectTypes::type_names.value(current->GetType()) << endl;
	}

	temp_file.close();

	if (!file.remove())
	{
		temp_file.remove();
		return false;
	}

	auto temp = temp_file.rename(patch_dir.absoluteFilePath(dependency_list_name));
	return true;
}

// Returns PatchList object parsed from object list file
PatchList FileHandler::ParseObjectList(const QString &path, bool &is_successful)
{
	const QDir patch_dir(path);
	QFile file(patch_dir.absoluteFilePath(object_list_name));

	if (!file.open(QIODevice::ReadOnly))
	{
		is_successful = false;
		return PatchList();
	}

	QTextStream input(&file);
	PatchList object_list;

	while (!input.atEnd())
	{
		const QString read_string = input.readLine();

		if (QRegExp("( )*").exactMatch(read_string))
		{
			continue;
		}

		int type = ObjectTypes::type_count;
		QString schema_name = "";
		QString name;
		QStringList parameters = QStringList("");

		if (QRegExp("([^ ])+ ([^ ])+ (table|sequence|view|trigger|index)( )*").exactMatch(read_string))
		{
			const auto split_result = read_string.split(" ", QString::SkipEmptyParts);
			schema_name = split_result.at(0);
			name = split_result.at(1);
			type = ObjectTypes::type_names.key(split_result.at(2));
		}
		else if (QRegExp("script ([^ ])+( )*").exactMatch(read_string))
		{
			const auto split_result = read_string.split(" ", QString::SkipEmptyParts);
			name = split_result.at(1);
			type = ObjectTypes::script;
		}
		else if (QRegExp("([^ ])+ ([^ ])+ function \\( (([^,() ])+ )*\\)( )*").exactMatch(read_string))
		{
			auto split_result = read_string.split(QRegExp("(\\ |\\(|\\))"), QString::SkipEmptyParts);
			schema_name = split_result.first();
			split_result.pop_front();
			name = split_result.first();
			split_result.pop_front();
			type = ObjectTypes::function;
			split_result.pop_front();

			if (!split_result.isEmpty())
			{
				parameters = split_result;
			}
		}
		else
		{
			file.close();
			is_successful = false;
			return PatchList();
		}

		object_list.Add(type, schema_name, name, parameters);
	}

	file.close();
	is_successful = true;
	return object_list;
}

// Returns PatchList object parsed from dependency list file
PatchList FileHandler::ParseDependencyList(const QString &path, bool &is_successful)
{
	const QDir patch_dir(path);
	QFile file(patch_dir.absoluteFilePath(dependency_list_name));

	if (!file.open(QIODevice::ReadOnly))
	{
		is_successful = false;
		return PatchList();
	}

	QTextStream input(&file);
	PatchList dependency_list;

	while (!input.atEnd())
	{
		const QString read_string = input.readLine();

		if (QRegExp("( )*").exactMatch(read_string))
		{
			continue;
		}

		int type = ObjectTypes::type_count;
		QString schema_name = "";
		QString name;

		if (QRegExp("([^ ])+ ([^ ])+ (table|sequence|view|trigger|index|function)( )*").exactMatch(read_string))
		{
			const auto split_result = read_string.split(" ", QString::SkipEmptyParts);
			schema_name = split_result.at(0);
			name = split_result.at(1);
			type = ObjectTypes::type_names.key(split_result.at(2));
		}
		else
		{
			file.close();
			is_successful = false;
			return PatchList();
		}

		dependency_list.Add(type, schema_name, name, QStringList());
	}

	file.close();
	is_successful = true;
	return dependency_list;
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