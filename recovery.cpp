#include "recovery.h"

#include "kernel.h"
#include "updatecenter.h"

#include <QFile>
#include <QDir>
#include <QMessageBox>

#include <QtAndroidExtras/QtAndroid>
#include <QtTest/QTest>

Recovery::Recovery(QWidget *parent) : QWidget(parent)
{
    settingsFirmware = new QSettings("/system/recovery.ini", QSettings::IniFormat);
    Name = settingsFirmware->value("General/Name").toString();
    version = settingsFirmware->value("General/Version").toString();
    url = settingsFirmware->value("General/URL").toString();
    path = settingsFirmware->value("General/Path").toString();
    recovery = settingsFirmware->value("General/Recovery").toString();
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
    btnInstall = new QPushButton("Установить");
    btnMain = new QPushButton("Главная");
    btnKernel = new QPushButton("Ядро");
    btnAbout = new QPushButton("О Программе!");

    textLog = new QTextEdit;

    btnDownload->setEnabled(false);
    //btnInstall->setEnabled(false);

    mainLayout->addWidget(btnSearch, 0, 0, 1, 2);
    mainLayout->addWidget(btnDownload, 1, 0, 1, 2);
    mainLayout->addWidget(btnInstall, 2, 0, 1, 2);
    mainLayout->addWidget(textLog, 3, 0, 1, 2);
    mainLayout->addWidget(btnMain, 4, 0, 1, 1);
    mainLayout->addWidget(btnKernel, 4, 1, 1, 1);
    mainLayout->addWidget(btnAbout, 5, 0, 1, 2);

    setLayout(mainLayout);

    textLog->setReadOnly(true);

    textLog->append("=====Recovery=====");
    textLog->append("Название: " + Name);
    textLog->append("Текущая Версия: " + version);
    textLog->append("Папка загрузки: " + path);

    QObject::connect(btnSearch, SIGNAL(clicked()), this, SLOT(Search()));
    QObject::connect(btnDownload, SIGNAL(clicked()), this, SLOT(Download()));
    QObject::connect(btnInstall, SIGNAL(clicked()), this, SLOT(onInstall()));
    QObject::connect(btnMain, SIGNAL(clicked()), this, SLOT(showMain()));
    QObject::connect(btnKernel, SIGNAL(clicked()), this, SLOT(showKernel()));
    QObject::connect(btnAbout, SIGNAL(clicked()), this, SLOT(About()));
}

Recovery::~Recovery()
{
    delete settingsFirmware;

    delete managerSearch;
    delete managerDownload;

    delete mainLayout;

    delete btnSearch;
    delete btnDownload;
    delete btnMain;
    delete btnKernel;
    delete btnAbout;

    delete textLog;
}

void Recovery::Search()
{
     QString site = url + "/update/recovery/" + version + "/version.ini";

     QObject::connect(managerSearch, SIGNAL(finished(QNetworkReply *)), this, SLOT(onSearchResult(QNetworkReply*))); //отправляем данные и получаем ответ успешно или ошибка
     managerSearch->get(QNetworkRequest(site));
}

void Recovery::onSearchResult(QNetworkReply *replyS)
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
        textLog->append("Доступна новая версия recovery!!! " + newVersion);
        textLog->append("Размер: " + fullSize);

        btnDownload->setEnabled(true);
    }
    else if (version.toInt() == newVersion.toInt())
    {
        textLog->clear();
        textLog->append("Обновлений нет!");
    }
}

void Recovery::Download()
{
    QString newVersion;

    QNetworkReply *reply;

    QSettings newSet(path + "/version.ini", QSettings::IniFormat);

    newVersion = newSet.value("General/newVersion").toString();

    QString site = url + "/update/recovery/" + newVersion + "/recovery.img";

    textLog->append("Начало Загрузки...");

    reply = managerDownload->get(QNetworkRequest(site));

    QObject::connect(reply, SIGNAL(downloadProgress(qint64, qint64)), this, SLOT(onProgress(qint64, qint64)));
    QObject::connect(managerDownload, SIGNAL(finished(QNetworkReply *)), this, SLOT(onDownloadResult(QNetworkReply*))); //отправляем данные и получаем ответ успешно или ошибка
}

void Recovery::onDownloadResult(QNetworkReply *replyD)
{
    QFile uFile(path + "/recovery.zip");

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
    textLog->append("Файл: " + path + "/recovery.img");

    btnInstall->setEnabled(true);
}

void Recovery::About()
{
    QMessageBox *boxAbout = new QMessageBox();

    boxAbout->setText("Text");

    boxAbout->show();
}

void Recovery::onProgress(qint64 receivedSize, qint64 totalSize)
{
    textLog->clear();
    textLog->append("Загружено: " + QString::number(receivedSize) + " / " + QString::number(totalSize));
}

void Recovery::showMain()
{
    UpdateCenter *upd = new UpdateCenter;
    upd->show();
}

void Recovery::showKernel()
{
    Kernel *boot = new Kernel;
    boot->show();
}

void Recovery::onInstall()
{
    textLog->append("Делаем резервную копию recovery");

    QString command = "busybox dd if=" + recovery + " of=" + backup + "/recovery_" + version + ".img";

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

    QString commandI = "busybox dd if=" + path + "/recovery.img of=" + recovery;
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

    system("su -c cp /system/recovery.ini /sdcard/recovery.ini");
    QSettings updSet("/sdcard/recovery.ini", QSettings::IniFormat);
    QSettings newSet(path + "/version.ini", QSettings::IniFormat);
    newVersion = newSet.value("General/newVersion").toString();
    updSet.setValue("General/Version", newVersion);
    updSet.sync();

    system("su -c busybox mount -o remount,rw /system");
    system("su -c cp /sdcard/recovery.ini /system/recovery.ini");
    system("su -c busybox chmod 644 /system/recovery.ini");

    textLog->append("Новая версия recovery: " + newVersion);
    textLog->append("Готово!");
}
