#include "kernel.h"
#include "updatecenter.h"
#include "recovery.h"

#include <QFile>
#include <QDir>

#include <QtAndroidExtras/QtAndroid>
#include <QtTest/QTest>


Kernel::Kernel(QWidget *parent) : QWidget(parent)
{
    settingsFirmware = new QSettings("/system/kernel.ini", QSettings::IniFormat);
    Name = settingsFirmware->value("General/Name").toString();
    version = settingsFirmware->value("General/Version").toString();
    url = settingsFirmware->value("General/URL").toString();
    path = settingsFirmware->value("General/Path").toString();
    boot = settingsFirmware->value("General/Boot").toString();
    backup = settingsFirmware->value("General/Backup").toString();

    QDir dir(path);
    if (!dir.exists())
    {
        dir.mkdir(path);
    }

    managerSearch = new QNetworkAccessManager();
    managerDownload = new QNetworkAccessManager();

    mainLayout = new QGridLayout;

    btnSearch = new QPushButton("Проверить");
    btnDownload = new QPushButton("Загрузить");
    btnMain = new QPushButton("Главная");
    btnRecovery = new QPushButton("Recovery");
    btnAbout = new QPushButton("О Программе!");

    boxAbout = new QMessageBox;

    textLog = new QTextEdit;

    btnDownload->setEnabled(false);

    mainLayout->addWidget(btnSearch, 0, 0, 1, 2);
    mainLayout->addWidget(btnDownload, 1, 0, 1, 2);
    mainLayout->addWidget(textLog, 2, 0, 1, 2);
    mainLayout->addWidget(btnMain, 3, 0, 1, 1);
    mainLayout->addWidget(btnRecovery, 3, 1, 1, 1);
    mainLayout->addWidget(btnAbout, 4, 0, 1, 2);

    setLayout(mainLayout);

    textLog->setReadOnly(true);

    textLog->append("=====Ядро=====");
    textLog->append("Название: " + Name);
    textLog->append("Текущая Версия: " + version);
    textLog->append("Папка загрузки: " + path);

    QObject::connect(btnSearch, SIGNAL(clicked()), this, SLOT(Search()));
    QObject::connect(btnDownload, SIGNAL(clicked()), this, SLOT(Download()));
    QObject::connect(btnMain, SIGNAL(clicked()), this, SLOT(showMain()));
    QObject::connect(btnRecovery, SIGNAL(clicked()), this, SLOT(showRecovery()));
    QObject::connect(btnAbout, SIGNAL(clicked()), this, SLOT(About()));
}

Kernel::~Kernel()
{
    delete settingsFirmware;

    delete managerSearch;
    delete managerDownload;

    delete mainLayout;

    delete btnSearch;
    delete btnDownload;
    delete btnMain;
    delete btnRecovery;
    delete btnAbout;

    delete textLog;

    delete boxAbout;
}

void Kernel::Search()
{
     QString site = url + "/update/kernel/" + version + "/version.ini";

     QObject::connect(managerSearch, SIGNAL(finished(QNetworkReply *)), this, SLOT(onSearchResult(QNetworkReply*))); //отправляем данные и получаем ответ успешно или ошибка
     managerSearch->get(QNetworkRequest(site));
}

void Kernel::onSearchResult(QNetworkReply *replyS)
{

    QFile nFile(path + "/version.ini");

    if(!replyS->error())
    {
        nFile.open(QFile::WriteOnly);
        nFile.write(replyS->readAll());
    }
    else
    {
        textLog->append(replyS->errorString());
        replyS->abort();
        replyS->deleteLater();
    }

    nFile.close();

    replyS->abort();
    replyS->deleteLater();

    QString newVersion;
    QString fullSize;

    QSettings newSet(path + "/version.ini", QSettings::IniFormat);
    fullSize = newSet.value("General/Size").toString();

    newVersion = newSet.value("General/newVersion").toString();

    if (version.toInt() < newVersion.toInt())
    {
        textLog->clear();
        textLog->append("Доступна новая версия ядра!!! " + newVersion);
        textLog->append("Размер: " + fullSize);

        btnDownload->setEnabled(true);
    }
    else if (version.toInt() == newVersion.toInt())
    {
        textLog->clear();
        textLog->append("Обновлений нет!");
    }
}

void Kernel::Download()
{
    QString newVersion;

    QNetworkReply *reply;

    QSettings newSet(path + "/version.ini", QSettings::IniFormat);

    newVersion = newSet.value("General/newVersion").toString();

    QString site = url + "/update/kernel/" + newVersion + "/boot.img";

    textLog->append("Начало Загрузки...");

    reply = managerDownload->get(QNetworkRequest(site));

    QObject::connect(reply, SIGNAL(downloadProgress(qint64, qint64)), this, SLOT(onProgress(qint64, qint64)));
    QObject::connect(managerDownload, SIGNAL(finished(QNetworkReply *)), this, SLOT(onDownloadResult(QNetworkReply*))); //отправляем данные и получаем ответ успешно или ошибка
}

void Kernel::onDownloadResult(QNetworkReply *replyD)
{
    QFile uFile(path + "/boot.img");

    if(!replyD->error())
    {
        uFile.open(QFile::WriteOnly);
        uFile.write(replyD->readAll());
    }
    else
    {
        textLog->append(replyD->errorString());
        replyD->abort();
        replyD->deleteLater();
    }

    uFile.close();

    replyD->abort();
    replyD->deleteLater();

    textLog->append("Загрузка Завершена!");
    textLog->append("Файл: " + path + "/boot.img");
}

void Kernel::About()
{
    boxAbout->setTextFormat(Qt::RichText);
    boxAbout->setText("Update Manager"

    "\n Разработано с помощью QT Framework."
    ""
    "\n Links:"
    "\n https://qt.io"
    "\nРазработчик: Danilka Terentyev(4pda: AC97; GitHub: ac973k)"
    "");

    boxAbout->exec();
}

void Kernel::onProgress(qint64 receivedSize, qint64 totalSize)
{
    textLog->clear();
    textLog->append("Загружено: " + QString::number(receivedSize) + " / " + QString::number(totalSize));
}

void Kernel::showMain()
{
    UpdateCenter *upd = new UpdateCenter;
    upd->show();
}

void Kernel::showRecovery()
{
    Recovery *rec = new Recovery;
    rec->show();
}

void Kernel::onInstall()
{
    textLog->append("Делаем резервную копию ядра");

    procBackup->setProcessChannelMode(QProcess::SeparateChannels);
    procBackup->start("su", QStringList() << "-c" << "busybox" << "dd" << "if=" << boot << "of=" + path + "/boot_" + version + ".img");

    if(!procBackup->waitForFinished())
    {
        textLog->append(procBackup->errorString());
    }
    else
    {
        textLog->append(procBackup->readAll());
        textLog->append("Готово! Бэкап лежит в " + path);
    }

    textLog->append("Прошиваем...");

    procInstall->setProcessChannelMode(QProcess::SeparateChannels);
    procInstall->start("su", QStringList() << "-c" << "busybox" << "dd" << "if=" << path + "/boot.img" << "of=" + boot);

    if(!procBackup->waitForFinished())
    {
        textLog->append(procBackup->errorString());
    }
    else
    {
        textLog->append(procBackup->readAll());
    }

    QSettings newSet(path + "/version.ini", QSettings::IniFormat);
    newVersion = newSet.value("General/newVersion").toString();
    settingsFirmware->setValue("General/Version", newVersion);
    settingsFirmware->sync();

    textLog->append("Текущая версия ядра: " + version);
    textLog->append("Готово!");
}
