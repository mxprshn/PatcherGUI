#include "PatchListWidget.h"
#include "ObjectTypes.h"

#include <QDropEvent>

// Constructor
PatchListWidget::PatchListWidget(QWidget *parent)
	: QTreeWidget(parent)
{
	setColumnCount(3);
	QStringList headerLabels;
	headerLabels.insert(typeColumn, "Type");
	headerLabels.insert(schemaColumn, "Schema");
	headerLabels.insert(nameColumn, "Name");
	setHeaderLabels(headerLabels);
	setRootIsDecorated(false);
	setSelectionMode(SingleSelection);
	setDragEnabled(true);
	viewport()->setAcceptDrops(true);
	setDropIndicatorShown(true);
	setDragDropMode(InternalMove);
}

// Checks object for existence in the list
bool PatchListWidget::ItemExists(int type_index, const class QString &schema, const class QString &name)
{
	const auto foundItems = findItems(name, Qt::MatchFixedString, nameColumn);

	for (const auto current : foundItems)
	{
		if (current->text(typeColumn) == ObjectTypes::type_names.value(type_index) && current->text(schemaColumn) == schema)
		{
			return true;
		}
	}

	return false;
}

// Adds a new object to list
void PatchListWidget::Add(int type_index, const class QString& schema, const class QString& name, bool is_draggable)
{
	auto *newItem = new QTreeWidgetItem(this);

	newItem->setIcon(typeColumn, QIcon(ObjectTypes::type_icons.value(type_index)));
	newItem->setText(typeColumn, ObjectTypes::type_names.value(type_index));
	newItem->setData(typeColumn, Qt::UserRole, type_index);
	newItem->setText(schemaColumn, schema);
	newItem->setText(nameColumn, name);

	if (is_draggable)
	{
		newItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled);
	}
	else
	{
		newItem->setFlags(Qt::ItemIsEnabled);
	}

	addTopLevelItem(newItem);
	scrollToItem(newItem);
}

// Handles drop of dragged object
void PatchListWidget::dropEvent(QDropEvent *event)
{
	QTreeWidget::dropEvent(event);
	setCurrentItem(itemAt(event->pos()));
}