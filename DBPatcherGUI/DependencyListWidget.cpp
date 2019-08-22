#include "DependencyListWidget.h"
#include "ObjectTypes.h"

#include <QHeaderView>
#include <QBitArray>

const QHash<int, QString> DependencyListWidget::status_icons = QHash<int, QString>({ {waitingForCheck, ":/images/unchecked.svg"}
		, {satisfied, ":/images/checked.svg"}, {notSatisfied, ":/images/error.svg"} });

// Constructor
DependencyListWidget::DependencyListWidget(QWidget *parent)
	: QTreeWidget(parent)
	, checked_count(0)
	, are_all_satisfied(true)
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
bool DependencyListWidget::SetCheckStatus(const QBitArray& check_result)
{
	if (check_result.count() != topLevelItemCount())
	{
		return false;
	}

	are_all_satisfied = true;

	for (auto i = 0; i < check_result.count(); ++i)
	{
		const auto current = topLevelItem(i);

		if (check_result[i])
		{
			++checked_count;
			current->setCheckState(statusColumn, Qt::Checked);
			current->setIcon(statusColumn, QIcon(status_icons.value(satisfied)));
			current->setData(statusColumn, Qt::UserRole, satisfied);
		}
		else
		{
			are_all_satisfied = false;
			current->setIcon(statusColumn, QIcon(status_icons.value(notSatisfied)));
			current->setData(statusColumn, Qt::UserRole, notSatisfied);
		}
	}

	emit ItemCheckChanged();
	return true;
}

// Adds a new dependency to list and marks it as waiting for check
void DependencyListWidget::Add(int type_index, const class QString &schema, const class QString &name)
{
	auto *newItem = new QTreeWidgetItem(this);
	newItem->setIcon(typeColumn, QIcon(ObjectTypes::type_icons.value(type_index)));
	newItem->setText(typeColumn, ObjectTypes::type_names.value(type_index));
	newItem->setData(typeColumn, Qt::UserRole, type_index);
	newItem->setText(schemaColumn, schema);
	newItem->setText(nameColumn, name);
	newItem->setIcon(statusColumn, QIcon(status_icons.value(waitingForCheck)));
	newItem->setCheckState(statusColumn, Qt::Unchecked);
	newItem->setData(statusColumn, Qt::UserRole, waitingForCheck);
	newItem->setFlags(Qt::ItemIsEnabled);
	addTopLevelItem(newItem);
}

// Clears current list
void DependencyListWidget::Clear()
{
	QTreeWidget::clear();
	checked_count = 0;
	are_all_satisfied = true;
}

// Clears current list check state
void DependencyListWidget::ClearCheck()
{
	for (auto i = 0; i < topLevelItemCount(); ++i)
	{
		const auto current = topLevelItem(i);
		current->setCheckState(statusColumn, Qt::Unchecked);
		current->setIcon(statusColumn, QIcon(status_icons.value(waitingForCheck)));
		current->setData(statusColumn, Qt::UserRole, waitingForCheck);
	}

	checked_count = 0;
	are_all_satisfied = true;
}

// Getter for checkedCount
int DependencyListWidget::GetCheckedCount() const
{
	return checked_count;
}

// Getter for are_all_satisfied
bool DependencyListWidget::GetAreAllSatisfied() const
{
	return are_all_satisfied;
}

// Handles user's click on item
void DependencyListWidget::OnItemClicked(QTreeWidgetItem *item, int column)
{
	if (item->data(statusColumn, Qt::UserRole).toInt() == waitingForCheck)
	{
		return;
	}

	if (item->checkState(statusColumn) != Qt::Checked)
	{
		++checked_count;
		item->setCheckState(statusColumn, Qt::Checked);		
	}
	else
	{
		--checked_count;
		item->setCheckState(statusColumn, Qt::Unchecked);
	}

	emit ItemCheckChanged();
}