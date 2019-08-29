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
	const auto connection_info = QString("%1:%2:%3:%4:%5").arg(server)	.arg(port).arg(database).arg(user).arg(password);
	const QStringList arguments = { connection_info, "install", path };

	QProcess installer_process;

	connect(&installer_process, &QProcess::readyReadStandardError, [&installer_process] ()
	{
		if (output_device)
		{
			output_device->write(installer_process.readAllStandardError());
		}
	});

	installer_process.start(program, arguments);

	if (!installer_process.waitForStarted())
	{
		return false;
	}

	if (!installer_process.waitForFinished() || installer_process.exitCode() != 0)
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
	const auto connection_info = QString("%1:%2:%3:%4:%5").arg(server).arg(port).arg(database).arg(user).arg(password);
	const QStringList arguments = { connection_info, "check", path };

	QProcess installer_process;

	connect(&installer_process, &QProcess::readyReadStandardError, [&installer_process] ()
	{
		if (output_device)
		{
			output_device->write(installer_process.readAllStandardError());
		}
	});

	QBitArray check_result;

	installer_process.start(program, arguments);

	if (!installer_process.waitForStarted())
	{
		is_successful = false;
		return check_result;
	}

	if (!installer_process.waitForFinished() || installer_process.exitCode() != 0)
	{
		is_successful = false;
		return check_result;
	}

	installer_process.setReadChannel(QProcess::ProcessChannel::StandardOutput);
	QByteArray read_data = installer_process.readAll();
	check_result.resize(read_data.count());

	for (auto i = 0; i < check_result.count(); ++i)
	{
		switch (read_data[i])
		{
			case '0':
			{
				check_result[i] = false;
				break;
			}
			case '1':
			{
				check_result[i] = true;
				break;
			}
			default:
			{
				is_successful = false;
				check_result.clear();
				return check_result;
			}
		}
	}

	is_successful = true;
	return check_result;
}