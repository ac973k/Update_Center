#include "kernel.h"
#include "updatecenter.h"
#include "recovery.h"

#include <QFile>
#include <QDir>
#include <QMessageBox>

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
    QDir dirBkp(backup);
    if (!dir.exists())
    {
        dir.mkdir(path);
    }
    else if (!dirBkp.exists())
    {
        dirBkp.mkdir(backup);
    }

    managerSearch = new QNetworkAccessManager();
    managerDownload = new QNetworkAccessManager();

    mainLayout = new QGridLayout;

    btnSearch = new QPushButton("Проверить");
    btnDownload = new QPushButton("Загрузить");
    btnMain = new QPushButton("Главная");
    btnRecovery = new QPushButton("Recovery");
    btnAbout = new QPushButton("О Программе!");

    textLog = new QTextEdit;

    btnDownload->setEnabled(false);
    btnInstall->setEnabled(false);

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

    QMessageBox *boxInstall = new QMessageBox;
    boxInstall->setText("Установить?");
    btnInstall = boxInstall->addButton("Да", QMessageBox::ActionRole);
    boxInstall->exec();

    if (boxInstall->clickedButton() == btnInstall)
    {
        textLog->append("Делаем резервную копию recovery");

        QString command = "busybox dd if=" + boot + " of=" + backup + "/boot_" + version + ".img";

        procBackup = new QProcess;
        procBackup->setProcessChannelMode(QProcess::SeparateChannels);
        procBackup->start("su", QStringList() << "-c" << command);


        if(!procBackup->waitForFinished())
        {
            textLog->append(procBackup->errorString());
        }
        else
        {
            textLog->append(procBackup->readAll());
            textLog->append("Готово! Бэкап лежит в " + backup);
        }

        delete procBackup;

        textLog->append("Прошиваем...");

        QString commandI = "busybox dd if=" + path + "/boot.img of=" + boot;
        textLog->append(commandI);

        procInstall = new QProcess;
        procInstall->setProcessChannelMode(QProcess::SeparateChannels);
        procInstall->start("su", QStringList() << "-c" << commandI);

        if(!procInstall->waitForFinished())
        {
            textLog->append(procInstall->errorString());
        }
        else
        {
            textLog->append(procInstall->readAll());
        }

        delete procInstall;

        textLog->append("Текущая версия recovery: " + version);

        system("su -c cp /system/kernel.ini /sdcard/kernel.ini");
        QSettings updSet("/sdcard/kernel.ini", QSettings::IniFormat);
        QSettings newSet(path + "/version.ini", QSettings::IniFormat);
        newVersion = newSet.value("General/newVersion").toString();
        updSet.setValue("General/Version", newVersion);
        updSet.sync();

        system("su -c busybox mount -o remount,rw /system");
        system("su -c cp /sdcard/kernel.ini /system/kernel.ini");
        system("su -c busybox chmod 644 /system/kernel.ini");

        textLog->append("Новая версия kernel: " + newVersion);
        textLog->append("Готово!");
    }
    else if (boxInstall->clickedButton() == btnCancel)
    {
        textLog->append("Готово!");
    }

}

void Kernel::About()
{
    QMessageBox *boxAbout = new QMessageBox();

    boxAbout->setText("Text");

    boxAbout->show();
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
