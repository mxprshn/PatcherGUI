#pragma once

#include <QCompleter>

class QSqlQueryModel;

// Class providing auto-completion of database object name input
class ObjectNameCompleter : public QCompleter
{
	Q_OBJECT

public:
	ObjectNameCompleter(QObject *parent = nullptr);
	void Fetch(int type_index, const QString &schema);
	void Clear();
	void Initialize();
	void Finish();
private:
	// Object list model
	QSqlQueryModel *model;
	// Queries for fetching object names from database
	static const QString table_query;
	static const QString sequence_query;
	static const QString function_query;
	static const QString view_query;
	static const QString trigger_query;
	static const QString index_query;
};