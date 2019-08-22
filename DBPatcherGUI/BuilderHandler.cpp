#include "BuilderHandler.h"

#include <QProcess>
#include <QIODevice>

const QString BuilderHandler::program = "PatchBuilder_exe.exe";
QString BuilderHandler::templates_path = "Templates.ini";
QIODevice *BuilderHandler::output_device = nullptr;

// Sets new log output device
void BuilderHandler::SetOutputDevice(QIODevice &new_device)
{
	output_device = &new_device;
}

void BuilderHandler::SetTemplatesFile(const QString &path)
{
	templates_path = path;
}

// Launches and manages Builder process
// Returns result of patch build
bool BuilderHandler::BuildPatch(const QString& database, const QString& user, const QString& password
	, const QString& server, int port, const QString &patch_dir, const QString &build_list_dir)
{
	const auto connectionInfo = QString("%1:%2:%3:%4:%5").arg(server).arg(port).arg(database).arg(user).arg(password);
	const QStringList arguments = { "-d", patch_dir, "-p", build_list_dir, "-c", connectionInfo, "-t", templates_path };

	QProcess builderProcess;

	connect(&builderProcess, &QProcess::readyReadStandardOutput, [&builderProcess]()
	{
		if (output_device)
		{
			output_device->write(builderProcess.readAllStandardOutput());
		}
	});

	connect(&builderProcess, &QProcess::readyReadStandardError, [&builderProcess]()
	{
		if (output_device)
		{
			output_device->write(builderProcess.readAllStandardError());
		}
	});

	builderProcess.start(program, arguments);

	if (!builderProcess.waitForStarted())
	{
		return false;
	}

	if (!builderProcess.waitForFinished() || builderProcess.exitCode() != 0)
	{
		return false;
	}

	return true;
}