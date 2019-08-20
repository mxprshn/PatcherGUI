#include "DependencyListWidget.h"
#include "ObjectTypes.h"

#include <QHeaderView>
#include <QBitArray>

const QHash<int, QString> DependencyListWidget::statusIcons = QHash<int, QString>({ {waitingForCheck, ":/images/unchecked.svg"}
		, {satisfied, ":/images/checked.svg"}, {notSatisfied, ":/images/error.svg"} });

// Constructor
DependencyListWidget::DependencyListWidget(QWidget *parent)
	: QTreeWidget(parent)
	, checkedCount(0)
	, areAllSatisfied(true)
{
	setColumnCount(4);
	QStringList headerLabels;
	headerLabels.insert(typeColumn, "Type");
	headerLabels.insert(schemaColumn, "Schema");
	headerLabels.insert(nameColumn, "Name");
	headerLabels.insert(statusColumn, "Status");
	setHeaderLabels(headerLabels);
	setRootIsDecorated(false);
	setSelectionMode(SingleSelection);
	header()->setStretchLastSection(false);
	header()->setSectionResizeMode(typeColumn, QHeaderView::ResizeMode::ResizeToContents);
	header()->setSectionResizeMode(schemaColumn, QHeaderView::ResizeMode::ResizeToContents);
	header()->setSectionResizeMode(nameColumn, QHeaderView::ResizeMode::Stretch);
	header()->setSectionResizeMode(statusColumn, QHeaderView::ResizeMode::ResizeToContents);
	setSortingEnabled(true);

	connect(this, SIGNAL(itemClicked(QTreeWidgetItem*, int)), SLOT(onItemClicked(QTreeWidgetItem*, int)));
}

// Marks dependencies in list as satisfied/not satisfied by check result bit array
bool DependencyListWidget::setCheckStatus(const QBitArray& checkResult)
{
	if (checkResult.count() != topLevelItemCount())
	{
		return false;
	}

	areAllSatisfied = true;

	for (auto i = 0; i < checkResult.count(); ++i)
	{
		if (checkResult[i])
		{
			++checkedCount;
			topLevelItem(i)->setCheckState(statusColumn, Qt::Checked);
			topLevelItem(i)->setIcon(statusColumn, QIcon(statusIcons.value(satisfied)));
			topLevelItem(i)->setData(statusColumn, Qt::UserRole, satisfied);
		}
		else
		{
			areAllSatisfied = false;
			topLevelItem(i)->setIcon(statusColumn, QIcon(statusIcons.value(notSatisfied)));
			topLevelItem(i)->setData(statusColumn, Qt::UserRole, notSatisfied);
		}
	}

	emit itemCheckChanged();
	return true;
}

// Adds a new dependency to list and marks it as waiting for check
void DependencyListWidget::add(int typeIndex, const class QString& schema, const class QString& name)
{
	auto *newItem = new QTreeWidgetItem(this);
	newItem->setIcon(typeColumn, QIcon(ObjectTypes::typeIcons.value(typeIndex)));
	newItem->setText(typeColumn, ObjectTypes::typeNames.value(typeIndex));
	newItem->setData(typeColumn, Qt::UserRole, typeIndex);
	newItem->setText(schemaColumn, schema);
	newItem->setText(nameColumn, name);
	newItem->setIcon(statusColumn, QIcon(statusIcons.value(waitingForCheck)));
	newItem->setCheckState(statusColumn, Qt::Unchecked);
	newItem->setData(statusColumn, Qt::UserRole, waitingForCheck);
	newItem->setFlags(Qt::ItemIsEnabled);
	addTopLevelItem(newItem);
}

// Clears current list
void DependencyListWidget::clear()
{
	QTreeWidget::clear();
	checkedCount = 0;
	areAllSatisfied = true;
}

// Getter for checkedCount
int DependencyListWidget::getCheckedCount() const
{
	return checkedCount;
}

// Getter for areAllSatisfied
bool DependencyListWidget::getAreAllSatisfied() const
{
	return areAllSatisfied;
}

// Handles user's click on item
void DependencyListWidget::onItemClicked(QTreeWidgetItem *item, int column)
{
	if (item->data(statusColumn, Qt::UserRole).toInt() == waitingForCheck)
	{
		return;
	}

	if (item->checkState(statusColumn) != Qt::Checked)
	{
		++checkedCount;
		item->setCheckState(statusColumn, Qt::Checked);		
	}
	else
	{
		--checkedCount;
		item->setCheckState(statusColumn, Qt::Unchecked);
	}

	emit itemCheckChanged();
}