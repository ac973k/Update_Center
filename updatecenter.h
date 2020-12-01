#ifndef UPDATECENTER_H
#define UPDATECENTER_H

#include <QWidget>

#include <QGridLayout>

#include <QNetworkAccessManager>
#include <QNetworkReply>

#include <QSettings>

#include <QPushButton>

#include <QTextEdit>

#include <QMessageBox>

class UpdateCenter : public QWidget
{
    Q_OBJECT

private:
    QString Model;
    QString version;
    QString url;
    QString path;

    QGridLayout *mainLayout;

    QPushButton *btnSearch;
    QPushButton *btnDownload;
    QPushButton *btnAbout;

    QTextEdit *textLog;

    QMessageBox *boxAbout;

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
};
#endif // UPDATECENTER_H
