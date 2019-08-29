#include "DependencyListWidget.h"
#include "ObjectTypes.h"

#include <QHeaderView>
#include <QBitArray>

const QHash<int, QString> DependencyListWidget::status_icons = QHash<int, QString>({ {waiting_for_check, ":/images/unchecked.svg"}
		, {satisfied, ":/images/checked.svg"}, {not_satisfied, ":/images/error.svg"} });

// Constructor
DependencyListWidget::DependencyListWidget(QWidget *parent)
	: QTreeWidget(parent)
	, checked_count(0)
	, are_all_satisfied(true)
{
	setColumnCount(4);
	QStringList header_labels;
	header_labels.insert(type_column, "Type");
	header_labels.insert(schema_column, "Schema");
	header_labels.insert(name_column, "Name");
	header_labels.insert(status_column, "Status");
	setHeaderLabels(header_labels);
	setRootIsDecorated(false);
	setSelectionMode(SingleSelection);
	header()->setStretchLastSection(false);
	header()->setSectionResizeMode(type_column, QHeaderView::ResizeMode::ResizeToContents);
	header()->setSectionResizeMode(schema_column, QHeaderView::ResizeMode::ResizeToContents);
	header()->setSectionResizeMode(name_column, QHeaderView::ResizeMode::Stretch);
	header()->setSectionResizeMode(status_column, QHeaderView::ResizeMode::ResizeToContents);
	setSortingEnabled(true);

	connect(this, SIGNAL(itemClicked(QTreeWidgetItem*, int)), SLOT(OnItemClicked(QTreeWidgetItem*, int)));
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
			current->setCheckState(status_column, Qt::Checked);
			current->setIcon(status_column, QIcon(status_icons.value(satisfied)));
			current->setData(status_column, Qt::UserRole, satisfied);
		}
		else
		{
			are_all_satisfied = false;
			current->setIcon(status_column, QIcon(status_icons.value(not_satisfied)));
			current->setData(status_column, Qt::UserRole, not_satisfied);
		}
	}

	emit ItemCheckChanged();
	return true;
}

// Adds a new dependency to list and marks it as waiting for check
void DependencyListWidget::Add(int type_index, const class QString &schema, const class QString &name)
{
	auto *new_item = new QTreeWidgetItem(this);
	new_item->setIcon(type_column, QIcon(ObjectTypes::type_icons.value(type_index)));
	new_item->setText(type_column, ObjectTypes::type_names.value(type_index));
	new_item->setData(type_column, Qt::UserRole, type_index);
	new_item->setText(schema_column, schema);
	new_item->setText(name_column, name);
	new_item->setIcon(status_column, QIcon(status_icons.value(waiting_for_check)));
	new_item->setCheckState(status_column, Qt::Unchecked);
	new_item->setData(status_column, Qt::UserRole, waiting_for_check);
	new_item->setFlags(Qt::ItemIsEnabled);
	addTopLevelItem(new_item);
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
		current->setCheckState(status_column, Qt::Unchecked);
		current->setIcon(status_column, QIcon(status_icons.value(waiting_for_check)));
		current->setData(status_column, Qt::UserRole, waiting_for_check);
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
	if (item->data(status_column, Qt::UserRole).toInt() == waiting_for_check)
	{
		return;
	}

	if (item->checkState(status_column) != Qt::Checked)
	{
		++checked_count;
		item->setCheckState(status_column, Qt::Checked);		
	}
	else
	{
		--checked_count;
		item->setCheckState(status_column, Qt::Unchecked);
	}

	emit ItemCheckChanged();
}