#include "InstallerHandler.h"

#include <QBitArray>
#include <QProcess>
#include <QIODevice>

const QString InstallerHandler::program = "PatchInstaller_exe.exe";
QIODevice *InstallerHandler::outputDevice = nullptr;

// Sets new log output device
void InstallerHandler::setOutputDevice(QIODevice &newDevice)
{
	outputDevice = &newDevice;
}

// Launches and manages patch installation process
// Returns result of installation
bool InstallerHandler::installPatch(const QString &database, const QString &user, const QString &password,
	const QString &server, int port, const QString &path)
{
	const auto connectionInfo = QString("%1:%2:%3:%4:%5").arg(database).arg(user).arg(password).arg(server)
		.arg(port);
	const QStringList arguments = { connectionInfo, "install", path };

	QProcess installerProcess;

	connect(&installerProcess, &QProcess::readyReadStandardError, [&installerProcess] ()
	{
		if (outputDevice)
		{
			outputDevice->write(installerProcess.readAllStandardError());
		}
	});

	installerProcess.start(program, arguments);

	if (!installerProcess.waitForStarted())
	{
		return false;
	}

	if (!installerProcess.waitForFinished() || installerProcess.exitCode() != 0)
	{
		return false;
	}

	return true;
}

// Launches and manages dependency check process
// Returns result of check as bit array
QBitArray InstallerHandler::checkDependencies(const QString &database, const QString &user, const QString &password,
	const QString &server, int port, const QString &path, bool &isSuccessful)
{
	const auto connectionInfo = QString("%1:%2:%3:%4:%5").arg(database).arg(user).arg(password).arg(server)
		.arg(port);
	const QStringList arguments = { connectionInfo, "check", path };

	QProcess installerProcess;

	connect(&installerProcess, &QProcess::readyReadStandardError, [&installerProcess] ()
	{
		if (outputDevice)
		{
			outputDevice->write(installerProcess.readAllStandardError());
		}
	});

	QBitArray checkResult;

	installerProcess.start(program, arguments);

	if (!installerProcess.waitForStarted())
	{
		isSuccessful = false;
		return checkResult;
	}

	if (!installerProcess.waitForFinished() || installerProcess.exitCode() != 0)
	{
		isSuccessful = false;
		return checkResult;
	}

	installerProcess.setReadChannel(QProcess::ProcessChannel::StandardOutput);
	QByteArray readData = installerProcess.readAll();
	checkResult.resize(readData.count());

	for (auto i = 0; i < checkResult.count(); ++i)
	{
		switch (readData[i])
		{
			case '0':
			{
				checkResult[i] = false;
				break;
			}
			case '1':
			{
				checkResult[i] = true;
				break;
			}
			default:
			{
				isSuccessful = false;
				checkResult.clear();
				return checkResult;
			}
		}
	}

	isSuccessful = true;
	return checkResult;
}