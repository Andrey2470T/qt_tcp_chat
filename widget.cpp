#include "widget.h"
#include <QApplication>
#include <QDateTime>

Chat::Chat(QWidget *parent)
    : QWidget(parent)
{
    setWindowTitle("Qt Simple TCP chat");
    resize(1024, 680);

    isTextCleared = true;

    QGridLayout *layout = new QGridLayout;

    QHBoxLayout *up_btns_layout = new QHBoxLayout;

    connect_btn = new QPushButton("Connect");
    connect_btn->setMaximumWidth(1024/6);

    QObject::connect(connect_btn, &QPushButton::clicked, this, &Chat::socketState);

    address_edit = new ChatSendEdit(connect_btn, "Enter IP-address to connect");
    address_edit->setMaximumWidth(1024/6);

    up_btns_layout->addWidget(connect_btn);
    up_btns_layout->addWidget(address_edit);

    QVBoxLayout *chat_layout = new QVBoxLayout;

    connect_status = new QLabel("You are not connected yet");

    chatlogs_list = new QTextEdit;
    chatlogs_list->setReadOnly(true);

    send_btn = new QPushButton("Send");
    send_btn->setMaximumWidth(1024/6);

    send_edit = new ChatSendEdit(send_btn, "Print something...");

    chat_layout->addWidget(connect_status);
    chat_layout->addWidget(chatlogs_list);
    chat_layout->addWidget(send_edit);

    QHBoxLayout *btns_layout = new QHBoxLayout;

    clear_btn = new QPushButton("Clear");
    clear_btn->setMaximumWidth(1024/6);
    clear_btn->setDisabled(true);

    btns_layout->addWidget(send_btn);
    btns_layout->addWidget(clear_btn);

    disableAllWidgets(true);

    layout->addLayout(up_btns_layout, 0, 0);
    layout->addLayout(chat_layout, 1, 0);
    layout->addLayout(btns_layout, 2, 0);

    setLayout(layout);

    socket = new QTcpSocket(this);

    QObject::connect(socket, &QTcpSocket::connected, this, &Chat::onConnect);
    QObject::connect(socket, &QTcpSocket::errorOccurred, this, &Chat::onError);
    QObject::connect(socket, &QTcpSocket::disconnected, this, &Chat::onDisconnect);
    QObject::connect(send_btn, &QPushButton::clicked, send_edit, &ChatSendEdit::onTextSend);
    QObject::connect(send_edit, &ChatSendEdit::onTextResend, this, &Chat::sendText);
    QObject::connect(socket, &QTcpSocket::readyRead, this, &Chat::onReadText);
    QObject::connect(clear_btn, &QPushButton::clicked, this, &Chat::clearAll);
}

Chat::~Chat()
{
    delete socket;
}

void Chat::socketState() const
{
    QAbstractSocket::SocketState state = socket->state();

    if (state == QAbstractSocket::UnconnectedState)
        connect();
    else if (state == QAbstractSocket::ConnectedState)
        disconnect();
}

void Chat::connect() const
{
    connect_status->setText("Connection...");
    address_edit->setDisabled(true);
    socket->connectToHost(QHostAddress(address_edit->text()), 3200, QIODevice::ReadWrite);

    socket->waitForConnected(10000);
}

void Chat::disconnect() const
{
    connect_status->setText("Disconnection...");
    address_edit->setDisabled(false);
    socket->disconnectFromHost();

    socket->waitForDisconnected(10000);
}

void Chat::onConnect()
{
    connect_status->setText("Connection has been established!");
    connect_btn->setText("Disconnect");

    disableAllWidgets(false);
}

void Chat::onDisconnect()
{
    connect_status->setText("You are not connected yet");
    connect_btn->setText("Connect");

    disableAllWidgets(true);
}

void Chat::onError(QAbstractSocket::SocketError err) const
{
    address_edit->setDisabled(false);
    QString error_type;

    switch (quint16(err))
    {
        case 0:
            error_type = "ConnectionRefusedError";
            break;
        case 1:
            error_type = "RemoteHoseClosedError";
            break;
        case 2:
            error_type = "HostNotFoundError";
            break;
        case 3:
            error_type = "SocketAccessError";
            break;
        case 4:
            error_type = "SocketResourceError";
            break;
        case 5:
            error_type = "SocketTimeoutError";
            break;
        case 6:
            error_type = "DatagramTooLargeError";
            break;
        case 7:
            error_type = "NetworkError";
            break;
        case 8:
            error_type = "AddressInUseError";
            break;
        case 9:
            error_type = "SocketAddressNotAvailableError";
            break;
        case 10:
            error_type = "UnsupportedSocketOperationError";
            break;
        case 11:
            error_type = "UnfinishedSocketOperationError";
            break;
        case 12:
            error_type = "ProxyAuthenticationRequiredError";
            break;
        case 13:
            error_type = "SslHandshakeFailedError";
            break;
        case 14:
            error_type = "ProxyConnectionRefusedError";
            break;
        case 15:
            error_type = "ProxyConnectionClosedError";
            break;
        case 16:
            error_type = "ProxyConnectionTimeoutError";
            break;
        case 17:
            error_type = "ProxyNotFoundError";
            break;
        case 18:
            error_type = "ProxyProtocolError";
            break;
        case 19:
            error_type = "OperationError";
            break;
        case 20:
            error_type = "SslInternalError";
            break;
        case 21:
            error_type = "SslInvalidUserDataError";
            break;
        case 22:
            error_type = "TemporaryError";
            break;

    }
    connect_status->setText("Connection error: " + error_type);
    socket->close();
}

void Chat::onReadText()
{
    QTcpSocket *sender_socket = (QTcpSocket*)sender();

    QTextStream stream(sender_socket);

    QString text;

    stream >> text;

    chatlogs_list->append(
        "[" + QDateTime::currentDateTime().toString() + "]" + text + "\n");

    isTextCleared = false;
    clear_btn->setDisabled(false);
}


void Chat::disableAllWidgets(bool yes)
{
    chatlogs_list->setDisabled(yes);
    send_edit->setDisabled(yes);

    if (!isTextCleared)
        clear_btn->setDisabled(yes);

    if (send_edit->isTextSendable())
        send_btn->setDisabled(yes);
    if (address_edit->isTextSendable())
        connect_btn->setDisabled(yes);
}

void Chat::sendText(const QString &text)
{
    if (text.isEmpty() || socket->state() != QAbstractSocket::ConnectedState)
        return;

    QTextStream text_stream(socket);

    QString send_text = "_" + socket->localAddress().toString() + "_" + text;

    text_stream << send_text;

    socket->waitForBytesWritten(10000);
}

void Chat::clearAll()
{
    chatlogs_list->setText("");

    isTextCleared = true;
    clear_btn->setDisabled(true);
}

Server::Server() : QObject(nullptr)
{
    server = new QTcpServer(this);

    connect(server, &QTcpServer::newConnection, this, &Server::newConnection);

    QTextStream listened(stdout);

    if (!server->listen(QHostAddress::AnyIPv4, 3200))
        listened << "Couldn't start the server and listen on the port 3200 :(" << Qt::endl;
    else
        listened << "Successful starting and listening on the port 3200!" << Qt::endl;
}

void Server::newConnection()
{
    QTextStream connected(stdout);
    connected << "Connected!" << Qt::endl;

    QTcpSocket *clientSocket = server->nextPendingConnection();
    clients.insert(clientSocket->socketDescriptor(), clientSocket);

    qInfo() << "clients count: " << clients.size() << Qt::endl;

    QObject::connect(clientSocket, &QTcpSocket::readyRead, this, &Server::readAndResendText);
    QObject::connect(clientSocket, &QTcpSocket::disconnected, this, &Server::onDisconnectClient);
}

void Server::readAndResendText()
{
    QTcpSocket *sender_socket = (QTcpSocket*)sender();

    QTextStream stream(sender_socket);

    QString send_text;

    stream >> send_text;

    qInfo() << send_text << Qt::endl;

    auto i = clients.begin();

    while (i != clients.end())
    {
        QTextStream to_client_stream(i.value());
        to_client_stream << send_text;

        i++;
    }
}

void Server::onDisconnectClient()
{
    QTcpSocket *disconnect_client = (QTcpSocket*)sender();

    quint16 descriptor = disconnect_client->socketDescriptor();

    clients.remove(descriptor);
}

ChatSendEdit::ChatSendEdit(QPushButton *send_button, const QString &default_label) :
    QLineEdit(nullptr)
{
    send_btn = send_button;
    defaultTextDeleted = false;

    default_text = new QLabel(this);

    QFont cur_font = default_text->font();
    cur_font.setStyle(QFont::StyleItalic);
    default_text->setFont(cur_font);

    QPalette cur_palette = default_text->palette();
    cur_palette.setColor(QPalette::Text, QColor(Qt::gray));
    default_text->setPalette(cur_palette);

    default_text->setText(default_label);
    default_text->setVisible(true);

    send_btn->setDisabled(true);

    QObject::connect(this, &QLineEdit::textChanged, this, &ChatSendEdit::onTextChanged);
}

void ChatSendEdit::onTextChanged(const QString &text)
{
    if (!defaultTextDeleted)
    {
        if (!text.isEmpty())
        {
            default_text->setVisible(false);
            defaultTextDeleted = true;
            send_btn->setDisabled(false);
        }
    }
    else
    {
        if (text.isEmpty())
        {
            default_text->setVisible(true);
            defaultTextDeleted = false;
            send_btn->setDisabled(true);
        }
    }

}

void ChatSendEdit::onTextSend()
{
    QString cur_text = text();

    setText("");
    default_text->setVisible(true);
    defaultTextDeleted = false;
    send_btn->setDisabled(true);

    emit onTextResend(cur_text);
}

