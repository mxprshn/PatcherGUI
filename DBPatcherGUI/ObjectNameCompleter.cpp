#include "ObjectNameCompleter.h"
#include "ObjectTypes.h"

#include <QSqlQueryModel>
#include <QSqlQuery>

const QString ObjectNameCompleter::tableQuery = "SELECT DISTINCT table_name FROM information_schema.tables WHERE table_schema = ? AND table_type != 'VIEW';";
const QString ObjectNameCompleter::sequenceQuery = "SELECT DISTINCT sequence_name FROM information_schema.sequences WHERE sequence_schema = ?;";
const QString ObjectNameCompleter::functionQuery = "SELECT r.routine_name || '(' || COALESCE(array_to_string(p.proargnames, ',', '*'), '') || ')' "
		"FROM information_schema.routines r, pg_catalog.pg_proc p WHERE r.specific_schema = ? and r.external_language = 'PLPGSQL' "
		"and r.routine_name = p.proname and r.specific_name = p.proname || '_' || p.oid;";
const QString ObjectNameCompleter::viewQuery = "SELECT DISTINCT table_name FROM information_schema.views WHERE table_schema = ?;";
const QString ObjectNameCompleter::triggerQuery = "SELECT DISTINCT trigger_name FROM information_schema.triggers WHERE trigger_schema = ?;";
const QString ObjectNameCompleter::indexQuery = "SELECT DISTINCT indexname FROM pg_indexes WHERE schemaname = ?;";

// Constructor
ObjectNameCompleter::ObjectNameCompleter(QObject *parent)
	: QCompleter(parent)
	, model(new QSqlQueryModel)
{
	setModel(model);
	setCompletionRole(Qt::DisplayRole);
	setCompletionMode(PopupCompletion);
}

// Fills model with object name data got from database by type and schema
void ObjectNameCompleter::initialize(int typeIndex, const QString &schema)
{
	clear();
	QString queryText = "";

	switch (typeIndex)
	{
		case ObjectTypes::table:
		{
			queryText = tableQuery;
			break;
		}
		case ObjectTypes::sequence:
		{
			queryText = sequenceQuery;
			break;
		}
		case ObjectTypes::function:
		{
			queryText = functionQuery;
			break;
		}
		case ObjectTypes::view:
		{
			queryText = viewQuery;
			break;
		}
		case ObjectTypes::trigger:
		{
			queryText = triggerQuery;
			break;
		}
		case ObjectTypes::index:
		{
			queryText = indexQuery;
			break;
		}
		default:
		{
			return;
		}
	}

	QSqlQuery fetch;
	fetch.prepare(queryText);
	fetch.addBindValue(schema);
	fetch.exec();
	model->setQuery(fetch);
}

// Clears model
void ObjectNameCompleter::clear()
{
	model->clear();
}

// Destructor
ObjectNameCompleter::~ObjectNameCompleter()
{
	delete model;
}
