#include "updatecenter.h"

#include <QFile>

#include <QtAndroidExtras/QtAndroid>
#include <QtTest/QTest>

bool requestAndroidPermissions(){
//Request requiered permissions at runtime

const QVector<QString> permissions({
                                    "android.permission.WRITE_EXTERNAL_STORAGE",
                                    "android.permission.READ_EXTERNAL_STORAGE"});

for(const QString &permission : permissions){
    auto result = QtAndroid::checkPermission(permission);
    if(result == QtAndroid::PermissionResult::Denied){
        auto resultHash = QtAndroid::requestPermissionsSync(QStringList({permission}));
        if(resultHash[permission] == QtAndroid::PermissionResult::Denied)
            return false;
    }
}

return true;
}

UpdateCenter::UpdateCenter(QWidget *parent)
    : QWidget(parent)
{
    requestAndroidPermissions();

    settingsFirmware = new QSettings("/system/version.ini", QSettings::IniFormat);
    Model = settingsFirmware->value("General/Model").toString();
    version = settingsFirmware->value("General/Version").toString();
    url = settingsFirmware->value("General/URL").toString();

    managerSearch = new QNetworkAccessManager();
    managerDownload = new QNetworkAccessManager();

    mainLayout = new QGridLayout;

    btnSearch = new QPushButton("Search");
    btnDownload = new QPushButton("Download");
    btnAbout = new QPushButton("About");

    boxAbout = new QMessageBox;

    textLog = new QTextEdit;

    btnDownload->setEnabled(false);

    mainLayout->addWidget(btnSearch, 0, 0, 1, 2);
    mainLayout->addWidget(btnDownload, 1, 0, 1, 2);
    mainLayout->addWidget(textLog, 2, 0, 1, 2);
    mainLayout->addWidget(btnAbout, 3, 0, 1, 2);

    setLayout(mainLayout);

    textLog->setReadOnly(true);

    textLog->append("Model: " + Model);
    textLog->append("Current Version: " + version);

    QObject::connect(btnSearch, SIGNAL(clicked()), this, SLOT(Search()));
    QObject::connect(btnDownload, SIGNAL(clicked()), this, SLOT(Download()));
    QObject::connect(btnAbout, SIGNAL(clicked()), this, SLOT(About()));
}

UpdateCenter::~UpdateCenter()
{
    delete settingsFirmware;

    delete managerSearch;
    delete managerDownload;

    delete mainLayout;

    delete btnSearch;
    delete btnDownload;
    delete btnAbout;

    delete textLog;

    delete boxAbout;
}

void UpdateCenter::Search()
{
     QString site = url + "/update/" + version + "/version.ini";

     QObject::connect(managerSearch, SIGNAL(finished(QNetworkReply *)), this, SLOT(onSearchResult(QNetworkReply*))); //отправляем данные и получаем ответ от вк успешно или ошибка
     managerSearch->get(QNetworkRequest(site));
}

void UpdateCenter::onSearchResult(QNetworkReply *replyS)
{
    QFile nFile("/sdcard/version.ini");

    if(!replyS->error())
    {
        nFile.open(QFile::WriteOnly);
        nFile.write(replyS->readAll());
    }

    nFile.close();

    replyS->abort();
    replyS->deleteLater();

    QString newVersion;
    QString fullSize;

    QSettings newSet("/sdcard/version.ini", QSettings::IniFormat);
    fullSize = newSet.value("General/Size").toString();

    newVersion = newSet.value("General/Version").toString();

    if (version.toInt() < newVersion.toInt())
    {
        textLog->clear();
        textLog->append("Available new version!!! " + newVersion);
        textLog->append("Size: " + fullSize);

        btnDownload->setEnabled(true);
    }
    else if (version.toInt() == newVersion.toInt())
    {
        textLog->clear();
        textLog->append("Update Not Found!");
    }
}

void UpdateCenter::Download()
{
    QString newVersion;

    QNetworkReply *reply;

    QSettings newSet("/sdcard/version.ini", QSettings::IniFormat);

    newVersion = newSet.value("General/Version").toString();

    QString site = url + "/update/" + newVersion + "/update.zip";

    textLog->append("Start Download...");

    reply = managerDownload->get(QNetworkRequest(site));

    QObject::connect(reply, SIGNAL(downloadProgress(qint64, qint64)), this, SLOT(onProgress(qint64, qint64)));
    QObject::connect(managerDownload, SIGNAL(finished(QNetworkReply *)), this, SLOT(onDownloadResult(QNetworkReply*))); //отправляем данные и получаем ответ успешно или ошибка
}

void UpdateCenter::onDownloadResult(QNetworkReply *replyD)
{
    QFile uFile("/sdcard/Update.zip");

    if(!replyD->error())
    {
        uFile.open(QFile::WriteOnly);
        uFile.write(replyD->readAll());
    }

    uFile.close();

    replyD->abort();
    replyD->deleteLater();

    textLog->append("Download Completed!");
    textLog->append("File: /sdcard/Update.zip");
}

void UpdateCenter::About()
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

void UpdateCenter::onProgress(qint64 receivedSize, qint64 totalSize)
{
    textLog->clear();
    textLog->append("Download: " + QString::number(receivedSize) + " / " + QString::number(totalSize));
}
