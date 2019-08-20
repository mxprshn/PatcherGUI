#include "BuilderHandler.h"

#include <QProcess>
#include <QIODevice>

const QString BuilderHandler::program = "PatchBuilder_exe.exe";
const QString BuilderHandler::templatesPath = "Templates.ini";
QIODevice *BuilderHandler::outputDevice = nullptr;

// Sets new log output device
void BuilderHandler::setOutputDevice(QIODevice &newDevice)
{
	outputDevice = &newDevice;
}

// Launches and manages Builder process
// Returns result of patch build
bool BuilderHandler::buildPatch(const QString& database, const QString& user, const QString& password
	, const QString& server, int port, const QString &patchDir, const QString &buildListDir)
{
	const auto connectionInfo = QString("%1:%2:%3:%4:%5").arg(database).arg(user).arg(password).arg(server)
		.arg(port);
	const QStringList arguments = { "-d", patchDir, "-p", buildListDir, "-c", connectionInfo, "-t", templatesPath };

	QProcess builderProcess;

	connect(&builderProcess, &QProcess::readyReadStandardOutput, [&builderProcess]()
	{
		if (outputDevice)
		{
			outputDevice->write(builderProcess.readAllStandardOutput());
		}
	});

	connect(&builderProcess, &QProcess::readyReadStandardError, [&builderProcess]()
	{
		if (outputDevice)
		{
			outputDevice->write(builderProcess.readAllStandardError());
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