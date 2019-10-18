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
	, model(nullptr)
{
	setCompletionRole(Qt::DisplayRole);
	setCompletionMode(PopupCompletion);
}

// Initializes completer with a new model
void ObjectNameCompleter::Initialize()
{
	model = new QSqlQueryModel(this);
	setModel(model);
}

// Finishes completer usage by deleting current model
void ObjectNameCompleter::Finish()
{
	delete model;
}

// Fills model with object name data got from database by type and schema
void ObjectNameCompleter::Fetch(int type_index, const QString &schema)
{
	QString query_text = "";

	switch (type_index)
	{
		case ObjectTypes::table:
		{
			query_text = table_query;
			break;
		}
		case ObjectTypes::sequence:
		{
			query_text = sequence_query;
			break;
		}
		case ObjectTypes::function:
		{
			query_text = function_query;
			break;
		}
		case ObjectTypes::view:
		{
			query_text = view_query;
			break;
		}
		case ObjectTypes::trigger:
		{
			query_text = trigger_query;
			break;
		}
		case ObjectTypes::index:
		{
			query_text = index_query;
			break;
		}
		default:
		{
			return;
		}
	}

	QSqlQuery fetch;
	fetch.prepare(query_text);
	fetch.addBindValue(schema);
	fetch.exec();
	model->setQuery(fetch);
}

// Clears model
void ObjectNameCompleter::Clear()
{
	model->clear();
}