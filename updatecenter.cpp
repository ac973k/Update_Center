#include "updatecenter.h"

#include <QFile>
#include <QDir>

#include <QtAndroidExtras/QtAndroid>
#include <QtTest/QTest>

bool requestAndroidPermissions(){
//Request requiered permissions at runtime

const QVector<QString> permissions({
                                    "android.permission.WRITE_EXTERNAL_STORAGE",
                                    "android.permission.READ_EXTERNAL_STORAGE,"
                                    "android.permission.INTERNET"});

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
    path = settingsFirmware->value("General/Path").toString();

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
    btnAbout = new QPushButton("О Программе!");

    boxAbout = new QMessageBox;

    textLog = new QTextEdit;

    btnDownload->setEnabled(false);

    mainLayout->addWidget(btnSearch, 0, 0, 1, 2);
    mainLayout->addWidget(btnDownload, 1, 0, 1, 2);
    mainLayout->addWidget(textLog, 2, 0, 1, 2);
    mainLayout->addWidget(btnAbout, 3, 0, 1, 2);

    setLayout(mainLayout);

    textLog->setReadOnly(true);

    textLog->append("Модель: " + Model);
    textLog->append("Текущая Версия: " + version);
    textLog->append("Папка загрузки: " + path);

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

     QObject::connect(managerSearch, SIGNAL(finished(QNetworkReply *)), this, SLOT(onSearchResult(QNetworkReply*))); //отправляем данные и получаем ответ успешно или ошибка
     managerSearch->get(QNetworkRequest(site));
}

void UpdateCenter::onSearchResult(QNetworkReply *replyS)
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

    newVersion = newSet.value("General/Version").toString();

    if (version.toInt() < newVersion.toInt())
    {
        textLog->clear();
        textLog->append("Доступна новая версия!!! " + newVersion);
        textLog->append("Размер: " + fullSize);

        btnDownload->setEnabled(true);
    }
    else if (version.toInt() == newVersion.toInt())
    {
        textLog->clear();
        textLog->append("Обновлений нет!");
    }
}

void UpdateCenter::Download()
{
    QString newVersion;

    QNetworkReply *reply;

    QSettings newSet(path + "/version.ini", QSettings::IniFormat);

    newVersion = newSet.value("General/Version").toString();

    QString site = url + "/update/" + newVersion + "/update.zip";

    textLog->append("Начало Загрузки...");

    reply = managerDownload->get(QNetworkRequest(site));

    QObject::connect(reply, SIGNAL(downloadProgress(qint64, qint64)), this, SLOT(onProgress(qint64, qint64)));
    QObject::connect(managerDownload, SIGNAL(finished(QNetworkReply *)), this, SLOT(onDownloadResult(QNetworkReply*))); //отправляем данные и получаем ответ успешно или ошибка
}

void UpdateCenter::onDownloadResult(QNetworkReply *replyD)
{
    QFile uFile(path + "/Update.zip");

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
    textLog->append("Файл: " + path + "/Update.zip");
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
    textLog->append("Загружено: " + QString::number(receivedSize) + " / " + QString::number(totalSize));
}
