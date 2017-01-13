/***********************************************************************************
**
** OrionLauncherWindow.cpp
**
** Copyright (C) December 2016 Hotride
**
************************************************************************************
*/
//----------------------------------------------------------------------------------
#include "orionlauncherwindow.h"
#include "ui_orionlauncherwindow.h"
#include <QMessageBox>
#include "ServerListItem.h"
#include "ProxyListItem.h"
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QDir>
#include <QFileDialog>
#include <QProcess>
#include <windows.h>

OrionLauncherWindow *g_OrionLauncherWindow = nullptr;
//----------------------------------------------------------------------------------
OrionLauncherWindow::OrionLauncherWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::OrionLauncherWindow)
{
	ui->setupUi(this);

	g_OrionLauncherWindow = this;

	setFixedSize(size());

	LoadProxyList();
	LoadServerList();

	ui->tw_Main->setCurrentIndex(0);
	ui->tw_Server->setCurrentIndex(0);
}
//----------------------------------------------------------------------------------
OrionLauncherWindow::~OrionLauncherWindow()
{
	g_OrionLauncherWindow = nullptr;

	delete ui;
}
//----------------------------------------------------------------------------------
void OrionLauncherWindow::closeEvent(QCloseEvent *event)
{
	SaveServerList();
	SaveProxyList();

	event->accept();
}
//----------------------------------------------------------------------------------
void OrionLauncherWindow::keyPressEvent(QKeyEvent *event)
{
	if (event->isAutoRepeat())
		return;

	if (event->key() == Qt::Key_Delete)
	{
		QWidget *focused = QApplication::focusWidget();

		if (focused == ui->lw_ServerList) //Servers list
			on_pb_ServerRemove_clicked();
		else if (focused == ui->lw_ProxyList) //Proxy list
			on_pb_ProxyRemove_clicked();
	}

	event->accept();
}
//----------------------------------------------------------------------------------
void OrionLauncherWindow::on_lw_ServerList_clicked(const QModelIndex &index)
{
	CServerListItem *item = (CServerListItem*)ui->lw_ServerList->item(index.row());

	if (item != nullptr)
	{
		ui->le_ServerName->setText(item->text());
		ui->le_ServerAddress->setText(item->GetAddress());
		ui->le_ServerAccount->setText(item->GetAccount());
		ui->le_ServerPassword->setText(item->GetPassword());
		ui->cb_ServerEncryptPassword->setChecked(item->GetEncrypted());
		ui->le_CommandLine->setText(item->GetCommand());

		ui->cb_ServerUseProxy->setChecked(item->GetUseProxy());
		ui->cb_ServerProxy->setCurrentText(item->GetProxy());
	}
}
//----------------------------------------------------------------------------------
void OrionLauncherWindow::on_lw_ServerList_doubleClicked(const QModelIndex &index)
{
	Q_UNUSED(index);

	on_pb_Launch_clicked();
}
//----------------------------------------------------------------------------------
void OrionLauncherWindow::on_cb_ServerShowPassword_clicked()
{
	if (ui->cb_ServerShowPassword->isChecked())
		ui->le_ServerPassword->setEchoMode(QLineEdit::EchoMode::Normal);
	else
		ui->le_ServerPassword->setEchoMode(QLineEdit::EchoMode::Password);
}
//----------------------------------------------------------------------------------
void OrionLauncherWindow::on_pb_ServerAdd_clicked()
{
	QString name = ui->le_ServerName->text().toLower();

	if (!name.length())
	{
		QMessageBox::critical(this, "Name is empty", "Enter the server name!");
		return;
	}

	for (int i = 0; i < ui->lw_ServerList->count(); i++)
	{
		QListWidgetItem *item = ui->lw_ServerList->item(i);

		if (item != nullptr && item->text().toLower() == name)
		{
			QMessageBox::critical(this, "Name is already exists", "Server name is already exists!");
			return;
		}
	}

	CServerListItem *item = new CServerListItem(ui->le_ServerName->text(), ui->le_ServerAddress->text(), ui->le_ServerAccount->text(), ui->le_ServerPassword->text(), ui->cb_ServerEncryptPassword->isChecked());
	item->SetUseProxy(ui->cb_ServerUseProxy->isChecked());
	item->SetProxy(ui->cb_ServerProxy->currentText());

	ui->lw_ServerList->addItem(item);

	ui->lw_ServerList->setCurrentRow(ui->lw_ServerList->count() - 1);

	SaveServerList();
}
//----------------------------------------------------------------------------------
void OrionLauncherWindow::on_pb_ServerSave_clicked()
{
	QString name = ui->le_ServerName->text().toLower();

	if (!name.length())
	{
		QMessageBox::critical(this, "Name is empty", "Enter the server name!");
		return;
	}

	CServerListItem *selected = (CServerListItem*)ui->lw_ServerList->currentItem();

	for (int i = 0; i < ui->lw_ServerList->count(); i++)
	{
		QListWidgetItem *item = ui->lw_ServerList->item(i);

		if (item != nullptr && item->text().toLower() == name)
		{
			if (item != nullptr && item != selected)
			{
				QMessageBox::critical(this, "Name is already exists", "Server name is already exists (not this item)!");
				return;
			}

			break;
		}
	}

	selected->setText(ui->le_ServerName->text());
	selected->SetAddress(ui->le_ServerAddress->text());
	selected->SetAccount(ui->le_ServerAccount->text());
	selected->SetPassword(ui->le_ServerPassword->text());
	selected->SetEncrypted(ui->cb_ServerEncryptPassword->isChecked());
	selected->SetUseProxy(ui->cb_ServerUseProxy->isChecked());
	selected->SetProxy(ui->cb_ServerProxy->currentText());

	SaveServerList();
}
//----------------------------------------------------------------------------------
void OrionLauncherWindow::on_pb_ServerRemove_clicked()
{
	QListWidgetItem *item = ui->lw_ServerList->currentItem();

	if (item != nullptr)
	{
		item = ui->lw_ServerList->takeItem(ui->lw_ServerList->row(item));

		if (item != nullptr)
		{
			delete item;

			SaveServerList();
		}
	}
}
//----------------------------------------------------------------------------------
void OrionLauncherWindow::on_le_CommandLine_textChanged(const QString &arg1)
{
	CServerListItem *selected = (CServerListItem*)ui->lw_ServerList->currentItem();

	if (selected != nullptr)
		selected->SetCommand(arg1);
}
//----------------------------------------------------------------------------------
void OrionLauncherWindow::on_lw_ProxyList_clicked(const QModelIndex &index)
{
	CProxyListItem *item = (CProxyListItem*)ui->lw_ProxyList->item(index.row());

	if (item != nullptr)
	{
		ui->le_ProxyName->setText(item->text());
		ui->le_ProxyAddress->setText(item->GetAddress());
		ui->le_ProxyPort->setText(item->GetProxyPort());
		ui->gb_ProxySocks5->setChecked(item->GetSocks5());
		ui->le_ProxyAccount->setText(item->GetAccount());
		ui->le_ProxyPassword->setText(item->GetPassword());
		ui->cb_ProxyEncryptPassword->setChecked(item->GetEncrypted());
	}
}
//----------------------------------------------------------------------------------
void OrionLauncherWindow::on_cb_ProxyShowPassword_clicked()
{
	if (ui->cb_ProxyShowPassword->isChecked())
		ui->le_ProxyPassword->setEchoMode(QLineEdit::EchoMode::Normal);
	else
		ui->le_ProxyPassword->setEchoMode(QLineEdit::EchoMode::Password);
}
//----------------------------------------------------------------------------------
void OrionLauncherWindow::on_pb_ProxyAdd_clicked()
{
	QString name = ui->le_ProxyName->text().toLower();

	if (!name.length())
	{
		QMessageBox::critical(this, "Name is empty", "Enter the proxy server name!");
		return;
	}

	for (int i = 0; i < ui->lw_ProxyList->count(); i++)
	{
		QListWidgetItem *item = ui->lw_ProxyList->item(i);

		if (item != nullptr && item->text().toLower() == name)
		{
			QMessageBox::critical(this, "Name is already exists", "Proxy server name is already exists!");
			return;
		}
	}

	ui->lw_ProxyList->addItem(new CProxyListItem(ui->le_ProxyName->text(), ui->le_ProxyAddress->text(), ui->le_ProxyPort->text(), ui->gb_ProxySocks5->isChecked(), ui->le_ProxyAccount->text(), ui->le_ProxyPassword->text(), ui->cb_ProxyEncryptPassword->isChecked()));

	ui->lw_ProxyList->setCurrentRow(ui->lw_ProxyList->count() - 1);

	SaveProxyList();
}
//----------------------------------------------------------------------------------
void OrionLauncherWindow::on_pb_ProxySave_clicked()
{
	QString name = ui->le_ProxyName->text().toLower();

	if (!name.length())
	{
		QMessageBox::critical(this, "Name is empty", "Enter the proxy server name!");
		return;
	}

	CProxyListItem *selected = (CProxyListItem*)ui->lw_ProxyList->currentItem();

	for (int i = 0; i < ui->lw_ProxyList->count(); i++)
	{
		QListWidgetItem *item = ui->lw_ProxyList->item(i);

		if (item != nullptr && item->text().toLower() == name)
		{
			if (item != nullptr && item != selected)
			{
				QMessageBox::critical(this, "Name is already exists", "Proxy server name is already exists (not this item)!");
				return;
			}

			break;
		}
	}

	selected->setText(ui->le_ProxyName->text());
	selected->SetAddress(ui->le_ProxyAddress->text());
	selected->SetProxyPort(ui->le_ProxyPort->text());
	selected->SetSocks5(ui->gb_ProxySocks5->isChecked());
	selected->SetAccount(ui->le_ProxyAccount->text());
	selected->SetPassword(ui->le_ProxyPassword->text());
	selected->SetEncrypted(ui->cb_ProxyEncryptPassword->isChecked());

	SaveProxyList();
}
//----------------------------------------------------------------------------------
void OrionLauncherWindow::on_pb_ProxyRemove_clicked()
{
	QListWidgetItem *item = ui->lw_ProxyList->currentItem();

	if (item != nullptr)
	{
		item = ui->lw_ProxyList->takeItem(ui->lw_ProxyList->row(item));

		if (item != nullptr)
		{
			delete item;

			SaveProxyList();
		}
	}
}
//----------------------------------------------------------------------------------
QString OrionLauncherWindow::BoolToText(const bool &value)
{
	if (value)
		return "true";

	return "false";
}
//----------------------------------------------------------------------------------
bool OrionLauncherWindow::RawStringToBool(QString value)
{
	value = value.toLower();
	bool result = false;

	if (value == "true" || value == "on")
		result = true;
	else
		result = (value.toInt() != 0);

	return result;
}
//----------------------------------------------------------------------------------
void OrionLauncherWindow::SaveProxyList()
{
	QFile file(QDir::currentPath() + "/Proxy.xml");

	if (file.open(QIODevice::WriteOnly | QIODevice::Text))
	{
		QXmlStreamWriter writter(&file);

		writter.setAutoFormatting(true);

		writter.writeStartDocument();

		int count = ui->lw_ProxyList->count();

		writter.writeStartElement("proxylist");
		writter.writeAttribute("version", "0");
		writter.writeAttribute("size", QString::number(count));

		for (int i = 0; i < count; i++)
		{
			CProxyListItem *item = (CProxyListItem*)ui->lw_ProxyList->item(i);

			if (item != nullptr)
			{
				writter.writeStartElement("proxy");

				writter.writeAttribute("name", item->text());
				writter.writeAttribute("address", item->GetAddress());
				writter.writeAttribute("port", item->GetProxyPort());
				writter.writeAttribute("socks5", BoolToText(item->GetSocks5()));
				writter.writeAttribute("account", item->GetAccount());
				writter.writeAttribute("password", item->GetPassword());
				writter.writeAttribute("encrypted", BoolToText(item->GetEncrypted()));

				writter.writeEndElement(); //proxy
			}
		}

		writter.writeEndElement(); //proxylist

		writter.writeEndDocument();

		file.close();
	}
}
//----------------------------------------------------------------------------------
void OrionLauncherWindow::SaveServerList()
{
	QFile file(QDir::currentPath() + "/Server.xml");

	if (file.open(QIODevice::WriteOnly | QIODevice::Text))
	{
		QXmlStreamWriter writter(&file);

		writter.setAutoFormatting(true);

		writter.writeStartDocument();

		int count = ui->lw_ServerList->count();

		writter.writeStartElement("serverlist");
		writter.writeAttribute("version", "0");
		writter.writeAttribute("size", QString::number(count));
		writter.writeAttribute("path", ui->le_OrionPath->text());

		for (int i = 0; i < count; i++)
		{
			CServerListItem *item = (CServerListItem*)ui->lw_ServerList->item(i);

			if (item != nullptr)
			{
				writter.writeStartElement("server");

				writter.writeAttribute("name", item->text());

				writter.writeAttribute("address", item->GetAddress());
				writter.writeAttribute("account", item->GetAccount());
				writter.writeAttribute("password", item->GetPassword());
				writter.writeAttribute("command", item->GetCommand());
				writter.writeAttribute("encrypted", BoolToText(item->GetEncrypted()));
				writter.writeAttribute("useproxy", BoolToText(item->GetUseProxy()));
				writter.writeAttribute("proxyname", item->GetProxy());

				writter.writeEndElement(); //server
			}
		}

		writter.writeEndElement(); //serverlist

		writter.writeEndDocument();

		file.close();
	}
}
//----------------------------------------------------------------------------------
void OrionLauncherWindow::LoadProxyList()
{
	ui->lw_ProxyList->clear();

	QFile file(QDir::currentPath() + "/Proxy.xml");

	if (file.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		QXmlStreamReader reader(&file);

		int version = 0;
		int count = 0;

		Q_UNUSED(version);
		Q_UNUSED(count);

		while (!reader.atEnd() && !reader.hasError())
		{
			if (reader.isStartElement())
			{
				QXmlStreamAttributes attributes = reader.attributes();

				if (reader.name() == "proxylist")
				{
					if (attributes.hasAttribute("version"))
						version = attributes.value("version").toInt();

					if (attributes.hasAttribute("size"))
						count = attributes.value("size").toInt();
				}
				else if (reader.name() == "proxy")
				{
					if (attributes.hasAttribute("name"))
					{
						CProxyListItem *item = new CProxyListItem(attributes.value("name").toString());

						if (attributes.hasAttribute("address"))
							item->SetAddress(attributes.value("address").toString());

						if (attributes.hasAttribute("port"))
							item->SetProxyPort(attributes.value("port").toString());

						if (attributes.hasAttribute("socks5"))
							item->SetSocks5(RawStringToBool(attributes.value("socks5").toString()));

						if (attributes.hasAttribute("account"))
							item->SetAccount(attributes.value("account").toString());

						if (attributes.hasAttribute("password"))
							item->SetPassword(attributes.value("password").toString());

						if (attributes.hasAttribute("encrypted"))
							item->SetEncrypted(RawStringToBool(attributes.value("encrypted").toString()));

						ui->lw_ProxyList->addItem(item);
					}
				}
			}

			reader.readNext();
		}

		file.close();
	}
}
//----------------------------------------------------------------------------------
void OrionLauncherWindow::LoadServerList()
{
	ui->lw_ServerList->clear();

	QFile file(QDir::currentPath() + "/Server.xml");

	if (file.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		QXmlStreamReader reader(&file);

		int version = 0;
		int count = 0;

		Q_UNUSED(version);
		Q_UNUSED(count);

		while (!reader.atEnd() && !reader.hasError())
		{
			if (reader.isStartElement())
			{
				QXmlStreamAttributes attributes = reader.attributes();

				if (reader.name() == "serverlist")
				{
					if (attributes.hasAttribute("version"))
						version = attributes.value("version").toInt();

					if (attributes.hasAttribute("size"))
						count = attributes.value("size").toInt();

					if (attributes.hasAttribute("path"))
						ui->le_OrionPath->setText(attributes.value("path").toString());
				}
				else if (reader.name() == "server")
				{
					if (attributes.hasAttribute("name"))
					{
						CServerListItem *item = new CServerListItem(attributes.value("name").toString());

						if (attributes.hasAttribute("address"))
							item->SetAddress(attributes.value("address").toString());

						if (attributes.hasAttribute("account"))
							item->SetAccount(attributes.value("account").toString());

						if (attributes.hasAttribute("password"))
							item->SetPassword(attributes.value("password").toString());

						if (attributes.hasAttribute("command"))
							item->SetCommand(attributes.value("command").toString());

						if (attributes.hasAttribute("encrypted"))
							item->SetEncrypted(RawStringToBool(attributes.value("encrypted").toString()));

						if (attributes.hasAttribute("useproxy"))
							item->SetUseProxy(RawStringToBool(attributes.value("useproxy").toString()));

						if (attributes.hasAttribute("proxyname"))
							item->SetProxy(attributes.value("proxyname").toString());

						ui->lw_ServerList->addItem(item);
					}
				}
			}

			reader.readNext();
		}

		file.close();
	}
}
//----------------------------------------------------------------------------------
void OrionLauncherWindow::on_tb_SetOrionPath_clicked()
{
	QString startPath = ui->le_OrionPath->text();

	if (!startPath.length())
		startPath = QDir::currentPath();

	QString path = QFileDialog::getOpenFileName(nullptr, tr("Select OrionUO"), startPath, tr("Executable(*.exe)"));

	if (path.length())
		ui->le_OrionPath->setText(path);
}
//----------------------------------------------------------------------------------
QString OrionLauncherWindow::DecodeArgumentString(const char *text, const int &length)
{
	QString result = "";

	for (int i = 0; i < length; i += 2)
	{
		char buf[3] = { text[i], text[i + 1], 0 };

		result += (char)QString(buf).toInt(nullptr, 16);
	}

	return result;
}
//----------------------------------------------------------------------------------
QString OrionLauncherWindow::EncodeArgumentString(const char *text, const int &length)
{
	QString result = "";

	for (int i = 0; i < length; i++)
	{
		QString buf = "";
		buf.sprintf("%02X", text[i]);

		result += buf;
	}

	return result;
}
//----------------------------------------------------------------------------------
void OrionLauncherWindow::on_pb_Launch_clicked()
{
	CServerListItem *serverItem = (CServerListItem*)ui->lw_ServerList->currentItem();

	if (serverItem == nullptr)
	{
		QMessageBox::critical(this, tr("Launch error"), tr("Server is not selected!"));
		return;
	}

	QString directoryPath = ui->le_OrionPath->text();
	int pos = directoryPath.lastIndexOf('/');

	if (pos == -1)
		pos = directoryPath.lastIndexOf('\\');

	if (pos != -1)
		directoryPath.resize(pos);

	QString program = ui->le_OrionPath->text();

	QString command = ui->le_CommandLine->text();

	if (ui->cb_LaunchFastLogin->isChecked())
		command += " -fastlogin";

	if (ui->cb_LaunchAutologin->isChecked())
		command += " -autologin";
	else
		command += " -autologin:0";

	if (ui->cb_LaunchSavePassword->isChecked())
		command += " -savepassword";
	else
		command += " -savepassword:0";

	if (ui->cb_LaunchSaveAero->isChecked())
		command += " -aero";

	command += " \"-login:" + serverItem->GetAddress() + "\"";

	QString account = serverItem->GetAccount();
	QString password = serverItem->GetPassword();
	command += " -account:" + EncodeArgumentString(account.toStdString().c_str(), account.length()) + "," + EncodeArgumentString(password.toStdString().c_str(), password.length());

	if (serverItem->GetUseProxy())
	{
		QString proxyName = serverItem->GetProxy().toLower();

		for (int i = 0; i < ui->lw_ProxyList->count(); i++)
		{
			CProxyListItem *proxy = (CProxyListItem*)ui->lw_ProxyList->item(i);

			if (proxy != nullptr && proxy->text().toLower() == proxyName)
			{
				command += " -proxyhost:" + proxy->GetAddress() + "," + proxy->GetProxyPort();

				if (proxy->GetSocks5())
				{
					QString proxyAccount = proxy->GetAccount();
					QString proxyPassword = proxy->GetPassword();
					command += " -proxyaccount:" + EncodeArgumentString(proxyAccount.toStdString().c_str(), proxyAccount.length()) + "," + EncodeArgumentString(proxyPassword.toStdString().c_str(), proxyPassword.length());

				}

				break;
			}
		}
	}

	if (command.length())
		program += " " + command;

	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	memset(&si, 0, sizeof(si));
	si.cb = sizeof(si);

	memset(&pi, 0, sizeof(pi));

	if (CreateProcess(NULL, (LPWSTR)program.toStdWString().c_str(), NULL, NULL, FALSE, 0, NULL, (LPWSTR)directoryPath.toStdWString().c_str(), &si, &pi))
	{
		CloseHandle(pi.hThread);
		CloseHandle(pi.hProcess);
	}

	if (ui->cb_LaunchCloseAfterLaunch->isChecked())
		exit(0);
}
//----------------------------------------------------------------------------------
