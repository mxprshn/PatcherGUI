#include "ObjectNameCompleter.h"
#include "ObjectTypes.h"

#include <QSqlQueryModel>
#include <QSqlQuery>

const QString ObjectNameCompleter::table_query = "SELECT DISTINCT table_name FROM information_schema.tables WHERE table_schema = ? AND table_type != 'VIEW';";
const QString ObjectNameCompleter::sequence_query = "SELECT DISTINCT sequence_name FROM information_schema.sequences WHERE sequence_schema = ?;";
const QString ObjectNameCompleter::function_query = "SELECT r.routine_name || '(' || COALESCE(array_to_string(p.proargnames, ',', '*'), '') || ')' "
		"FROM information_schema.routines r, pg_catalog.pg_proc p WHERE r.specific_schema = ? AND r.external_language = 'PLPGSQL' "
		"AND r.routine_name = p.proname AND r.specific_name = p.proname || '_' || p.oid;";
const QString ObjectNameCompleter::view_query = "SELECT DISTINCT table_name FROM information_schema.views WHERE table_schema = ?;";
const QString ObjectNameCompleter::trigger_query = "SELECT DISTINCT trigger_name FROM information_schema.triggers WHERE trigger_schema = ?;";
const QString ObjectNameCompleter::index_query = "SELECT DISTINCT indexname FROM pg_indexes WHERE schemaname = ?;";

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
void ObjectNameCompleter::Initialize(int type_index, const QString &schema)
{
	QString queryText = "";

	switch (type_index)
	{
		case ObjectTypes::table:
		{
			queryText = table_query;
			break;
		}
		case ObjectTypes::sequence:
		{
			queryText = sequence_query;
			break;
		}
		case ObjectTypes::function:
		{
			queryText = function_query;
			break;
		}
		case ObjectTypes::view:
		{
			queryText = view_query;
			break;
		}
		case ObjectTypes::trigger:
		{
			queryText = trigger_query;
			break;
		}
		case ObjectTypes::index:
		{
			queryText = index_query;
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
void ObjectNameCompleter::Clear()
{
	model->clear();
}

// Destructor
ObjectNameCompleter::~ObjectNameCompleter()
{
	delete model;
}
