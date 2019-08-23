#include "PatchListWidget.h"
#include "ObjectTypes.h"

#include <QDropEvent>

// Constructor
PatchListWidget::PatchListWidget(QWidget* parent)
	: QTreeWidget(parent)
{
	setColumnCount(3);
	QStringList header_labels;
	header_labels.insert(type_column, "Type");
	header_labels.insert(schema_column, "Schema");
	header_labels.insert(name_column, "Name");
	setHeaderLabels(header_labels);
	setRootIsDecorated(false);
	setSelectionMode(SingleSelection);
	setDragEnabled(true);
	viewport()->setAcceptDrops(true);
	setDropIndicatorShown(true);
	setDragDropMode(InternalMove);
}

// Checks object for existence in the list
bool PatchListWidget::ItemExists(int type_index, const class QString& schema, const class QString& name)
{
	const auto found_items = findItems(name, Qt::MatchFixedString, name_column);

	for (const auto current : found_items)
	{
		if (current->text(type_column) == ObjectTypes::type_names.value(type_index) && current->text(schema_column) ==
			schema)
		{
			return true;
		}
	}

	return false;
}

// Adds a new object to list
void PatchListWidget::Add(int type_index, const class QString& schema, const class QString& name, bool is_draggable)
{
	auto* new_item = new QTreeWidgetItem(this);

	new_item->setIcon(type_column, QIcon(ObjectTypes::type_icons.value(type_index)));
	new_item->setText(type_column, ObjectTypes::type_names.value(type_index));
	new_item->setData(type_column, Qt::UserRole, type_index);
	new_item->setText(schema_column, schema);
	new_item->setText(name_column, name);

	if (is_draggable)
	{
		new_item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled);
	}
	else
	{
		new_item->setFlags(Qt::ItemIsEnabled);
	}

	addTopLevelItem(new_item);
	scrollToItem(new_item);
}

// Handles drop of dragged object
void PatchListWidget::dropEvent(QDropEvent* event)
{
	QTreeWidget::dropEvent(event);
	setCurrentItem(itemAt(event->pos()));
}