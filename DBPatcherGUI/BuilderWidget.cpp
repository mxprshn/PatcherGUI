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
	, schemaListModel(new QSqlQueryModel(this))
	, nameCompleter(new ObjectNameCompleter(this))
{
	ui->setupUi(this);

	// Initialization of ui elements
	ui->schemaComboBox->setModel(schemaListModel);
	ui->moveUpButton->setDisabled(true);
	ui->moveDownButton->setDisabled(true);
	ui->removeButton->setDisabled(true);
	ui->clearButton->setDisabled(true);
	ui->buildButton->setDisabled(true);

	// Filling type box with elements
	ui->typeComboBox->addItem(QIcon(":/images/script.svg"), "script", ObjectTypes::script);
	ui->typeComboBox->addItem(QIcon(":/images/table.svg"), "table", ObjectTypes::table);
	ui->typeComboBox->addItem(QIcon(":/images/sequence.svg"), "sequence", ObjectTypes::sequence);
	ui->typeComboBox->addItem(QIcon(":/images/function.svg"), "function", ObjectTypes::function);
	ui->typeComboBox->addItem(QIcon(":/images/view.svg"), "view", ObjectTypes::view);
	ui->typeComboBox->addItem(QIcon(":/images/trigger.svg"), "trigger", ObjectTypes::trigger);
	ui->typeComboBox->addItem(QIcon(":/images/index.svg"), "index", ObjectTypes::index);

	initScriptInput();
 
	connect(ui->nameEdit, &QLineEdit::returnPressed, [=]()
	{
		if (!nameCompleter->popup()->isVisible())
		{
			onAddButtonClicked();
		}
	});

	connect(ui->addButton, SIGNAL(clicked()), this, SLOT(onAddButtonClicked()));
	connect(ui->buildButton, SIGNAL(clicked()), this, SLOT(onBuildButtonClicked()));
	connect(ui->removeButton, SIGNAL(clicked()), this, SLOT(onRemoveButtonClicked()));
	connect(ui->buildListWidget, SIGNAL(itemSelectionChanged()), this, SLOT(onItemSelectionChanged()));
	connect(ui->typeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onCurrentTypeChanged(int)));
	connect(ui->moveUpButton, SIGNAL(clicked()), this, SLOT(onMoveUpButtonClicked()));
	connect(ui->moveDownButton, SIGNAL(clicked()), this, SLOT(onMoveDownButtonClicked()));
	connect(ui->clearButton, SIGNAL(clicked()), this, SLOT(onClearButtonClicked()));
	connect(ui->explorerButton, SIGNAL(clicked()), this, SLOT(onExplorerButtonClicked()));
	connect(ui->nameEdit, SIGNAL(textChanged(const QString&)), this, SLOT(onNameTextChanged(const QString&)));
	connect(this, SIGNAL(itemCountChanged()), SLOT(onItemCountChanged()));
	connect(ui->schemaComboBox, SIGNAL(currentTextChanged(const QString&)), this, SLOT(onCurrentSchemaChanged(const QString&)));
}

// Destructor with ui object deleting
BuilderWidget::~BuilderWidget()
{
	delete ui;
}

// Checks database connection, shows error message and requests connection
bool BuilderWidget::checkConnection()
{
	if (!DatabaseProvider::isConnected())
	{
		QApplication::beep();
		QMessageBox::warning(this, "Database error", "Not connected to database."
			, QMessageBox::Ok, QMessageBox::Ok);
		emit connectionRequested();
		return false;
	}

	return true;
}

// Handles add button click
// Checks the availability of new element addition and calls add methods
void BuilderWidget::onAddButtonClicked()
{
	if (!checkConnection())
	{
		return;
	}
	
	const auto typeIndex = ui->typeComboBox->currentData(Qt::UserRole).toInt();
	const auto schema = ui->schemaComboBox->currentText();
	const auto nameInput = ui->nameEdit->text().remove(QRegExp("\\ "));

	if (typeIndex == ObjectTypes::script)
	{
		addScripts(nameInput);
		return;
	}

	if (nameInput.isEmpty())
	{
		QMessageBox::information(this, "Item not added", "Please, enter "
			+ ui->nameEdit->placeholderText().toLower() + "."
			, QMessageBox::Ok, QMessageBox::Ok);
		return;
	}

	if (ui->buildListWidget->itemExists(typeIndex, schema, nameInput))
	{
		QApplication::beep();
		QMessageBox::warning(this, "Item not added"
			, ui->typeComboBox->currentText().replace(0, 1, ui->typeComboBox->currentText()[0].toUpper())
			+ " " + ui->nameEdit->text() + " already exists in patch list."
			, QMessageBox::Ok, QMessageBox::Ok);
		return;
	}

	auto exists = false;

	switch (ui->typeComboBox->currentData(Qt::UserRole).toInt())
	{
		case ObjectTypes::table:
		{
			exists = DatabaseProvider::tableExists(schema, nameInput);
			break;
		}
		case ObjectTypes::sequence:
		{
			exists = DatabaseProvider::sequenceExists(schema, nameInput);
			break;
		}
		case ObjectTypes::view:
		{
			exists = DatabaseProvider::viewExists(schema, nameInput);
			break;
		}
		case ObjectTypes::trigger:
		{
			exists = DatabaseProvider::triggerExists(schema, nameInput);
			break;
		}
		case ObjectTypes::function:
		{
			exists = DatabaseProvider::functionExists(schema, nameInput);
			break;
		}
		case ObjectTypes::index:
		{
			exists = DatabaseProvider::indexExists(schema, nameInput);
			break;
		}
	}

	if (exists)
	{
		ui->buildListWidget->add(typeIndex, schema, nameInput, true);
		ui->nameEdit->clear();
		emit itemCountChanged();
	}
	else
	{
		QApplication::beep();
		QMessageBox::warning(this, "Item not added"
			, ui->typeComboBox->currentText().replace(0, 1, ui->typeComboBox->currentText()[0].toUpper())
				+ " " + nameInput + " does not exist in current schema."
			, QMessageBox::Ok, QMessageBox::Ok);
	}
}

// Parses script names string if it is not empty, or opens file dialog otherwise
// Adds parsed script objects to the list widget
void BuilderWidget::addScripts(const QString &input)
{
	QStringList fileList;

	auto allScriptsExist = true;

	if (input.isEmpty())
	{
		fileList = QFileDialog::getOpenFileNames(this, "Open script files", "", "SQL Script Files (*.sql)");
	}
	else
	{
		const auto scriptPaths = input.split(QRegExp("\\,"), QString::SkipEmptyParts);

		for (const auto &current : scriptPaths)
		{
			const QFileInfo fileInfo(current);

			if (fileInfo.exists() && fileInfo.suffix() == "sql")
			{
				fileList.append(fileInfo.filePath());
			}
			else
			{
				allScriptsExist = false;
			}
		}
	}

	for (const auto &current : fileList)
	{
		if (!ui->buildListWidget->itemExists(ObjectTypes::script, "", current))
		{
			ui->buildListWidget->add(ObjectTypes::script, "", current, true);
		}
		else
		{
			allScriptsExist = false;
		}
	}

	if (!allScriptsExist)
	{
		QApplication::beep();
		QMessageBox::warning(this, "Some scripts not added"
			, "Some script files already exist in patch list, not found or are not SQL script files (*.sql) and were not added."
			, QMessageBox::Ok, QMessageBox::Ok);
	}
	else
	{
		ui->nameEdit->clear();
	}

	emit itemCountChanged();
}

// Initializes ui elements for script path input
void BuilderWidget::initScriptInput()
{
	ui->schemaComboBox->setDisabled(true);
	ui->nameEdit->setPlaceholderText("SQL script file paths (leave empty to open in explorer)");
	ui->nameLabel->setText("Path");
}

// Updates name completer from database by schema name and type index 
void BuilderWidget::initCompleter()
{
	if (!DatabaseProvider::isConnected())
	{
		return;
	}

	if (ui->typeComboBox->currentData(Qt::UserRole) == ObjectTypes::script)
	{
		nameCompleter->clear();
		ui->nameEdit->setCompleter(nullptr);
		return;
	}

	nameCompleter->initialize(ui->typeComboBox->currentData(Qt::UserRole).toInt(), ui->schemaComboBox->currentText());
	ui->nameEdit->setCompleter(nameCompleter);
}

// Handles open explorer button click
void BuilderWidget::onExplorerButtonClicked()
{
	ui->patchPathEdit->setText(QFileDialog::getExistingDirectory(this, "Choose build directory"));
}

// Handles build button click, calls build method
void BuilderWidget::onBuildButtonClicked()
{
	if (!checkConnection())
	{
		return;
	}

	if (ui->patchPathEdit->text().isEmpty())
	{
		QMessageBox::information(this, "Build error", "Please, choose target directory."
			, QMessageBox::Ok, QMessageBox::Ok);
		onExplorerButtonClicked();
		return;
	}

	QDir patchDir;
	patchDir.setPath(ui->patchPathEdit->text());

	if (!patchDir.exists())
	{
		QApplication::beep();
		QMessageBox::warning(this, "Build error", "Target directory does not exist."
			, QMessageBox::Ok, QMessageBox::Ok);
		return;
	}

	const auto dialogResult = QMessageBox::information(this, "Patch list order information"
		, "For successful further installation of the built script it is recommended to keep the following"
		" order of patch list:\n\n"
		"- sequences\n"
		"- tables\n"
		"- views\n"
		"- indexes\n"
		"- functions\n"
		"- triggers\n"
		"- scripts\n\n"
		"Are you sure to continue?"
		, QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Cancel);

	if (dialogResult == QMessageBox::Ok)
	{
		if (startPatchBuild(ui->patchPathEdit->text()))
		{
			QApplication::beep();
			QMessageBox::information(this, "Build completed"
				, "Build completed. See log for detailed information."
				, QMessageBox::Ok, QMessageBox::Ok);
		}
		else
		{
			QApplication::beep();
			QMessageBox::warning(this, "Build error"
				, "Error occured. See log for detailed information."
				, QMessageBox::Ok, QMessageBox::Ok);			
		}
	}
}

// Handles remove item button click
void BuilderWidget::onRemoveButtonClicked()
{
	const auto dialogResult = QMessageBox::question(this, "Remove item", "Are you sure to remove " +
		ui->buildListWidget->currentItem()->text(PatchListWidget::ColumnIndexes::nameColumn) +
		" from patch list?"
		, QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Cancel);

	if (dialogResult == QMessageBox::Ok && ui->buildListWidget->topLevelItemCount() != 0)
	{
		ui->buildListWidget->takeTopLevelItem(ui->buildListWidget->currentIndex().row());
		emit itemCountChanged();
	}
}

// Handles move item up button click
void BuilderWidget::onMoveUpButtonClicked()
{
	if (ui->buildListWidget->topLevelItemCount() > 1 && ui->buildListWidget->currentIndex().row() > 0)
	{
		const auto selectedRow = ui->buildListWidget->currentIndex().row();
		const auto selectedItem = ui->buildListWidget->takeTopLevelItem(ui->buildListWidget->currentIndex().row());
		ui->buildListWidget->insertTopLevelItem(selectedRow - 1, selectedItem);
		ui->buildListWidget->setCurrentItem(selectedItem);
	}	
}

// Handles move item down button click
void BuilderWidget::onMoveDownButtonClicked()
{
	if (ui->buildListWidget->topLevelItemCount() > 1 && ui->buildListWidget->currentIndex().row() != ui->buildListWidget->topLevelItemCount() - 1)
	{
		const auto selectedRow = ui->buildListWidget->currentIndex().row();
		const auto selectedItem = ui->buildListWidget->takeTopLevelItem(selectedRow);
		ui->buildListWidget->insertTopLevelItem(selectedRow + 1, selectedItem);
		ui->buildListWidget->setCurrentItem(selectedItem);
	}	
}

// Handles clear build list button click
void BuilderWidget::onClearButtonClicked()
{
	const auto dialogResult = QMessageBox::question(this, "Clear list", "Are you sure to clear patch list?"
		, QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Cancel);

	if (dialogResult == QMessageBox::Ok && ui->buildListWidget->topLevelItemCount() != 0)
	{
		ui->buildListWidget->clear();
		emit itemCountChanged();
	}
}

// Handles list elements selection state change
// Enables operations with list elements if one of them is selected
void BuilderWidget::onItemSelectionChanged()
{
	if (ui->buildListWidget->selectedItems().isEmpty())
	{
		ui->moveUpButton->setDisabled(true);
		ui->moveDownButton->setDisabled(true);
		ui->removeButton->setDisabled(true);
	}
	else
	{
		ui->moveUpButton->setEnabled(true);
		ui->moveDownButton->setEnabled(true);
		ui->removeButton->setEnabled(true);
	}
}

// Handles current type change
// Sets ui elements for object name input by selected type
void BuilderWidget::onCurrentTypeChanged(int type)
{
	initCompleter();

	if (type == ObjectTypes::function)
	{
		ui->nameEdit->setPlaceholderText("Function signature (e.g. function(arg_1,arg_2))");
		ui->nameLabel->setText("Signature (Invalid, function may not be found))");
		emit ui->nameEdit->textChanged(ui->nameEdit->text());
	}

	if (type == ObjectTypes::script)
	{
		initScriptInput();
	}
	else if (!ui->schemaComboBox->isEnabled())
	{
		ui->schemaComboBox->setEnabled(true);
	}

	if (type != ObjectTypes::function && type != ObjectTypes::script)
	{
		ui->nameEdit->setPlaceholderText(ui->typeComboBox->currentText().replace(0, 1, ui->typeComboBox->currentText()[0].toUpper())
			+ " name");
		ui->nameLabel->setText("Name");
	}
}

// Handles current schema change
void BuilderWidget::onCurrentSchemaChanged(const QString& schema)
{
	initCompleter();
}

// Handles current name input change
// If it is a function, checks its signature with regular expression
void BuilderWidget::onNameTextChanged(const QString &input)
{
	if (ui->typeComboBox->currentData(Qt::UserRole).toInt() != ObjectTypes::function)
	{
		return;
	}

	ui->nameLabel->setText(QRegExp("[^,\\(\\) ]+\\((([^,\\(\\) ]+,)*([^, \\(\\)]+)+)?\\)").exactMatch(input) ? "Signature (Valid)" : "Signature (Invalid, function may not be found)");
}

// Handles amount of build list elements change
// Enables build and clear options if the list is not empty
void BuilderWidget::onItemCountChanged()
{
	if (ui->buildListWidget->topLevelItemCount() == 0)
	{
		ui->clearButton->setDisabled(true);
		ui->buildButton->setDisabled(true);
	}
	else if (!ui->clearButton->isEnabled())
	{
		ui->clearButton->setEnabled(true);
		ui->buildButton->setEnabled(true);
	}
}

// Handles connection to database
// Initializes elements which depend on database
void BuilderWidget::onConnected()
{
	DatabaseProvider::initSchemaListModel(*schemaListModel);
	initCompleter();
}

// Handles start of disconnection from database
// Clears elements which depend on database
void BuilderWidget::onDisconnectionStarted()
{
	schemaListModel->clear();
	nameCompleter->clear();
	ui->nameEdit->setCompleter(nullptr);
}

// Launches patch build
bool BuilderWidget::startPatchBuild(const QString &path)
{
	auto isSuccessful = false;
	const auto patchDir = FileHandler::makePatchDir(path, isSuccessful);

	if (!isSuccessful)
	{
		return false;
	}

	PatchList buildList;

	for (auto i = 0; i < ui->buildListWidget->topLevelItemCount(); ++i)
	{
		const auto currentItem = ui->buildListWidget->topLevelItem(i);
		auto nameSplitResult = currentItem->text(PatchListWidget::ColumnIndexes::nameColumn).split(QRegExp("(\\ |\\,|\\(|\\))")
			, QString::SkipEmptyParts);
		const auto itemName = nameSplitResult.first();
		nameSplitResult.pop_front();
		buildList.add(currentItem->data(PatchListWidget::ColumnIndexes::typeColumn, Qt::UserRole).toInt()
			, currentItem->text(PatchListWidget::ColumnIndexes::schemaColumn)
			, itemName, nameSplitResult);
	}

	if (!FileHandler::makePatchList(patchDir.absolutePath(), buildList))
	{
		return false;
	}

	return BuilderHandler::buildPatch(DatabaseProvider::database(), DatabaseProvider::user(), DatabaseProvider::password()
		, DatabaseProvider::host(), DatabaseProvider::port(), patchDir.absolutePath(), patchDir.absoluteFilePath(FileHandler::getPatchListName()));
}