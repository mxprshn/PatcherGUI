#include "InstallerWidget.h"
#include "ui_InstallerWidget.h"
#include "InstallerHandler.h"
#include "PatchListWidget.h"
#include "DependencyListWidget.h"
#include "PatchList.h"
#include "PatchListElement.h"
#include "ObjectTypes.h"
#include "DatabaseProvider.h"
#include "FileHandler.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QBitArray>

// Widget constructor, taking pointer to parent widget
// When parent widget is being deleted, all its children are deleted automatically
InstallerWidget::InstallerWidget(QWidget *parent)
	: QWidget(parent)
	, ui(new Ui::InstallerWidget)
	, is_patch_opened(false)
{
	ui->setupUi(this);

	ui->install_info_label->setText("");
	ui->check_button->setDisabled(true);
	ui->install_button->setDisabled(true);
	SetReadyToOpen();

	connect(ui->check_button, &QPushButton::clicked, this, &InstallerWidget::OnCheckButtonClicked);
	connect(ui->install_button, &QPushButton::clicked, this, &InstallerWidget::OnInstallButtonClicked);
	connect(ui->open_patch_button, &QPushButton::clicked, this, &InstallerWidget::OnOpenButtonClicked);
	connect(ui->dependency_list_widget, &DependencyListWidget::ItemCheckChanged, this, &InstallerWidget::OnItemCheckChanged);
}

// Destructor with ui object deleting
InstallerWidget::~InstallerWidget()
{
	delete ui;
}

// Checks database connection, shows error message and requests connection
bool InstallerWidget::CheckConnection()
{
	if (!DatabaseProvider::IsConnected())
	{
		QApplication::beep();
		QMessageBox::warning(this, "Database error"
			, "No connection to database"
			, QMessageBox::Ok, QMessageBox::Ok);
		emit ConnectionRequested();
		return false;
	}

	return true;
}

// Sets elements of interface which is ready to open patch
void InstallerWidget::SetReadyToOpen()
{
	ui->patch_path_edit->setPlaceholderText("Patch folder path (leave empty to open in explorer)");
	ui->open_patch_button->setText("Open");
	ui->open_patch_button->setIcon(QIcon(":/images/box.svg"));
	ui->open_patch_button->setIconSize(QSize(20, 20));
}

// Fills list widget of patch objects with information from patch 
bool InstallerWidget::InitPatchList(const QString &path)
{
	auto is_successful = false;
	const auto object_list = FileHandler::ParseObjectList(path, is_successful);

	if (!is_successful)
	{
		return false;		
	}

	for (const auto current : object_list)
	{
		const auto type = current->GetType();
		ui->patch_list_widget->Add(type, current->GetSchema(), current->GetName()
			+ QString(type == ObjectTypes::function ? "(" + current->GetParameters().join(",") + ")" : ""), false);
	}

	ui->patch_list_widget->scrollToTop();
	return true;
}

// Fills list widget of dependencies with information from patch 
bool InstallerWidget::InitDependencyList(const QString &path)
{
	auto is_successful = false;
	const auto dependency_list = FileHandler::ParseDependencyList(path, is_successful);

	if(!is_successful)
	{
		return false;
	}

	for (const auto current : dependency_list)
	{
		ui->dependency_list_widget->Add(current->GetType(), current->GetSchema(), current->GetName());
	}

	return true;
}

// Sets all interface elements affected by patch opening to default state
void InstallerWidget::ClearCurrentPatch()
{
	patch_dir = QDir();
	ui->dependency_list_widget->Clear();
	ui->patch_list_widget->clear();
	ui->patch_path_edit->setPlaceholderText("Patch folder path");
	ui->patch_path_edit->setEnabled(true);
	ui->check_button->setDisabled(true);
	ui->install_button->setDisabled(true);
	ui->install_info_label->setText("");
	SetReadyToOpen();
	is_patch_opened = false;
}

// Handles open button click
// Opens patch list files if possible and sets interface elements to appropriate state
void InstallerWidget::OnOpenButtonClicked()
{
	if (is_patch_opened)
	{
		const auto dialog_result = QMessageBox::question(this, "Close", "Are you sure to close current patch?"
			, QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Cancel);

		if (dialog_result == QMessageBox::Ok)
		{
			ClearCurrentPatch();
		}
		
		return;
	}

	if (ui->patch_path_edit->text().isEmpty())
	{
		patch_dir.setPath(QFileDialog::getExistingDirectory(this, "Choose patch directory"));

		if (patch_dir.path().isEmpty())
		{
			ClearCurrentPatch();
			return;
		}
	}
	else
	{
		patch_dir.setPath(ui->patch_path_edit->text());

		if (!patch_dir.exists())
		{
			QApplication::beep();
			QMessageBox::warning(this, "Open error", "Patch directory does not exist"
				, QMessageBox::Ok, QMessageBox::Ok);
			ClearCurrentPatch();
			return;
		}
	}

	if (!patch_dir.exists(FileHandler::GetObjectListName()))
	{
		QApplication::beep();
		QMessageBox::warning(this, "Open error", FileHandler::GetObjectListName() + " does not exist in the patch directory"
			, QMessageBox::Ok, QMessageBox::Ok);
		ClearCurrentPatch();
		return;
	}

	if (!patch_dir.exists(FileHandler::GetDependencyListName()))
	{
		QApplication::beep();
		QMessageBox::warning(this, "Open error", FileHandler::GetDependencyListName() + " does not exist in the patch directory"
			, QMessageBox::Ok, QMessageBox::Ok);
		ClearCurrentPatch();
		return;
	}

	if (!InitPatchList(patch_dir.absolutePath()))
	{
		QApplication::beep();
		QMessageBox::warning(this, "Open error", "Incorrect file " + FileHandler::GetObjectListName()
			, QMessageBox::Ok, QMessageBox::Ok);
		ClearCurrentPatch();
		return;
	}

	if (!InitDependencyList(patch_dir.absolutePath()))
	{
		QApplication::beep();
		QMessageBox::warning(this, "Open error", "Incorrect file " + FileHandler::GetDependencyListName()
			, QMessageBox::Ok, QMessageBox::Ok);
		ClearCurrentPatch();
		return;
	}

	if (ui->dependency_list_widget->topLevelItemCount() == 0)
	{
		ui->install_info_label->setText("The patch has no dependencies");
		ui->install_button->setEnabled(true);
	}
	else
	{
		ui->check_button->setEnabled(true);
	}

	ui->patch_path_edit->clear();
	ui->patch_path_edit->setPlaceholderText("Opened patch: " + patch_dir.absolutePath());
	ui->patch_path_edit->setDisabled(true);
	ui->open_patch_button->setText("Close");
	ui->open_patch_button->setIcon(QIcon(":/images/close.svg"));
	ui->open_patch_button->setIconSize(QSize(12, 12));
	is_patch_opened = true;
}

// Handles check button click
// Launches dependency check and shows information about its result
void InstallerWidget::OnCheckButtonClicked()
{
	if (!CheckConnection())
	{
		return;
	}

	if (StartDependencyCheck())
	{
		ui->check_button->setDisabled(true);

		QApplication::beep();

		if (!ui->dependency_list_widget->GetAreAllSatisfied())
		{
			QMessageBox::warning(this, "Verification completed"
				, "Verification completed. Not all dependencies are found. If you want to continue the installation, "
				"confirm all dependencies manually in the list"
				, QMessageBox::Ok, QMessageBox::Ok);
		}
		else
		{
			QMessageBox::information(this, "Verification completed"
				, "Verification completed. All dependencies found. The patch can be installed safely... almost safely :)"
				, QMessageBox::Ok, QMessageBox::Ok);
		}
	}
	else
	{
		QApplication::beep();
		QMessageBox::warning(this, "Check error"
			, "Error occured. See log for details"
			, QMessageBox::Ok, QMessageBox::Ok);
	}
}

// Handles install button click
// Launches patch installation and shows information about its result
void InstallerWidget::OnInstallButtonClicked()
{
	if (!CheckConnection())
	{
		return;
	}

	if (!ui->dependency_list_widget->GetAreAllSatisfied())
	{
		QApplication::beep();
		const auto dialog_result = QMessageBox::warning(this, "Unsafe installation"
			,"WARNING: not all dependencies are found. Installation may cause database errors."
			" Are you sure you want to continue?"
			, QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Cancel);

		if (dialog_result == QMessageBox::Cancel)
		{
			return;
		}
	}

	if (InstallerHandler::InstallPatch(DatabaseProvider::Database()
		, DatabaseProvider::User(), DatabaseProvider::Password(), DatabaseProvider::Host()
		, DatabaseProvider::Port(), patch_dir.absolutePath()))
	{
		QApplication::beep();
		QMessageBox::information(this, "Installation completed"
			, "Installation completed. See log for details"
			, QMessageBox::Ok, QMessageBox::Ok);
	}
	else
	{
		QApplication::beep();
		QMessageBox::warning(this, "Installation error"
			, "Error occured. See log for details"
			, QMessageBox::Ok, QMessageBox::Ok);
	}
}

// Handles amount of checked dependencies change
// Shows appropriate information and enables install option if all dependencies are checked
void InstallerWidget::OnItemCheckChanged()
{
	if (ui->dependency_list_widget->GetCheckedCount() == ui->dependency_list_widget->topLevelItemCount())
	{
		if (!ui->dependency_list_widget->GetAreAllSatisfied())
		{
			ui->install_info_label->setText("Some dependencies are not found!");
		}
		else
		{
			ui->install_info_label->setText("All dependencies are found");
		}
		
		ui->install_button->setEnabled(true);
	}
	else
	{
		ui->install_info_label->setText("Confirm all dependencies manually in the list");
		ui->install_button->setEnabled(false);
	}
}

// Handles start of disconnection from database
void InstallerWidget::OnDisconnectionStarted()
{
	if (!is_patch_opened)
	{
		return;
	}

	ui->dependency_list_widget->ClearCheck();
	ui->check_button->setEnabled(true);
	ui->install_button->setDisabled(true);
	ui->install_info_label->setText("");
}

bool InstallerWidget::StartDependencyCheck()
{
	PatchList dependency_list;

	for (auto i = 0; i < ui->dependency_list_widget->topLevelItemCount(); ++i)
	{
		const auto current_item = ui->dependency_list_widget->topLevelItem(i);
		dependency_list.Add(current_item->data(PatchListWidget::ColumnIndexes::type_column, Qt::UserRole).toInt()
			, current_item->text(PatchListWidget::ColumnIndexes::schema_column)
			, current_item->text(PatchListWidget::ColumnIndexes::name_column), QStringList());
	}

	if (!FileHandler::MakeDependencyList(patch_dir.absolutePath(), dependency_list))
	{
		return false;
	}

	auto is_successful = false;
	const auto check_result = InstallerHandler::CheckDependencies(DatabaseProvider::Database(), DatabaseProvider::User(), DatabaseProvider::Password()
		, DatabaseProvider::Host(), DatabaseProvider::Port(), patch_dir.absolutePath(), is_successful);
	return is_successful && ui->dependency_list_widget->SetCheckStatus(check_result);
}