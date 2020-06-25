#ifndef VN_TCPSYSTEM_H
#define VN_TCPSYSTEM_H

#include "globalIncludes.h"
#include <ve_eventsystem.h>

#include <vh_handlemanager.h>

#include <QAbstractSocket>
#include <QUuid>


namespace VeinTcp
{
  class TcpPeer;
  class TcpServer;
}

namespace google
{
  namespace protobuf
  {
    class Message;
  }
}

namespace VeinNet
{
  /**
   * @brief TCP wire protocol implementation for protobuf::VeinProtocol data
   *
   * Handles ProtocolEvents, it interfaces libxiqnet to send messages over the network
   * @todo needs something like interexchangable strategy objects that allow error recovery depending on the application needs
   */
  class VFNET2_EXPORT TcpSystem : public VeinEvent::EventSystem
  {
    Q_OBJECT

  public:
    explicit TcpSystem(QObject *t_parent = nullptr);
    virtual ~TcpSystem();

  signals:
    //client part
    void sigConnnectionEstablished(QUuid t_connectionId);

    //server part
    void sigClientConnected(QUuid t_connectionId);
    void sigClientDisconnected(QUuid t_connectionId);

  public slots:
    /**
     * @brief listens on QHostAddress::Any with port t_port
     * @param t_port: Port number
     * @param t_systemdSocket: true: use systemd socket
     * @note the QHostAddress::Any may have issues with ipv4 vs ipv6, but aparently they only affect udp connections so far
     */
    bool startServer(quint16 t_port, bool t_systemdSocket=true);
    void connectToServer(const QString &t_host, quint16 t_port);

    /**
     * @brief adds the client to the waiting auth list and sends auth message
     * @param t_networkPeer
     * @todo implement real authentication
     */
    void onClientConnected(VeinTcp::TcpPeer *t_networkPeer);

    /**
     * @brief adds the server to the waiting auth list
     */
    void onConnectionEstablished(VeinTcp::TcpPeer *t_peer);

    /**
     * @brief cleans up the reference to the server peer
     */
    void onConnectionClosed(VeinTcp::TcpPeer *t_peer);

    /**
     * @brief Cleans up local references to the client and calls sigClientDisconnected();
     */
    void onClientDisconnected(VeinTcp::TcpPeer *t_peer);

    /**
     * @brief onMessageReceived
     * @todo add support for multiple commands in one protobuf message
     */
    void onMessageReceived(VeinTcp::TcpPeer *t_sender, QByteArray t_buffer);

    /**
     * @brief sends a NetworkStatusEvent about the socketerror
     * @param t_socketError
     */
    void onSocketError(VeinTcp::TcpPeer *t_peer, QAbstractSocket::SocketError t_socketError);

    // EventSystem interface
  public:
    bool processEvent(QEvent *t_event) override;

  private:
    /**
     * @brief List of clients waiting for approval
     */
    QList<VeinTcp::TcpPeer *> m_waitingAuth;

    /**
     * @brief List of active clients
     */
    QHash<QUuid, VeinTcp::TcpPeer *> m_peerList;

    /**
     * @brief The server instance
     */
    VeinTcp::TcpServer *m_server = nullptr;
  };
}
#endif // VN_TCPSYSTEM_H
