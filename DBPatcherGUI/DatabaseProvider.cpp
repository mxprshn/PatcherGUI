#include "DatabaseProvider.h"

#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlQueryModel>

// Returns name of current database
QString DatabaseProvider::Database()
{
	return IsConnected() ? QSqlDatabase::database().databaseName() : "";
}

// Returns username of current database connection
QString DatabaseProvider::User()
{
	return IsConnected() ? QSqlDatabase::database().userName() : "";
}

// Returns password of current database connection
QString DatabaseProvider::Password()
{
	return IsConnected() ? QSqlDatabase::database().password() : "";
}

// Returns host name of current database connection
QString DatabaseProvider::Host()
{
	return IsConnected() ? QSqlDatabase::database().hostName() : "";
}

// Returns port of current database
int DatabaseProvider::Port()
{
	return IsConnected() ? QSqlDatabase::database().port() : -1;
}

// Checks if connection to database is established
bool DatabaseProvider::IsConnected()
{
	return QSqlDatabase::database(QSqlDatabase::database().connectionName(), false).isOpen();
}

// Connects to database and returns result of connection
bool DatabaseProvider::Connect(const QString &database, const QString &user, const QString &password,
	const QString &server, const int port, QString &error_message)
{
	if (IsConnected())
	{
		error_message = "Already connected.";
		return false;
	}

	auto connection = QSqlDatabase::addDatabase("QPSQL");
	connection.setDatabaseName(database);
	connection.setUserName(user);
	connection.setPassword(password);
	connection.setHostName(server);
	connection.setPort(port);

	const auto is_connection_set = connection.open();

	if (!is_connection_set)
	{
		error_message = connection.lastError().text();
	}

	return is_connection_set;
}

// Disconnects from database
void DatabaseProvider::Disconnect()
{
	const auto connection_name = QSqlDatabase::database().connectionName();
	auto connection = QSqlDatabase::database(connection_name, false);

	if (connection.isOpen())
	{
		connection.close();
	}

	QSqlDatabase::removeDatabase(connection_name);
}

// Checks table for existence in database
bool DatabaseProvider::TableExists(const QString &schema, const QString &name)
{
	QSqlQuery check;
	check.prepare("SELECT EXISTS (SELECT * FROM information_schema.tables WHERE table_schema = ? AND table_type != 'VIEW'"
		" AND table_name = ?)");
	check.addBindValue(schema);
	check.addBindValue(name);
	check.exec();
	check.next();
	return check.value("exists").toBool();
}

// Checks sequence for existence in database
bool DatabaseProvider::SequenceExists(const QString &schema, const QString &name)
{
	QSqlQuery check;
	check.prepare("SELECT EXISTS (SELECT * FROM information_schema.sequences WHERE sequence_schema = ?"
		"AND sequence_name = ?)");
	check.addBindValue(schema);
	check.addBindValue(name);
	check.exec();
	check.next();
	return check.value("exists").toBool();
}

// Checks function for existence in database
bool DatabaseProvider::FunctionExists(const QString &schema, const QString &signature)
{
	QSqlQuery check;
	check.prepare("SELECT EXISTS (SELECT * FROM information_schema.routines r, pg_catalog.pg_proc p WHERE"
		" r.specific_schema = ? AND r.routine_name||'('||COALESCE(array_to_string(p.proargnames, ',', '*'),'')||')' = ?"
		" AND r.external_language = 'PLPGSQL' AND r.routine_name = p.proname AND"
		" r.specific_name = p.proname || '_' || p.oid);");
	check.addBindValue(schema);
	check.addBindValue(signature);
	check.exec();
	check.next();
	return check.value("exists").toBool();
}

// Checks view for existence in database
bool DatabaseProvider::ViewExists(const QString &schema, const QString &name)
{
	QSqlQuery check;
	check.prepare("SELECT EXISTS (SELECT * FROM information_schema.views WHERE table_schema = ?"
		"AND table_name = ?)");
	check.addBindValue(schema);
	check.addBindValue(name);
	check.exec();
	check.next();
	return check.value("exists").toBool();
}

// Checks trigger for existence in database
bool DatabaseProvider::TriggerExists(const QString &schema, const QString &name)
{
	QSqlQuery check;
	check.prepare("SELECT EXISTS (SELECT * FROM information_schema.triggers WHERE trigger_schema = ?"
		"AND trigger_name = ?)");
	check.addBindValue(schema);
	check.addBindValue(name);
	check.exec();
	check.next();
	return check.value("exists").toBool();
}

// Checks index for existence in database
bool DatabaseProvider::IndexExists(const QString &schema, const QString &name)
{
	QSqlQuery check;
	check.prepare("SELECT EXISTS (SELECT * FROM pg_indexes WHERE schemaname = ? AND indexname = ?);");
	check.addBindValue(schema);
	check.addBindValue(name);
	check.exec();
	check.next();
	return check.value("exists").toBool();
}

// Initializes schema list with data from database
void DatabaseProvider::InitSchemaListModel(QSqlQueryModel &model)
{
	model.setQuery("SELECT schema_name FROM information_schema.schemata WHERE"
		" schema_name NOT IN ('pg_catalog', 'information_schema') AND schema_name NOT LIKE 'pg_toast%' AND schema_name NOT LIKE 'pg_temp%';");
}