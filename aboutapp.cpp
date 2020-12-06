#include "aboutapp.h"

#include <QFile>

AboutApp::AboutApp(QWidget *parent) : QWidget(parent)
{
    settingsFirmware = new QSettings("/system/version.ini", QSettings::IniFormat);
    version = settingsFirmware->value("General/appVersion").toString();
    url = settingsFirmware->value("Geberal/URL").toString();
    urlApp = settingsFirmware->value("General/LinkApp").toString();
    path = settingsFirmware->value("General/Path").toString();

    managerSearch = new QNetworkAccessManager();
    managerDownload = new QNetworkAccessManager();

    mainLayout = new QGridLayout;

    btnSearch = new QPushButton("Проверить");
    btnDownload = new QPushButton("Загрузить");

    pBar = new QProgressBar;

    textLog = new QTextEdit;

    btnDownload->setEnabled(false);

    mainLayout->addWidget(btnSearch, 0, 0, 1, 2);
    mainLayout->addWidget(btnDownload, 1, 0, 1, 2);
    mainLayout->addWidget(pBar, 2, 0, 1, 2);
    mainLayout->addWidget(textLog, 3, 0, 1, 2);

    setLayout(mainLayout);

    textLog->setReadOnly(true);

    textLog->append("=====Update Center=====");
    textLog->append("Текущая Версия Приложения: " + version);

    QObject::connect(btnSearch, SIGNAL(clicked()), this, SLOT(Search()));
    QObject::connect(btnDownload, SIGNAL(clicked()), this, SLOT(Download()));
}

AboutApp::~AboutApp()
{
    delete settingsFirmware;

    delete managerSearch;
    delete managerDownload;

    delete mainLayout;

    delete btnSearch;
    delete btnDownload;

    delete pBar;

    delete textLog;
}

void AboutApp::Search()
{
     QString site = urlApp + "/update/app/version.ini";

     QObject::connect(managerSearch, SIGNAL(finished(QNetworkReply *)), this, SLOT(onSearchResult(QNetworkReply*))); //отправляем данные и получаем ответ успешно или ошибка
     managerSearch->get(QNetworkRequest(site));
}

void AboutApp::onSearchResult(QNetworkReply *replyS)
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
        textLog->append("Доступна новая версия Приложения!!! " + newVersion);
        textLog->append("Размер: " + fullSize);

        btnDownload->setEnabled(true);
    }
    else if (version.toInt() == newVersion.toInt())
    {
        textLog->clear();
        textLog->append("Обновлений нет!");
    }
}

void AboutApp::Download()
{
    QString newVersion;

    QNetworkReply *reply;

    QSettings newSet(path + "/version.ini", QSettings::IniFormat);

    newVersion = newSet.value("General/newVersion").toString();

    QString site = "https://github.com/ac973k/Update_Center/releases/download/" + newVersion + "/UpdateCenter.apk";

    textLog->append("Начало Загрузки...");

    reply = managerDownload->get(QNetworkRequest(site));

    QObject::connect(reply, SIGNAL(downloadProgress(qint64, qint64)), this, SLOT(onProgress(qint64, qint64)));
    QObject::connect(managerDownload, SIGNAL(finished(QNetworkReply *)), this, SLOT(onDownloadResult(QNetworkReply*))); //отправляем данные и получаем ответ успешно или ошибка
}

void AboutApp::onDownloadResult(QNetworkReply *replyD)
{
    QFile uFile(path + "/UpdateCenter.apk");

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
    textLog->append("Файл: " + path + "UpdateCenter.apk");
}

void AboutApp::onProgress(qint64 receivedSize, qint64 totalSize)
{
    pBar->setMaximum(totalSize);
    pBar->setValue(receivedSize);
}
