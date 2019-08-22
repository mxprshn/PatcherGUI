#include "InstallerHandler.h"

#include <QBitArray>
#include <QProcess>
#include <QIODevice>

const QString InstallerHandler::program = "PatchInstaller_exe.exe";
QIODevice *InstallerHandler::output_device = nullptr;

// Sets new log output device
void InstallerHandler::SetOutputDevice(QIODevice &new_device)
{
	output_device = &new_device;
}

// Launches and manages patch installation process
// Returns result of installation
bool InstallerHandler::InstallPatch(const QString &database, const QString &user, const QString &password,
	const QString &server, int port, const QString &path)
{
	const auto connectionInfo = QString("%1:%2:%3:%4:%5").arg(server)	.arg(port).arg(database).arg(user).arg(password);
	const QStringList arguments = { connectionInfo, "install", path };

	QProcess installerProcess;

	connect(&installerProcess, &QProcess::readyReadStandardError, [&installerProcess] ()
	{
		if (output_device)
		{
			output_device->write(installerProcess.readAllStandardError());
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
QBitArray InstallerHandler::CheckDependencies(const QString &database, const QString &user, const QString &password,
	const QString &server, int port, const QString &path, bool &is_successful)
{
	const auto connectionInfo = QString("%1:%2:%3:%4:%5").arg(server)	.arg(port).arg(database).arg(user).arg(password);
	const QStringList arguments = { connectionInfo, "check", path };

	QProcess installerProcess;

	connect(&installerProcess, &QProcess::readyReadStandardError, [&installerProcess] ()
	{
		if (output_device)
		{
			output_device->write(installerProcess.readAllStandardError());
		}
	});

	QBitArray checkResult;

	installerProcess.start(program, arguments);

	if (!installerProcess.waitForStarted())
	{
		is_successful = false;
		return checkResult;
	}

	if (!installerProcess.waitForFinished() || installerProcess.exitCode() != 0)
	{
		is_successful = false;
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
				is_successful = false;
				checkResult.clear();
				return checkResult;
			}
		}
	}

	is_successful = true;
	return checkResult;
}