#ifndef RECOVERY_H
#define RECOVERY_H

#include <QWidget>

#include <QGridLayout>

#include <QNetworkAccessManager>
#include <QNetworkReply>

#include <QSettings>

#include <QPushButton>

#include <QTextEdit>

#include <QProcess>

class Recovery : public QWidget
{
    Q_OBJECT

private:
    QString Name;
    QString version;
    QString url;
    QString path;
    QString recovery;
    QString backup;
    QString newVersion;

    QGridLayout *mainLayout;

    QPushButton *btnSearch;
    QPushButton *btnDownload;
    QPushButton *btnAbout;
    QPushButton *btnMain;
    QPushButton *btnKernel;
    QPushButton *btnInstall;

    QProcess *procInstall;
    QProcess *procBackup;

    QTextEdit *textLog;

    QSettings *settingsFirmware;

    QNetworkAccessManager *managerSearch;
    QNetworkAccessManager *managerDownload;

public:
    explicit Recovery(QWidget *parent = nullptr);
    ~Recovery();

private slots:
    void Search();
    void Download();
    void About();

    void onSearchResult(QNetworkReply *replyS);
    void onDownloadResult(QNetworkReply *replyD);

    void onProgress(qint64 receivedSize, qint64 totalSize);

    void showMain();
    void showKernel();

    void onInstall();

};

#endif // RECOVERY_H
