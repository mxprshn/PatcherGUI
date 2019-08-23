#pragma once

#include <QTreeWidget>

// Class implementing graphical interface for list of patch objects
class PatchListWidget : public QTreeWidget
{
	Q_OBJECT

public:

	enum ColumnIndexes
	{
		type_column,
		schema_column,
		name_column
	};

	PatchListWidget(QWidget *parent = nullptr);
	bool ItemExists(int type_index, const QString &schema, const QString &name);
	void Add(int type_index, const QString &schema, const QString &name, bool is_draggable);
private:
	void dropEvent(QDropEvent *event) override;
};