#include "vn_tcpsystem.h"

#include "vn_protocolevent.h"
#include "vn_networkstatusevent.h"

#include <vtcp_peer.h>
#include <vtcp_server.h>

Q_LOGGING_CATEGORY(VEIN_NET_TCP, "\e[1;33m<Vein.Network.Tcp>\033[0m")
Q_LOGGING_CATEGORY(VEIN_NET_TCP_VERBOSE, "\e[0;33m<Vein.Network.Tcp>\033[0m")

using namespace VeinEvent;

namespace VeinNet
{
  TcpSystem::TcpSystem(QObject *t_parent) :
    EventSystem(t_parent) ,
    m_server(new VeinTcp::TcpServer(this))
  {
    vCDebug(VEIN_NET_TCP)  << "Created TCP system";
    connect(m_server, &VeinTcp::TcpServer::sigClientConnected, this, &TcpSystem::onClientConnected);
  }

  TcpSystem::~TcpSystem()
  {
    vCDebug(VEIN_NET_TCP)  << "Destroyed TCP system";
  }

  void TcpSystem::startServer(quint16 t_port)
  {
    Q_ASSERT(m_server->isListening() == false);

    m_server->startServer(t_port);
  }

  void TcpSystem::connectToServer(const QString &t_host, quint16 t_port)
  {
    VF_ASSERT(t_host.isEmpty() == false, "Empty host");
    VF_ASSERT(t_port > 0, "Port must be > 0");

    vCDebug(VEIN_NET_TCP) << "Attempting connection to:"<< t_host << "on port:" << t_port;

    VeinTcp::TcpPeer *tmpPeer = new VeinTcp::TcpPeer(this);
    connect(tmpPeer, SIGNAL(sigSocketError(QAbstractSocket::SocketError)), this, SLOT(onSocketError(QAbstractSocket::SocketError)));
    connect(tmpPeer, &VeinTcp::TcpPeer::sigConnectionEstablished, this, &TcpSystem::onConnectionEstablished);
    connect(tmpPeer, &VeinTcp::TcpPeer::sigConnectionClosed, this, &TcpSystem::onConnectionClosed);
    tmpPeer->startConnection(t_host, t_port);
  }

  void TcpSystem::onClientConnected(VeinTcp::TcpPeer *t_networkPeer)
  {
    Q_ASSERT(t_networkPeer != 0);

    connect(t_networkPeer, &VeinTcp::TcpPeer::sigMessageReceived, this, &TcpSystem::onMessageReceived);
    m_waitingAuth.append(t_networkPeer);

    connect(t_networkPeer, &VeinTcp::TcpPeer::sigConnectionClosed, this, &TcpSystem::onClientDisconnected);

    t_networkPeer->sendMessage(QByteArray("welcome"));
#ifdef VN2_LEGACY_UNREACHABLE
    /** @todo implement authentication */
    protobuf::VeinProtocol *protoAuth = new protobuf::VeinProtocol();
    t_networkPeer->sendMessage(protoAuth);
    delete protoAuth;
#endif // VN2_LEGACY_UNREACHABLE
  }

  void TcpSystem::onConnectionEstablished()
  {
    /// @todo requiring QObject::sender() is bad design
    Q_ASSERT(QObject::sender()!=0);
    VeinTcp::TcpPeer *tmpPeer = qobject_cast<VeinTcp::TcpPeer *>(QObject::sender());
    Q_ASSERT(tmpPeer != 0);

    m_waitingAuth.append(tmpPeer);

    connect(tmpPeer, &VeinTcp::TcpPeer::sigMessageReceived, this, &TcpSystem::onMessageReceived);

    tmpPeer->sendMessage(QByteArray("hello"));
#ifdef VN2_LEGACY_UNREACHABLE
    /** @todo implement authentication */
    protobuf::VeinProtocol *protoAuth = new protobuf::VeinProtocol();
    tmpPeer->sendMessage(protoAuth);
    delete protoAuth;
#endif // VN2_LEGACY_UNREACHABLE
  }

  void TcpSystem::onConnectionClosed()
  {
    /// @todo requiring QObject::sender() is bad design
    Q_ASSERT(QObject::sender()!=0);
    VeinTcp::TcpPeer *tmpPeer = qobject_cast<VeinTcp::TcpPeer *>(QObject::sender());
    Q_ASSERT(tmpPeer != 0);

    int tmpPeerId = tmpPeer->getPeerId();

    vCDebug(VEIN_NET_TCP) << "Disconnected from server with ID:" << tmpPeerId;
    m_waitingAuth.removeAll(tmpPeer);
    m_peerList.remove(tmpPeerId);
  }

  void TcpSystem::onClientDisconnected()
  {
    /// @todo requiring QObject::sender() is bad design
    Q_ASSERT(QObject::sender()!=0);
    VeinTcp::TcpPeer *tmpPPeer = qobject_cast<VeinTcp::TcpPeer *>(QObject::sender());
    Q_ASSERT(tmpPPeer != 0);

    int tmpPeerId = tmpPPeer->getPeerId();

    NetworkStatusEvent *sEvent = new NetworkStatusEvent(NetworkStatusEvent::NetworkStatus::NSE_DISCONNECTED, tmpPeerId);
    emit sigSendEvent(sEvent);

    vCDebug(VEIN_NET_TCP) << "Client disconnected with ID:" << tmpPeerId << "sent NetworkStatusEvent:" << sEvent;
    m_waitingAuth.removeAll(tmpPPeer);
    m_peerList.remove(tmpPeerId);
    emit sigClientDisconnected(tmpPeerId);
    delete tmpPPeer;
  }

  void TcpSystem::onMessageReceived(QByteArray t_buffer)
  {
    Q_ASSERT(t_buffer.isNull() == false);

    VeinTcp::TcpPeer *tmpPPeer = qobject_cast<VeinTcp::TcpPeer *>(QObject::sender());
    Q_ASSERT(tmpPPeer != 0);

    //vCDebug(VEIN_NET_TCP_VERBOSE)  << "Message received:" << proto->DebugString().c_str();
    if(m_waitingAuth.contains(tmpPPeer))
    {
      int newId = m_peerList.append(tmpPPeer);
      vCDebug(VEIN_NET_TCP) << "New connection with id:" << newId;
      tmpPPeer->setPeerId(newId);

      m_waitingAuth.removeAll(tmpPPeer);
      emit sigConnnectionEstablished(newId);
    }
    ProtocolEvent *tmpEvent = new ProtocolEvent(ProtocolEvent::EventOrigin::EO_REMOTE);
    tmpEvent->setBuffer(t_buffer);
    tmpEvent->setPeerId(tmpPPeer->getPeerId());
#ifdef VN2_LEGACY_UNREACHABLE
    if(proto->command_size()>0)
    {
      vCDebug(VEIN_NET_TCP_VERBOSE) << "Received protocol event of type:" << proto->command(0).datatype();
    }
#endif // VN2_LEGACY_UNREACHABLE
    emit sigSendEvent(tmpEvent);
  }

  void TcpSystem::onSocketError(QAbstractSocket::SocketError t_socketError)
  {
    VeinTcp::TcpPeer *tmpPPeer = qobject_cast<VeinTcp::TcpPeer *>(QObject::sender());
    Q_ASSERT(tmpPPeer != 0);

    NetworkStatusEvent *sEvent = new NetworkStatusEvent(NetworkStatusEvent::NetworkStatus::NSE_SOCKET_ERROR, tmpPPeer->getPeerId());
    sEvent->setError(t_socketError);
    qCCritical(VEIN_NET_TCP) << "Connection error on network with id:" << tmpPPeer->getPeerId() << "error:" << tmpPPeer->getErrorString() << "sent NetworkStatusEvent:" << sEvent;
    emit sigSendEvent(sEvent);
  }

  bool TcpSystem::processEvent(QEvent *t_event)
  {
    Q_ASSERT(t_event != 0);

    bool retVal = false;
    if(t_event->type()==ProtocolEvent::getEventType())
    {
      ProtocolEvent *pEvent=0;
      pEvent = static_cast<ProtocolEvent *>(t_event);
      Q_ASSERT(pEvent != 0);
      /// @todo rework event origin concept
      //do not process protocol events from foreign systems, that is the job of NetworkSystem
      if(pEvent->isOfLocalOrigin() == true)
      {
        //send to all
        if(pEvent->receivers().isEmpty())
        {
          const auto tmpPeerlist = m_peerList.values();
          vCDebug(VEIN_NET_TCP_VERBOSE) << "Sending ProtocolEvent" << pEvent << "to receivers:" << tmpPeerlist;// << pEvent->protobuf()->DebugString().c_str();
          for(VeinTcp::TcpPeer *tmpPeer : tmpPeerlist)
          {
            tmpPeer->sendMessage(pEvent->buffer());
          }
        }
        else //send to all explicit receivers
        {
          const auto tmpEventReceiversCopy = pEvent->receivers();
          for(const int receiverId : tmpEventReceiversCopy)
          {
            VeinTcp::TcpPeer *tmpPeer = m_peerList.value(receiverId,0);
            if(tmpPeer)
            {
              vCDebug(VEIN_NET_TCP_VERBOSE) << "Sending ProtocolEvent" << pEvent << "to receiver:" << tmpPeer;// << pEvent->protobuf()->DebugString().c_str();
              tmpPeer->sendMessage(pEvent->buffer());
            }
          }
        }
      }
    }
    return retVal;
  }
}
