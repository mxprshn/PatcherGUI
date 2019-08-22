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

	ui->installInfoLabel->setText("");
	ui->checkButton->setDisabled(true);
	ui->installButton->setDisabled(true);
	SetReadyToOpen();

	connect(ui->checkButton, SIGNAL(clicked()), this, SLOT(onCheckButtonClicked()));
	connect(ui->installButton, SIGNAL(clicked()), this, SLOT(onInstallButtonClicked()));
	connect(ui->openPatchButton, SIGNAL(clicked()), this, SLOT(onOpenButtonClicked()));
	connect(ui->dependencyListWidget, SIGNAL(itemCheckChanged()), this, SLOT(onItemCheckChanged()));
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
	ui->patchPathEdit->setPlaceholderText("Patch folder path (leave empty to open in explorer)");
	ui->openPatchButton->setText("Open");
	ui->openPatchButton->setIcon(QIcon(":/images/box.svg"));
	ui->openPatchButton->setIconSize(QSize(20, 20));
}

// Fills list widget of patch objects with information from patch 
bool InstallerWidget::InitPatchList(const QString &path)
{
	auto isSuccessful = false;
	const auto objectList = FileHandler::ParseObjectList(path, isSuccessful);

	if (!isSuccessful)
	{
		return false;		
	}

	for (const auto current : objectList)
	{
		const auto type = current->getType();
		ui->patchListWidget->Add(type, current->getSchema(), current->getName()
			+ QString(type == ObjectTypes::function ? "(" + current->getParameters().join(",") + ")" : ""), false);
	}

	ui->patchListWidget->scrollToTop();
	return true;
}

// Fills list widget of dependencies with information from patch 
bool InstallerWidget::InitDependencyList(const QString &path)
{
	auto isSuccessful = false;
	const auto dependencyList = FileHandler::ParseDependencyList(path, isSuccessful);

	if(!isSuccessful)
	{
		return false;
	}

	for (const auto current : dependencyList)
	{
		ui->dependencyListWidget->Add(current->getType(), current->getSchema(), current->getName());
	}

	return true;
}

// Sets all interface elements affected by patch opening to default state
void InstallerWidget::ClearCurrentPatch()
{
	patch_dir = QDir();
	ui->dependencyListWidget->Clear();
	ui->patchListWidget->clear();
	ui->patchPathEdit->setPlaceholderText("Patch folder path");
	ui->patchPathEdit->setEnabled(true);
	ui->checkButton->setDisabled(true);
	ui->installButton->setDisabled(true);
	ui->installInfoLabel->setText("");
	SetReadyToOpen();
	is_patch_opened = false;
}

// Handles open button click
// Opens patch list files if possible and sets interface elements to appropriate state
void InstallerWidget::OnOpenButtonClicked()
{
	if (is_patch_opened)
	{
		const auto dialogResult = QMessageBox::question(this, "Close", "Are you sure to close current patch?"
			, QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Cancel);

		if (dialogResult == QMessageBox::Ok)
		{
			ClearCurrentPatch();
		}
		
		return;
	}

	if (ui->patchPathEdit->text().isEmpty())
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
		patch_dir.setPath(ui->patchPathEdit->text());

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

	if (ui->dependencyListWidget->topLevelItemCount() == 0)
	{
		ui->installInfoLabel->setText("The patch has no dependencies");
		ui->installButton->setEnabled(true);
	}
	else
	{
		ui->checkButton->setEnabled(true);
	}

	ui->patchPathEdit->clear();
	ui->patchPathEdit->setPlaceholderText("Opened patch: " + patch_dir.absolutePath());
	ui->patchPathEdit->setDisabled(true);
	ui->openPatchButton->setText("Close");
	ui->openPatchButton->setIcon(QIcon(":/images/close.svg"));
	ui->openPatchButton->setIconSize(QSize(12, 12));
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
		ui->checkButton->setDisabled(true);

		QApplication::beep();

		if (!ui->dependencyListWidget->GetAreAllSatisfied())
		{
			QMessageBox::warning(this, "Verification completed"
				, "Verification completed. Not all dependencies are found. If you want to continue the installation, "
				"confirm all dependencies manually in the list"
				, QMessageBox::Ok, QMessageBox::Ok);
		}
		else
		{
			QMessageBox::information(this, "Verification completed"
				, "Verification completed. All dependencies found. The patch can be installed safely ... almost safely :)"
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

	if (!ui->dependencyListWidget->GetAreAllSatisfied())
	{
		QApplication::beep();
		const auto dialogResult = QMessageBox::warning(this, "Unsafe installation"
			,"WARNING: not all dependencies are found. Installation may cause database errors."
			" Are you sure you want to continue?"
			, QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Cancel);

		if (dialogResult == QMessageBox::Cancel)
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
	if (ui->dependencyListWidget->GetCheckedCount() == ui->dependencyListWidget->topLevelItemCount())
	{
		if (!ui->dependencyListWidget->GetAreAllSatisfied())
		{
			ui->installInfoLabel->setText("Some dependencies are not found!");
		}
		else
		{
			ui->installInfoLabel->setText("All dependencies are found");
		}
		
		ui->installButton->setEnabled(true);
	}
	else
	{
		ui->installInfoLabel->setText("Confirm all dependencies manually in the list");
		ui->installButton->setEnabled(false);
	}
}

// Handles start of disconnection from database
void InstallerWidget::OnDisconnectionStarted()
{
	if (!is_patch_opened)
	{
		return;
	}

	ui->dependencyListWidget->ClearCheck();
	ui->checkButton->setEnabled(true);
	ui->installButton->setDisabled(true);
	ui->installInfoLabel->setText("");
}

bool InstallerWidget::StartDependencyCheck()
{
	PatchList dependencyList;

	for (auto i = 0; i < ui->dependencyListWidget->topLevelItemCount(); ++i)
	{
		const auto currentItem = ui->dependencyListWidget->topLevelItem(i);
		dependencyList.Add(currentItem->data(PatchListWidget::ColumnIndexes::typeColumn, Qt::UserRole).toInt()
			, currentItem->text(PatchListWidget::ColumnIndexes::schemaColumn)
			, currentItem->text(PatchListWidget::ColumnIndexes::nameColumn), QStringList());
	}

	if (!FileHandler::MakeDependencyList(patch_dir.absolutePath(), dependencyList))
	{
		return false;
	}

	auto isSuccessful = false;
	const auto checkResult = InstallerHandler::CheckDependencies(DatabaseProvider::Database(), DatabaseProvider::User(), DatabaseProvider::Password()
		, DatabaseProvider::Host(), DatabaseProvider::Port(), patch_dir.absolutePath(), isSuccessful);
	return isSuccessful && ui->dependencyListWidget->SetCheckStatus(checkResult);
}
