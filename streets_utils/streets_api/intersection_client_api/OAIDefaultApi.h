/**
 * Intersection Model API
 * No description provided (generated by Openapi Generator https://github.com/openapitools/openapi-generator)
 *
 * The version of the OpenAPI document: 1.0.0
 *
 * NOTE: This class is auto generated by OpenAPI Generator (https://openapi-generator.tech).
 * https://openapi-generator.tech
 * Do not edit the class manually.
 */

#ifndef OAI_OAIDefaultApi_H
#define OAI_OAIDefaultApi_H

#include "OAIHelpers.h"
#include "OAIHttpRequest.h"
#include "OAIServerConfiguration.h"

#include "OAIIntersection_info.h"
#include "OAILanelet_info.h"
#include <QString>

#include <QObject>
#include <QByteArray>
#include <QStringList>
#include <QList>
#include <QNetworkAccessManager>

namespace OpenAPI {

class OAIDefaultApi : public QObject {
    Q_OBJECT

public:
    OAIDefaultApi(const int timeOut = 0);
    ~OAIDefaultApi();

    void initializeServerConfigs();
    int setDefaultServerValue(int serverIndex,const QString &operation, const QString &variable,const QString &val);
    void setServerIndex(const QString &operation, int serverIndex);
    void setApiKey(const QString &apiKeyName, const QString &apiKey);
    void setBearerToken(const QString &token);
    void setUsername(const QString &username);
    void setPassword(const QString &password);
    void setTimeOut(const int timeOut);
    void setWorkingDirectory(const QString &path);
    void setNetworkAccessManager(QNetworkAccessManager* manager);
    int addServerConfiguration(const QString &operation, const QUrl &url, const QString &description = "", const QMap<QString, OAIServerVariable> &variables = QMap<QString, OAIServerVariable>());
    void setNewServerForAllOperations(const QUrl &url, const QString &description = "", const QMap<QString, OAIServerVariable> &variables =  QMap<QString, OAIServerVariable>());
    void setNewServer(const QString &operation, const QUrl &url, const QString &description = "", const QMap<QString, OAIServerVariable> &variables =  QMap<QString, OAIServerVariable>());
    void addHeaders(const QString &key, const QString &value);
    void enableRequestCompression();
    void enableResponseCompression();
    void abortRequests();
    QString getParamStylePrefix(const QString &style);
    QString getParamStyleSuffix(const QString &style);
    QString getParamStyleDelimiter(const QString &style, const QString &name, bool isExplode);

    /**
    * @param[in]  link_lanelet_id qint32 [required]
    */
    void getConflictLanelets(const qint32 &link_lanelet_id);


    void getIntersectionInfo();


    void listDepartureLanelets();


    void listEntryLanelets();


    void listLinkLanelets();


private:
    QMap<QString,int> _serverIndices;
    QMap<QString,QList<OAIServerConfiguration>> _serverConfigs;
    QMap<QString, QString> _apiKeys;
    QString _bearerToken;
    QString _username;
    QString _password;
    int _timeOut;
    QString _workingDirectory;
    QNetworkAccessManager* _manager;
    QMap<QString, QString> _defaultHeaders;
    bool _isResponseCompressionEnabled;
    bool _isRequestCompressionEnabled;

    void getConflictLaneletsCallback(OAIHttpRequestWorker *worker);
    void getIntersectionInfoCallback(OAIHttpRequestWorker *worker);
    void listDepartureLaneletsCallback(OAIHttpRequestWorker *worker);
    void listEntryLaneletsCallback(OAIHttpRequestWorker *worker);
    void listLinkLaneletsCallback(OAIHttpRequestWorker *worker);

signals:

    void getConflictLaneletsSignal(QList<OAILanelet_info> summary);
    void getIntersectionInfoSignal(OAIIntersection_info summary);
    void listDepartureLaneletsSignal(QList<OAILanelet_info> summary);
    void listEntryLaneletsSignal(QList<OAILanelet_info> summary);
    void listLinkLaneletsSignal(QList<OAILanelet_info> summary);

    void getConflictLaneletsSignalFull(OAIHttpRequestWorker *worker, QList<OAILanelet_info> summary);
    void getIntersectionInfoSignalFull(OAIHttpRequestWorker *worker, OAIIntersection_info summary);
    void listDepartureLaneletsSignalFull(OAIHttpRequestWorker *worker, QList<OAILanelet_info> summary);
    void listEntryLaneletsSignalFull(OAIHttpRequestWorker *worker, QList<OAILanelet_info> summary);
    void listLinkLaneletsSignalFull(OAIHttpRequestWorker *worker, QList<OAILanelet_info> summary);

    void getConflictLaneletsSignalE(QList<OAILanelet_info> summary, QNetworkReply::NetworkError error_type, QString error_str);
    void getIntersectionInfoSignalE(OAIIntersection_info summary, QNetworkReply::NetworkError error_type, QString error_str);
    void listDepartureLaneletsSignalE(QList<OAILanelet_info> summary, QNetworkReply::NetworkError error_type, QString error_str);
    void listEntryLaneletsSignalE(QList<OAILanelet_info> summary, QNetworkReply::NetworkError error_type, QString error_str);
    void listLinkLaneletsSignalE(QList<OAILanelet_info> summary, QNetworkReply::NetworkError error_type, QString error_str);

    void getConflictLaneletsSignalEFull(OAIHttpRequestWorker *worker, QNetworkReply::NetworkError error_type, QString error_str);
    void getIntersectionInfoSignalEFull(OAIHttpRequestWorker *worker, QNetworkReply::NetworkError error_type, QString error_str);
    void listDepartureLaneletsSignalEFull(OAIHttpRequestWorker *worker, QNetworkReply::NetworkError error_type, QString error_str);
    void listEntryLaneletsSignalEFull(OAIHttpRequestWorker *worker, QNetworkReply::NetworkError error_type, QString error_str);
    void listLinkLaneletsSignalEFull(OAIHttpRequestWorker *worker, QNetworkReply::NetworkError error_type, QString error_str);

    void abortRequestsSignal();
    void allPendingRequestsCompleted();
};

} // namespace OpenAPI
#endif
