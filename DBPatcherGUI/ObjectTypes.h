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
		typeCount
	};

	// Hash for type icon names and type names
	extern const QHash<int, QString> typeIcons;
	extern const QHash<int, QString> typeNames;
}

