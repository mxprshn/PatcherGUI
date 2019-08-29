#pragma once

#include <QHash>

// Namespace for global type name indexes
namespace ObjectTypes
{
	enum Type
	{
		script,
		table,
		sequence,
		function,
		view,
		trigger,
		index,
		type_count
	};

	// Hash for type icon names and type names
	extern const QHash<int, QString> type_icons;
	extern const QHash<int, QString> type_names;
}

