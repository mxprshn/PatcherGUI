#pragma once

#include <QString>

class QSqlDatabase;
class QSqlQueryModel;

// Class for database connection and retrieving information from it
class DatabaseProvider
{
public:
	DatabaseProvider() = delete;
	static QString Database();
	static QString User();
	static QString Password();
	static QString Host();
	static int Port();
	static bool IsConnected();
	static bool Connect(const QString &database, const QString &user, const QString &password,
		const QString &server, const int port, QString &error_message);
	static void Disconnect();
	static bool TableExists(const QString &schema, const QString &name);
	static bool SequenceExists(const QString &schema, const QString &name);
	static bool FunctionExists(const QString &schema, const QString &signature);
	static bool ViewExists(const QString &schema, const QString &name);
	static bool TriggerExists(const QString &schema, const QString &name);
	static bool IndexExists(const QString &schema, const QString &name);
	static void InitSchemaListModel(QSqlQueryModel &model);
};