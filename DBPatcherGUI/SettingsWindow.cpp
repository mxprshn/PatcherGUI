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

	connect(ui->button_box, &QDialogButtonBox::accepted, this, &SettingsWindow::SaveButtonClicked);
	connect(ui->button_box, &QDialogButtonBox::rejected, this, &SettingsWindow::close);
}

// Initializes settings dialog with current settings and opens it
void SettingsWindow::OpenSettingsDialog(const QSettings &settings)
{
	ui->templates_edit->setText(settings.value("templates", "Templates.ini").toString());
	open();
	ui->templates_edit->deselect();
}

// Saves settings entered by user if they are valid
void SettingsWindow::SaveSettings(QSettings &settings)
{
	const QFileInfo templates_file(ui->templates_edit->text());

	if (templates_file.suffix() == "ini" && templates_file.exists())
	{
		settings.setValue("templates", templates_file.absoluteFilePath());
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