#include "DatabaseProvider.h"

#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlQueryModel>

// Returns name of current database
QString DatabaseProvider::database()
{
	return isConnected() ? QSqlDatabase::database().databaseName() : "";
}

// Returns username of current database connection
QString DatabaseProvider::user()
{
	return isConnected() ? QSqlDatabase::database().userName() : "";
}

// Returns password of current database connection
QString DatabaseProvider::password()
{
	return isConnected() ? QSqlDatabase::database().password() : "";
}

// Returns host name of current database connection
QString DatabaseProvider::host()
{
	return isConnected() ? QSqlDatabase::database().hostName() : "";
}

// Returns port of current database
int DatabaseProvider::port()
{
	return isConnected() ? QSqlDatabase::database().port() : -1;
}

// Checks if connection to database is established
bool DatabaseProvider::isConnected()
{
	return QSqlDatabase::database(QSqlDatabase::database().connectionName(), false).isOpen();
}

// Connects to database and returns result of connection
bool DatabaseProvider::connect(const QString &database, const QString &user, const QString &password,
	const QString &server, const int port, QString &errorMessage)
{
	if (isConnected())
	{
		errorMessage = "Already connected.";
		return false;
	}

	auto connection = QSqlDatabase::addDatabase("QPSQL");
	connection.setDatabaseName(database);
	connection.setUserName(user);
	connection.setPassword(password);
	connection.setHostName(server);
	connection.setPort(port);

	const auto isConnectionSet = connection.open();

	if (!isConnectionSet)
	{
		errorMessage = connection.lastError().text();
	}

	return isConnectionSet;
}

// Disconnects from database
void DatabaseProvider::disconnect()
{
	const auto connectionName = QSqlDatabase::database().connectionName();
	auto connection = QSqlDatabase::database(connectionName, false);

	if (connection.isOpen())
	{
		connection.close();
	}

	QSqlDatabase::removeDatabase(connectionName);
}

// Checks table for existence in database
bool DatabaseProvider::tableExists(const QString &schema, const QString &name)
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
bool DatabaseProvider::sequenceExists(const QString &schema, const QString &name)
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
bool DatabaseProvider::functionExists(const QString &schema, const QString &signature)
{
	QSqlQuery check;
	check.prepare("SELECT EXISTS (SELECT * FROM information_schema.routines r, pg_catalog.pg_proc p WHERE"
		" r.specific_schema = ? and r.routine_name||'('||COALESCE(array_to_string(p.proargnames, ',', '*'),'')||')' = ?"
		" and r.external_language = 'PLPGSQL' and r.routine_name = p.proname and"
		" r.specific_name = p.proname || '_' || p.oid);");
	check.addBindValue(schema);
	check.addBindValue(signature);
	check.exec();
	check.next();
	return check.value("exists").toBool();
}

// Checks view for existence in database
bool DatabaseProvider::viewExists(const QString &schema, const QString &name)
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
bool DatabaseProvider::triggerExists(const QString &schema, const QString &name)
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
bool DatabaseProvider::indexExists(const QString &schema, const QString &name)
{
	QSqlQuery check;
	check.prepare("SELECT EXISTS (SELECT * FROM pg_indexes WHERE schemaname = ? and indexname = ?);");
	check.addBindValue(schema);
	check.addBindValue(name);
	check.exec();
	check.next();
	return check.value("exists").toBool();
}

// Initializes schema list with data from database
void DatabaseProvider::initSchemaListModel(QSqlQueryModel &model)
{
	model.setQuery("SELECT schema_name FROM information_schema.schemata WHERE"
		" schema_name NOT IN ('pg_catalog', 'information_schema') AND schema_name NOT LIKE 'pg_toast%' AND schema_name NOT LIKE 'pg_temp%';");
}