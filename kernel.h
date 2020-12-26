#ifndef KERNEL_H
#define KERNEL_H

#include <QWidget>

#include <QGridLayout>

#include <QNetworkAccessManager>
#include <QNetworkReply>

#include <QSettings>

#include <QPushButton>

#include <QTextEdit>

#include <QProcess>

class Kernel : public QWidget
{
    Q_OBJECT

private:
    QString Name;
    QString version;
    QString url;
    QString path;
    QString boot;
    QString backup;
    QString newVersion;

    QGridLayout *mainLayout;

    QPushButton *btnSearch;
    QPushButton *btnDownload;
    QPushButton *btnAbout;
    QPushButton *btnMain;
    QPushButton *btnRecovery;
    QPushButton *btnInstall;
    QPushButton *btnCancel;

    QProcess *procInstall;
    QProcess *procBackup;

    QTextEdit *textLog;

    QSettings *settingsFirmware;

    QNetworkAccessManager *managerSearch;
    QNetworkAccessManager *managerDownload;

public:
    explicit Kernel(QWidget *parent = nullptr);
    ~Kernel();

private slots:
    void Search();
    void Download();
    void About();

    void onSearchResult(QNetworkReply *replyS);
    void onDownloadResult(QNetworkReply *replyD);

    void onProgress(qint64 receivedSize, qint64 totalSize);

    void showMain();
    void showRecovery();

    void onInstall();

};

#endif // KERNEL_H
