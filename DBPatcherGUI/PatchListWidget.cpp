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
bool PatchListWidget::itemExists(int typeIndex, const class QString &schema, const class QString &name)
{
	const auto foundItems = findItems(name, Qt::MatchFixedString, nameColumn);

	for (const auto current : foundItems)
	{
		if (current->text(typeColumn) == ObjectTypes::typeNames.value(typeIndex) && current->text(schemaColumn) == schema)
		{
			return true;
		}
	}

	return false;
}

// Adds a new object to list
void PatchListWidget::add(int typeIndex, const class QString& schema, const class QString& name, bool isDraggable)
{
	auto *newItem = new QTreeWidgetItem(this);

	newItem->setIcon(typeColumn, QIcon(ObjectTypes::typeIcons.value(typeIndex)));
	newItem->setText(typeColumn, ObjectTypes::typeNames.value(typeIndex));
	newItem->setData(typeColumn, Qt::UserRole, typeIndex);
	newItem->setText(schemaColumn, schema);
	newItem->setText(nameColumn, name);

	if (isDraggable)
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