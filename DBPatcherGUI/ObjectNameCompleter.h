#pragma once

#include <QCompleter>

class QSqlQueryModel;

// Class providing autocompletion of database object name input
class ObjectNameCompleter : public QCompleter
{
	Q_OBJECT

public:
	ObjectNameCompleter(QObject *parent = nullptr);
	~ObjectNameCompleter();
	void initialize(int typeIndex, const QString &schema);
	void clear();
private:
	// Object list model
	QSqlQueryModel *model;
	// Queries for fetching object names from database
	static const QString tableQuery;
	static const QString sequenceQuery;
	static const QString functionQuery;
	static const QString viewQuery;
	static const QString triggerQuery;
	static const QString indexQuery;
};