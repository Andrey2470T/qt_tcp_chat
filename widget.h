#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTextEdit>
#include <QLineEdit>
#include <QTcpSocket>
#include <QTcpServer>

class ChatSendEdit;

class Chat : public QWidget
{
    Q_OBJECT

    Q_DISABLE_COPY(Chat);

    QTcpSocket *socket;

    QLabel *connect_status;

    QPushButton *connect_btn;
    QPushButton *send_btn;
    QPushButton *clear_btn;

    QTextEdit *chatlogs_list;
    ChatSendEdit *address_edit;
    ChatSendEdit *send_edit;

    bool isTextCleared;

public slots:
    void socketState() const;
    void connect() const;
    void sendText(const QString &text);
    void clearAll();
    void disconnect() const;
    void onConnect();
    void onDisconnect();
    void onReadText();
    void onError(QAbstractSocket::SocketError err) const;

public:
    Chat(QWidget *parent = nullptr);
    ~Chat();

    void disableAllWidgets(bool yes);
};
#endif // WIDGET_H


class Server : public QObject
{
    Q_OBJECT

    Q_DISABLE_COPY(Server)

    QTcpServer *server;
    QMap<quint16, QTcpSocket*> clients;

public slots:
    void newConnection();
    void readAndResendText();
    void onDisconnectClient();

public:
    Server();
};

class ChatSendEdit : public QLineEdit
{
    Q_OBJECT

    bool defaultTextDeleted;
    QLabel *default_text;
    QPushButton *send_btn;

public slots:
    void onTextChanged(const QString &text);
    void onTextSend();

signals:
    void onTextResend(const QString &text);
public:
    ChatSendEdit(QPushButton *send_button=nullptr, const QString &default_label="");

    ~ChatSendEdit()
    {
        close();
    }

    bool isTextSendable() const
    {
        return defaultTextDeleted;
    }
};
