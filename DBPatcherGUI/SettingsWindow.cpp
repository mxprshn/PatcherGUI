#include "SettingsWindow.h"
#include "ui_SettingsWindow.h"

#include <QFileInfo>
#include <QMessageBox>

// Constructor
SettingsWindow::SettingsWindow(QWidget *parent)
	: QDialog(parent)
	, ui(new Ui::SettingsWindow)
{
	ui->setupUi(this);
	setWindowFlag(Qt::WindowContextHelpButtonHint, false);

	connect(ui->buttonBox, SIGNAL(accepted()), this, SIGNAL(saveButtonClicked()));
	connect(ui->buttonBox, SIGNAL(rejected()), this, SLOT(close()));
}

// Initializes settings dialog with current settings and opens it
void SettingsWindow::openSettingsDialog(const QSettings &settings)
{
	ui->templatesEdit->setText(settings.value("templates", "Templates.ini").toString());
	open();
	ui->templatesEdit->deselect();
}

// Saves settings entered by user if they are valid
void SettingsWindow::saveSettings(QSettings &settings)
{
	const QFileInfo templatesFile(ui->templatesEdit->text());

	if (templatesFile.suffix() == "ini" && templatesFile.exists())
	{
		settings.setValue("templates", templatesFile.absoluteFilePath());
		close();
	}
	else
	{
		QMessageBox::warning(this, "Error", "Templates file not found or not a (*.ini) configuration file"
			, QMessageBox::Ok, QMessageBox::Ok);
	}
}

// Destructor
SettingsWindow::~SettingsWindow()
{
	delete ui;
}