#include "SettingsWindow.h"
#include "ui_SettingsWindow.h"

#include <QFileDialog>

// Constructor
SettingsWindow::SettingsWindow(QWidget *parent)
	: QDialog(parent)
	, ui(new Ui::SettingsWindow)
{
	ui->setupUi(this);
	setWindowFlag(Qt::WindowContextHelpButtonHint, false);

	connect(ui->button_box, &QDialogButtonBox::accepted, this, &SettingsWindow::SaveButtonClicked);
	connect(ui->button_box, &QDialogButtonBox::accepted, this, &SettingsWindow::close);
	connect(ui->button_box, &QDialogButtonBox::rejected, this, &SettingsWindow::close);
	connect(ui->explorer_button, &QToolButton::clicked, this, &SettingsWindow::OnExplorerButtonClicked);
	connect(ui->default_button, &QToolButton::clicked, this, &SettingsWindow::OnDefaultButtonClicked);
}

// Initializes settings dialog with current settings and opens it
void SettingsWindow::OpenSettingsDialog(const QSettings &settings)
{
	ui->templates_edit->setText(settings.value("templates", "Templates.ini").toString());
	open();
	ui->templates_edit->deselect();
}

// Saves settings entered by user
void SettingsWindow::SaveSettings(QSettings &settings)
{
	settings.setValue("templates", ui->templates_edit->text());
}

// Handles explorer button click
// Opens explorer to open a templates file
void SettingsWindow::OnExplorerButtonClicked()
{
	const auto path_input = QFileDialog::getOpenFileName(this, "Open templates file", "", "Configuration File (*.ini)");
	if (!path_input.isEmpty())
	{
		ui->templates_edit->setText(path_input);
	}
}

// Handles default button click
// Sets default templates file path
void SettingsWindow::OnDefaultButtonClicked()
{
	ui->templates_edit->setText("Templates.ini");
}

// Destructor
SettingsWindow::~SettingsWindow()
{
	delete ui;
}