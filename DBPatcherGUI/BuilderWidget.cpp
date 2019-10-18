#include "BuilderWidget.h"
#include "ui_BuilderWidget.h"
#include "PatchListWidget.h"
#include "PatchList.h"
#include "DatabaseProvider.h"
#include "ObjectTypes.h"
#include "BuilderHandler.h"
#include "ObjectNameCompleter.h"
#include "FileHandler.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QSqlQueryModel>

// Widget constructor, taking pointer to parent widget
// When parent widget is being deleted, all its children are deleted automatically
BuilderWidget::BuilderWidget(QWidget *parent)
	: QWidget(parent)
	, ui(new Ui::BuilderWidget)
	, schema_list_model(new QSqlQueryModel(this))
	, name_completer(new ObjectNameCompleter(this))
{
	ui->setupUi(this);

	// Initialization of ui elements
	ui->schema_combo_box->setModel(schema_list_model);
	ui->move_up_button->setDisabled(true);
	ui->move_down_button->setDisabled(true);
	ui->remove_button->setDisabled(true);
	ui->clear_button->setDisabled(true);
	ui->build_button->setDisabled(true);

	// Filling type box with elements
	ui->type_combo_box->addItem(QIcon(":/images/script.svg"), "script", ObjectTypes::script);
	ui->type_combo_box->addItem(QIcon(":/images/table.svg"), "table", ObjectTypes::table);
	ui->type_combo_box->addItem(QIcon(":/images/sequence.svg"), "sequence", ObjectTypes::sequence);
	ui->type_combo_box->addItem(QIcon(":/images/function.svg"), "function", ObjectTypes::function);
	ui->type_combo_box->addItem(QIcon(":/images/view.svg"), "view", ObjectTypes::view);
	ui->type_combo_box->addItem(QIcon(":/images/trigger.svg"), "trigger", ObjectTypes::trigger);
	ui->type_combo_box->addItem(QIcon(":/images/index.svg"), "index", ObjectTypes::index);

	InitScriptInput();
 
	connect(ui->name_edit, &QLineEdit::returnPressed, [=]()
	{
		if (!name_completer->popup()->isVisible())
		{
			OnAddButtonClicked();
		}
	});

	connect(ui->add_button, &QPushButton::clicked, this, &BuilderWidget::OnAddButtonClicked);
	connect(ui->build_button, &QPushButton::clicked, this, &BuilderWidget::OnBuildButtonClicked);
	connect(ui->remove_button, &QPushButton::clicked, this, &BuilderWidget::OnRemoveButtonClicked);
	connect(ui->build_list_widget, &PatchListWidget::itemSelectionChanged, this, &BuilderWidget::OnItemSelectionChanged);
	connect(ui->type_combo_box, SIGNAL(currentIndexChanged(int)), this, SLOT(OnCurrentTypeChanged(int)));
	connect(ui->move_up_button, &QPushButton::clicked, this, &BuilderWidget::OnMoveUpButtonClicked);
	connect(ui->move_down_button, &QPushButton::clicked, this, &BuilderWidget::OnMoveDownButtonClicked);
	connect(ui->clear_button, &QPushButton::clicked, this, &BuilderWidget::OnClearButtonClicked);
	connect(ui->explorer_button, &QPushButton::clicked, this, &BuilderWidget::OnExplorerButtonClicked);
	connect(ui->name_edit, SIGNAL(textChanged(const QString&)), this, SLOT(OnNameTextChanged(const QString&)));
	connect(this, &BuilderWidget::ItemCountChanged, &BuilderWidget::OnItemCountChanged);
	connect(ui->schema_combo_box, SIGNAL(currentTextChanged(const QString&)), this, SLOT(OnCurrentSchemaChanged(const QString&)));
}

// Destructor with ui object deleting
BuilderWidget::~BuilderWidget()
{
	delete ui;
}

// Checks database connection, shows error message and requests connection
bool BuilderWidget::CheckConnection()
{
	if (!DatabaseProvider::IsConnected())
	{
		QApplication::beep();
		QMessageBox::warning(this, "Database error", "No connection to database"
			, QMessageBox::Ok, QMessageBox::Ok);
		emit ConnectionRequested();
		return false;
	}

	return true;
}

// Handles add button click
// Checks the availability of new element addition and calls add methods
void BuilderWidget::OnAddButtonClicked()
{
	if (!CheckConnection())
	{
		return;
	}
	
	const auto type_index = ui->type_combo_box->currentData(Qt::UserRole).toInt();
	const auto schema = ui->schema_combo_box->currentText();
	const auto name_input = ui->name_edit->text().remove(QRegExp("\\ "));

	if (type_index == ObjectTypes::script)
	{
		AddScripts(name_input);
		return;
	}

	if (name_input.isEmpty())
	{
		QMessageBox::information(this, "Item not added", "Please, enter "
			+ ui->name_edit->placeholderText().toLower()
			, QMessageBox::Ok, QMessageBox::Ok);
		return;
	}

	if (ui->build_list_widget->ItemExists(type_index, schema, name_input))
	{
		QApplication::beep();
		QMessageBox::warning(this, "Item not added"
			, ui->type_combo_box->currentText().replace(0, 1, ui->type_combo_box->currentText()[0].toUpper())
			+ " " + ui->name_edit->text() + " already exists in patch list"
			, QMessageBox::Ok, QMessageBox::Ok);
		return;
	}

	auto exists = false;

	switch (ui->type_combo_box->currentData(Qt::UserRole).toInt())
	{
		case ObjectTypes::table:
		{
			exists = DatabaseProvider::TableExists(schema, name_input);
			break;
		}
		case ObjectTypes::sequence:
		{
			exists = DatabaseProvider::SequenceExists(schema, name_input);
			break;
		}
		case ObjectTypes::view:
		{
			exists = DatabaseProvider::ViewExists(schema, name_input);
			break;
		}
		case ObjectTypes::trigger:
		{
			exists = DatabaseProvider::TriggerExists(schema, name_input);
			break;
		}
		case ObjectTypes::function:
		{
			exists = DatabaseProvider::FunctionExists(schema, name_input);
			break;
		}
		case ObjectTypes::index:
		{
			exists = DatabaseProvider::IndexExists(schema, name_input);
			break;
		}
	}

	if (exists)
	{
		ui->build_list_widget->Add(type_index, schema, name_input, true);
		ui->name_edit->clear();
		emit ItemCountChanged();
	}
	else
	{
		QApplication::beep();
		QMessageBox::warning(this, "Item not added"
			, ui->type_combo_box->currentText().replace(0, 1, ui->type_combo_box->currentText()[0].toUpper())
				+ " " + name_input + " does not exist in current schema"
			, QMessageBox::Ok, QMessageBox::Ok);
	}
}

// Parses script names string if it is not empty, or opens file dialog otherwise
// Adds parsed script objects to the list widget
void BuilderWidget::AddScripts(const QString &input)
{
	QStringList file_list;

	auto all_scripts_exist = true;

	if (input.isEmpty())
	{
		file_list = QFileDialog::getOpenFileNames(this, "Open script files", "", "SQL Script Files (*.sql)");
	}
	else
	{
		const auto script_paths = input.split(QRegExp("\\,"), QString::SkipEmptyParts);

		for (const auto &current : script_paths)
		{
			const QFileInfo file_info(current);

			if (file_info.exists() && file_info.suffix() == "sql")
			{
				file_list.append(file_info.filePath());
			}
			else
			{
				all_scripts_exist = false;
			}
		}
	}

	for (const auto &current : file_list)
	{
		if (!ui->build_list_widget->ItemExists(ObjectTypes::script, "", current))
		{
			ui->build_list_widget->Add(ObjectTypes::script, "", current, true);
		}
		else
		{
			all_scripts_exist = false;
		}
	}

	if (!all_scripts_exist)
	{
		QApplication::beep();
		QMessageBox::warning(this, "Some scripts not added"
			, "Some files were not added because they already exist in the patch list, not found or not a SQL-script (*.sql)"
			, QMessageBox::Ok, QMessageBox::Ok);
	}
	else
	{
		ui->name_edit->clear();
	}

	emit ItemCountChanged();
}

// Initializes ui elements for script path input
void BuilderWidget::InitScriptInput()
{
	ui->schema_combo_box->setDisabled(true);
	ui->name_edit->setPlaceholderText("SQL script file path (leave empty to open in explorer)");
	ui->name_label->setText("Path");
}

// Updates name completer from database by schema name and type index 
void BuilderWidget::InitCompleter()
{
	if (!DatabaseProvider::IsConnected())
	{
		return;
	}

	if (ui->type_combo_box->currentData(Qt::UserRole) == ObjectTypes::script)
	{
		name_completer->Clear();
		ui->name_edit->setCompleter(nullptr);
		return;
	}

	name_completer->Fetch(ui->type_combo_box->currentData(Qt::UserRole).toInt(), ui->schema_combo_box->currentText());
	ui->name_edit->setCompleter(name_completer);
}

// Handles open explorer button click
void BuilderWidget::OnExplorerButtonClicked()
{
	const auto path_input = QFileDialog::getExistingDirectory(this, "Choose build directory");
	if (!path_input.isEmpty())
	{
		ui->patch_path_edit->setText(path_input);
	}
}

// Handles build button click, calls build method
void BuilderWidget::OnBuildButtonClicked()
{
	if (!CheckConnection())
	{
		return;
	}

	if (ui->patch_path_edit->text().isEmpty())
	{
		QMessageBox::information(this, "Build error", "Please, choose target directory"
			, QMessageBox::Ok, QMessageBox::Ok);
		OnExplorerButtonClicked();
		return;
	}

	QDir patch_dir;
	patch_dir.setPath(ui->patch_path_edit->text());

	if (!patch_dir.exists())
	{
		QApplication::beep();
		QMessageBox::warning(this, "Build error", "Target directory does not exist"
			, QMessageBox::Ok, QMessageBox::Ok);
		return;
	}

	const auto dialog_result = QMessageBox::information(this, "Scripts installation order in the patch"
		, "For successful patch installation it is recommended to save the following order in a patch list:\n\n"
		"- sequences\n"
		"- tables\n"
		"- views\n"
		"- indexes\n"
		"- functions\n"
		"- triggers\n"
		"- scripts\n\n"
		"Are you sure to continue?"
		, QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Cancel);

	if (dialog_result == QMessageBox::Ok)
	{
		if (StartPatchBuild(ui->patch_path_edit->text()))
		{
			QApplication::beep();
			QMessageBox::information(this, "Build completed"
				, "Build completed. See log for details"
				, QMessageBox::Ok, QMessageBox::Ok);
		}
		else
		{
			QApplication::beep();
			QMessageBox::warning(this, "Build error"
				, "Error occured. See log for details"
				, QMessageBox::Ok, QMessageBox::Ok);			
		}
	}
}

// Handles remove item button click
void BuilderWidget::OnRemoveButtonClicked()
{
	const auto dialog_result = QMessageBox::question(this, "Remove item", "Are you sure to remove " +
		ui->build_list_widget->currentItem()->text(PatchListWidget::ColumnIndexes::name_column) +
		" from patch list?"
		, QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Cancel);

	if (dialog_result == QMessageBox::Ok && ui->build_list_widget->topLevelItemCount() != 0)
	{
		ui->build_list_widget->takeTopLevelItem(ui->build_list_widget->currentIndex().row());
		emit ItemCountChanged();
	}
}

// Handles move item up button click
void BuilderWidget::OnMoveUpButtonClicked()
{
	if (ui->build_list_widget->topLevelItemCount() > 1 && ui->build_list_widget->currentIndex().row() > 0)
	{
		const auto selected_row = ui->build_list_widget->currentIndex().row();
		const auto selected_item = ui->build_list_widget->takeTopLevelItem(ui->build_list_widget->currentIndex().row());
		ui->build_list_widget->insertTopLevelItem(selected_row - 1, selected_item);
		ui->build_list_widget->setCurrentItem(selected_item);
	}	
}

// Handles move item down button click
void BuilderWidget::OnMoveDownButtonClicked()
{
	if (ui->build_list_widget->topLevelItemCount() > 1 && ui->build_list_widget->currentIndex().row() != ui->build_list_widget->topLevelItemCount() - 1)
	{
		const auto selected_row = ui->build_list_widget->currentIndex().row();
		const auto selected_item = ui->build_list_widget->takeTopLevelItem(selected_row);
		ui->build_list_widget->insertTopLevelItem(selected_row + 1, selected_item);
		ui->build_list_widget->setCurrentItem(selected_item);
	}	
}

// Handles clear build list button click
void BuilderWidget::OnClearButtonClicked()
{
	const auto dialog_result = QMessageBox::question(this, "Clear list", "Are you sure to clear patch list?"
		, QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Cancel);

	if (dialog_result == QMessageBox::Ok && ui->build_list_widget->topLevelItemCount() != 0)
	{
		ui->build_list_widget->clear();
		emit ItemCountChanged();
	}
}

// Handles list elements selection state change
// Enables operations with list elements if one of them is selected
void BuilderWidget::OnItemSelectionChanged()
{
	if (ui->build_list_widget->selectedItems().isEmpty())
	{
		ui->move_up_button->setDisabled(true);
		ui->move_down_button->setDisabled(true);
		ui->remove_button->setDisabled(true);
	}
	else
	{
		ui->move_up_button->setEnabled(true);
		ui->move_down_button->setEnabled(true);
		ui->remove_button->setEnabled(true);
	}
}

// Handles current type change
// Sets ui elements for object name input by selected type
void BuilderWidget::OnCurrentTypeChanged(int type)
{
	InitCompleter();

	if (type == ObjectTypes::function)
	{
		ui->name_edit->setPlaceholderText("Function signature (e.g. function(arg_1,arg_2))");
		ui->name_label->setText("Signature (Invalid. Function may not be found in database)");
		emit ui->name_edit->textChanged(ui->name_edit->text());
	}

	if (type == ObjectTypes::script)
	{
		InitScriptInput();
	}
	else if (!ui->schema_combo_box->isEnabled())
	{
		ui->schema_combo_box->setEnabled(true);
	}

	if (type != ObjectTypes::function && type != ObjectTypes::script)
	{
		ui->name_edit->setPlaceholderText(ui->type_combo_box->currentText().replace(0, 1, ui->type_combo_box->currentText()[0].toUpper())
			+ " name");
		ui->name_label->setText("Name");
	}
}

// Handles current schema change
void BuilderWidget::OnCurrentSchemaChanged(const QString &schema)
{
	InitCompleter();
}

// Handles current name input change
// If it is a function, checks its signature with regular expression
void BuilderWidget::OnNameTextChanged(const QString &input)
{
	if (ui->type_combo_box->currentData(Qt::UserRole).toInt() != ObjectTypes::function)
	{
		return;
	}

	ui->name_label->setText(QRegExp("[^,\\(\\) ]+\\((([^,\\(\\) ]+,)*([^, \\(\\)]+)+)?\\)").exactMatch(input) ? "Signature (Valid)" : "Signature (Invalid. Function may not be found in database)");
}

// Handles amount of build list elements change
// Enables build and clear options if the list is not empty
void BuilderWidget::OnItemCountChanged()
{
	if (ui->build_list_widget->topLevelItemCount() == 0)
	{
		ui->clear_button->setDisabled(true);
		ui->build_button->setDisabled(true);
	}
	else if (!ui->clear_button->isEnabled())
	{
		ui->clear_button->setEnabled(true);
		ui->build_button->setEnabled(true);
	}
}

// Handles connection to database
// Initializes elements which depend on database
void BuilderWidget::OnConnected()
{
	DatabaseProvider::InitSchemaListModel(*schema_list_model);
	name_completer->Initialize();
	InitCompleter();
}

// Handles start of disconnection from database
// Clears elements which depend on database
void BuilderWidget::OnDisconnectionStarted()
{
	schema_list_model->clear();
	name_completer->Finish();
	ui->name_edit->setCompleter(nullptr);
	ui->build_list_widget->clear();
	ui->build_button->setDisabled(true);
	ui->clear_button->setDisabled(true);
}

// Launches patch build
bool BuilderWidget::StartPatchBuild(const QString &path)
{
	auto is_successful = false;
	const auto patch_dir = FileHandler::MakePatchDir(path, is_successful);

	if (!is_successful)
	{
		return false;
	}

	PatchList build_list;

	for (auto i = 0; i < ui->build_list_widget->topLevelItemCount(); ++i)
	{
		const auto current_item = ui->build_list_widget->topLevelItem(i);
		auto name_split_result = current_item->text(PatchListWidget::ColumnIndexes::name_column).split(QRegExp("(\\ |\\,|\\(|\\))")
			, QString::SkipEmptyParts);
		const auto item_name = name_split_result.first();
		name_split_result.pop_front();
		build_list.Add(current_item->data(PatchListWidget::ColumnIndexes::type_column, Qt::UserRole).toInt()
			, current_item->text(PatchListWidget::ColumnIndexes::schema_column)
			, item_name, name_split_result);
	}

	if (!FileHandler::MakePatchList(patch_dir.absolutePath(), build_list))
	{
		return false;
	}

	const auto build_result = BuilderHandler::BuildPatch(DatabaseProvider::Database(), DatabaseProvider::User(), DatabaseProvider::Password()
		, DatabaseProvider::Host(), DatabaseProvider::Port(), patch_dir.absolutePath(), patch_dir.absoluteFilePath(FileHandler::GetPatchListName()));

	QFile::remove(patch_dir.absoluteFilePath(FileHandler::GetPatchListName()));

	return build_result;
}