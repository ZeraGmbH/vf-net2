#ifndef VN_TCPSYSTEM_H
#define VN_TCPSYSTEM_H

#include "veinnet_global.h"
#include <ve_eventsystem.h>

#include <vh_handlemanager.h>

#include <QAbstractSocket>




class XiQNetPeer;
class XiQNetServer;

namespace google
{
  namespace protobuf
  {
    class Message;
  }
}

namespace VeinNet
{
  class ProtocolWrapper;

  /**
   * @brief handles ProtocolEvents and interfaces libxiqnet to send messages over the network
   * @todo needs something like interexchangable strategy objects that allow error recovery depending on the application needs
   */
  class VEINNETSHARED_EXPORT TcpSystem : public VeinEvent::EventSystem
  {
    Q_OBJECT
  public:
    explicit TcpSystem(QObject *t_parent = 0);
    virtual ~TcpSystem();

  signals:
    //client part
    void sigConnnectionEstablished(int t_connectionId);
    void sigConnnectionTimeout(int t_connectionId);

    //server part
    void sigClientConnected(int t_connectionId);
    void sigClientDisconnected(int t_connectionId);

  public slots:
    /**
     * @brief listens on QHostAddress::Any with port t_port
     * @param t_port
     * @note the QHostAddress::Any may have issues with ipv4 vs ipv6, but aparently they only affect udp connections so far
     */
    void startServer(quint16 t_port);
    void connectToServer(QString t_host, quint16 t_port);
    void onClientConnected(XiQNetPeer *t_networkPeer);
    void onConnectionEstablished();
    void onConnectionClosed();
    void onClientDisconnected();
    /**
     * @brief onMessageReceived
     * @param t_protobufMessage
     * @todo add support for multiple commands in one protobuf message
     */
    void onMessageReceived(google::protobuf::Message *t_protobufMessage);
    void onSocketError(QAbstractSocket::SocketError t_socketError);

    // EventSystem interface
  public:
    bool processEvent(QEvent *t_event) override;

  private:
    QList<XiQNetPeer *> m_waitingAuth;
    VeinHelper::HandleManager<int, XiQNetPeer *> m_peerList;
    XiQNetServer *m_server = 0;
    ProtocolWrapper *m_protoWrapper = 0;
  };
}
#endif // VN_TCPSYSTEM_H
