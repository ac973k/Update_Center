#ifndef ABOUTAPP_H
#define ABOUTAPP_H

#include <QWidget>

#include <QGridLayout>

#include <QPushButton>
#include <QTextEdit>
#include <QProgressBar>

#include <QNetworkAccessManager>
#include <QNetworkReply>

#include <QSettings>

class AboutApp : public QWidget
{
    Q_OBJECT

private:
    QString version;
    QString newVersion;
    QString url;
    QString urlApp;
    QString path;

    QGridLayout *mainLayout;

    QPushButton *btnSearch;
    QPushButton *btnDownload;

    QProgressBar *pBar;

    QTextEdit *textLog;

    QSettings *settingsFirmware;

    QNetworkAccessManager *managerSearch;
    QNetworkAccessManager *managerDownload;

public:
    explicit AboutApp(QWidget *parent = nullptr);
    ~AboutApp();

private slots:
    void Search();
    void Download();

    void onSearchResult(QNetworkReply *replyS);
    void onDownloadResult(QNetworkReply *replyD);

    void onProgress(qint64 receivedSize, qint64 totalSize);

};

#endif // ABOUTAPP_H
