#ifndef UPDATECENTER_H
#define UPDATECENTER_H

#include <QWidget>

#include <QGridLayout>

#include <QNetworkAccessManager>
#include <QNetworkReply>

#include <QSettings>

#include <QPushButton>

#include <QTextEdit>

class UpdateCenter : public QWidget
{
    Q_OBJECT

private:
    QString Model;
    QString version;
    QString url;
    QString path;
    QString newVersion;

    QGridLayout *mainLayout;

    QPushButton *btnSearch;
    QPushButton *btnDownload;
    QPushButton *btnAbout;
    QPushButton *btnKernel;
    QPushButton *btnRecovery;

    QTextEdit *textLog;

    QSettings *settingsFirmware;

    QNetworkAccessManager *managerSearch;
    QNetworkAccessManager *managerDownload;

public:
    UpdateCenter(QWidget *parent = nullptr);
    ~UpdateCenter();

private slots:
    void Search();
    void Download();
    void About();

    void onSearchResult(QNetworkReply *replyS);
    void onDownloadResult(QNetworkReply *replyD);

    void onProgress(qint64 receivedSize, qint64 totalSize);

    void showKernel();
    void showRecovery();
};
#endif // UPDATECENTER_H
