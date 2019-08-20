#pragma once

#include <QString>

class QSqlDatabase;
class QSqlQueryModel;

// Class for database connection and retrieving information from it
class DatabaseProvider
{
public:
	DatabaseProvider() = delete;
	static QString database();
	static QString user();
	static QString password();
	static QString host();
	static int port();
	static bool isConnected();
	static bool connect(const QString &database, const QString &user, const QString &password,
		const QString &server, const int port, QString &errorMessage);
	static void disconnect();
	static bool tableExists(const QString &schema, const QString &name);
	static bool sequenceExists(const QString &schema, const QString &name);
	static bool functionExists(const QString &schema, const QString &signature);
	static bool viewExists(const QString &schema, const QString &name);
	static bool triggerExists(const QString &schema, const QString &name);
	static bool indexExists(const QString &schema, const QString &name);
	static void initSchemaListModel(QSqlQueryModel &model);
};