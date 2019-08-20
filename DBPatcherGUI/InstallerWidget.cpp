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
	, isPatchOpened(false)
{
	ui->setupUi(this);

	ui->installInfoLabel->setText("");
	ui->checkButton->setDisabled(true);
	ui->installButton->setDisabled(true);
	setReadyToOpen();

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
bool InstallerWidget::checkConnection()
{
	if (!DatabaseProvider::isConnected())
	{
		QApplication::beep();
		QMessageBox::warning(this, "Database error"
			, "Not connected to database."
			, QMessageBox::Ok, QMessageBox::Ok);
		emit connectionRequested();
		return false;
	}

	return true;
}

// Sets elements of interface which is ready to open patch
void InstallerWidget::setReadyToOpen()
{
	ui->patchPathEdit->setPlaceholderText("Patch folder path (leave empty to open in explorer)");
	ui->openPatchButton->setText("Open");
	ui->openPatchButton->setIcon(QIcon(":/images/box.svg"));
	ui->openPatchButton->setIconSize(QSize(20, 20));
}

// Fills list widget of patch objects with information from patch 
bool InstallerWidget::initPatchList(const QString &path)
{
	auto isSuccessful = false;
	const auto objectList = FileHandler::parseObjectList(path, isSuccessful);

	if (!isSuccessful)
	{
		return false;		
	}

	for (const auto current : objectList)
	{
		const auto type = current->getType();
		ui->patchListWidget->add(type, current->getSchema(), current->getName()
			+ QString(type == ObjectTypes::function ? "(" + current->getParameters().join(",") + ")" : ""), false);
	}

	ui->patchListWidget->scrollToTop();
	return true;
}

// Fills list widget of dependencies with information from patch 
bool InstallerWidget::initDependencyList(const QString &path)
{
	auto isSuccessful = false;
	const auto dependencyList = FileHandler::parseDependencyList(path, isSuccessful);

	if(!isSuccessful)
	{
		return false;
	}

	for (const auto current : dependencyList)
	{
		ui->dependencyListWidget->add(current->getType(), current->getSchema(), current->getName());
	}

	return true;
}

// Sets all interface elements affected by patch opening to default state
void InstallerWidget::clearCurrentPatch()
{
	patchDir = QDir();
	ui->dependencyListWidget->clear();
	ui->patchListWidget->clear();
	ui->patchPathEdit->setPlaceholderText("Patch folder path");
	ui->patchPathEdit->setEnabled(true);
	ui->checkButton->setDisabled(true);
	ui->installButton->setDisabled(true);
	ui->installInfoLabel->setText("");
	setReadyToOpen();
	isPatchOpened = false;
}

// Handles open button click
// Opens patch list files if possible and sets interface elements to appropriate state
void InstallerWidget::onOpenButtonClicked()
{
	if (isPatchOpened)
	{
		const auto dialogResult = QMessageBox::question(this, "Close", "Are you sure to close current patch?"
			, QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Cancel);

		if (dialogResult == QMessageBox::Ok)
		{
			clearCurrentPatch();
		}
		
		return;
	}

	if (ui->patchPathEdit->text().isEmpty())
	{
		patchDir.setPath(QFileDialog::getExistingDirectory(this, "Choose patch directory"));

		if (patchDir.path().isEmpty())
		{
			clearCurrentPatch();
			return;
		}
	}
	else
	{
		patchDir.setPath(ui->patchPathEdit->text());

		if (!patchDir.exists())
		{
			QApplication::beep();
			QMessageBox::warning(this, "Open error", "Patch directory does not exist."
				, QMessageBox::Ok, QMessageBox::Ok);
			clearCurrentPatch();
			return;
		}
	}

	if (!patchDir.exists(FileHandler::getObjectListName()))
	{
		QApplication::beep();
		QMessageBox::warning(this, "Open error", FileHandler::getObjectListName() + " does not exist in patch directory."
			, QMessageBox::Ok, QMessageBox::Ok);
		clearCurrentPatch();
		return;
	}

	if (!patchDir.exists(FileHandler::getDependencyListName()))
	{
		QApplication::beep();
		QMessageBox::warning(this, "Open error", FileHandler::getDependencyListName() + " does not exist in patch directory."
			, QMessageBox::Ok, QMessageBox::Ok);
		clearCurrentPatch();
		return;
	}

	if (!initPatchList(patchDir.absolutePath()))
	{
		QApplication::beep();
		QMessageBox::warning(this, "Open error", "Incorrect file " + FileHandler::getObjectListName() + " ."
			, QMessageBox::Ok, QMessageBox::Ok);
		clearCurrentPatch();
		return;
	}

	if (!initDependencyList(patchDir.absolutePath()))
	{
		QApplication::beep();
		QMessageBox::warning(this, "Open error", "Incorrect file " + FileHandler::getDependencyListName() + " ."
			, QMessageBox::Ok, QMessageBox::Ok);
		clearCurrentPatch();
		return;
	}

	if (ui->dependencyListWidget->topLevelItemCount() == 0)
	{
		ui->installInfoLabel->setText("Patch has not any dependencies.");
		ui->installButton->setEnabled(true);
	}
	else
	{
		ui->checkButton->setEnabled(true);
	}

	ui->patchPathEdit->clear();
	ui->patchPathEdit->setPlaceholderText("Opened patch: " + patchDir.absolutePath());
	ui->patchPathEdit->setDisabled(true);
	ui->openPatchButton->setText("Close");
	ui->openPatchButton->setIcon(QIcon(":/images/close.svg"));
	ui->openPatchButton->setIconSize(QSize(12, 12));
	isPatchOpened = true;
}

// Handles check button click
// Launches dependency check and shows information about its result
void InstallerWidget::onCheckButtonClicked()
{
	if (!checkConnection())
	{
		return;
	}

	auto isSuccessful = false;
	const auto checkResult = InstallerHandler::checkDependencies(DatabaseProvider::database(), DatabaseProvider::user(), DatabaseProvider::password()
		, DatabaseProvider::host(), DatabaseProvider::port(), patchDir.absolutePath(), isSuccessful);

	if (isSuccessful && ui->dependencyListWidget->setCheckStatus(checkResult))
	{
		ui->checkButton->setEnabled(false);

		QApplication::beep();

		if (!ui->dependencyListWidget->getAreAllSatisfied())
		{
			QMessageBox::warning(this, "Check completed"
				, "Check completed. Not all dependencies are satisfied. If you want to install patch anyway, "
				"mark all not satisfied dependencies manually in the list."
				, QMessageBox::Ok, QMessageBox::Ok);
		}
		else
		{
			QMessageBox::information(this, "Check completed"
				, "Check completed. All dependencies are satisfied. Patch may be installed safely (or almost safely)."
				, QMessageBox::Ok, QMessageBox::Ok);
		}
	}
	else
	{
		QApplication::beep();
		QMessageBox::warning(this, "Check error"
			, "Error occured. See log for detailed information."
			, QMessageBox::Ok, QMessageBox::Ok);
	}
}

// Handles install button click
// Launches patch installation and shows information about its result
void InstallerWidget::onInstallButtonClicked()
{
	if (!checkConnection())
	{
		return;
	}

	if (!ui->dependencyListWidget->getAreAllSatisfied())
	{
		QApplication::beep();
		const auto dialogResult = QMessageBox::warning(this, "Unsafe installation"
			,"WARNING: Not all dependencies are satisfied. Installation may cause database errors. "
			"Are you sure to continue?"
			, QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Cancel);

		if (dialogResult == QMessageBox::Cancel)
		{
			return;
		}
	}

	if (InstallerHandler::installPatch(DatabaseProvider::database()
		, DatabaseProvider::user(), DatabaseProvider::password(), DatabaseProvider::host()
		, DatabaseProvider::port(), patchDir.absolutePath()))
	{
		QApplication::beep();
		QMessageBox::information(this, "Installation completed"
			, "Installation completed. See log for detailed information."
			, QMessageBox::Ok, QMessageBox::Ok);
	}
	else
	{
		QApplication::beep();
		QMessageBox::warning(this, "Installation error"
			, "Error occured. See log for detailed information."
			, QMessageBox::Ok, QMessageBox::Ok);
	}
}

// Handles amount of checked dependencies change
// Shows appropriate information and enables install option if all dependencies are checked
void InstallerWidget::onItemCheckChanged()
{
	if (ui->dependencyListWidget->getCheckedCount() == ui->dependencyListWidget->topLevelItemCount())
	{
		if (!ui->dependencyListWidget->getAreAllSatisfied())
		{
			ui->installInfoLabel->setText("WARNING: Some dependencies are not satisfied!");
		}
		else
		{
			ui->installInfoLabel->setText("All dependencies are satisfied.");
		}
		
		ui->installButton->setEnabled(true);
	}
	else
	{
		ui->installInfoLabel->setText("To enable installation, mark all dependencies manually.");
		ui->installButton->setEnabled(false);
	}
}