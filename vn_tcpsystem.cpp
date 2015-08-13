#include "vn_tcpsystem.h"

#include "vn_protocolevent.h"
#include "vn_protocolwrapper.h"
#include "vn_networkstatusevent.h"

#include <xiqnetpeer.h>
#include <xiqnetserver.h>
#include <vfcore.pb.h>

Q_LOGGING_CATEGORY(VEIN_NET_TCP, "\e[1;33m<Vein.Network.Tcp>\033[0m")
Q_LOGGING_CATEGORY(VEIN_NET_TCP_VERBOSE, "\e[0;33m<Vein.Network.Tcp>\033[0m")

using namespace VeinEvent;

namespace VeinNet
{
  TcpSystem::TcpSystem(QObject *t_parent) :
    EventSystem(t_parent) ,
    m_server(new XiQNetServer(this)),
    m_protoWrapper(new ProtocolWrapper())
  {
    vCDebug(VEIN_NET_TCP)  << "Created TCP system";
    connect(m_server, &XiQNetServer::sigClientConnected, this, &TcpSystem::onClientConnected);
  }

  TcpSystem::~TcpSystem()
  {
    vCDebug(VEIN_NET_TCP)  << "Destroyed TCP system";
    delete m_protoWrapper;
  }

  void TcpSystem::startServer(quint16 t_port)
  {
    if(!m_server->isListening())
    {
      m_server->startServer(t_port);
    }
  }

  void TcpSystem::connectToServer(QString t_host, quint16 t_port)
  {
    XiQNetPeer *tmpPeer = new XiQNetPeer(this);
    tmpPeer->setWrapper(m_protoWrapper);
    connect(tmpPeer, SIGNAL(sigSocketError(QAbstractSocket::SocketError)), this, SLOT(onSocketError(QAbstractSocket::SocketError)));
    connect(tmpPeer, &XiQNetPeer::sigConnectionEstablished, this, &TcpSystem::onConnectionEstablished);
    connect(tmpPeer, &XiQNetPeer::sigConnectionClosed, this, &TcpSystem::onConnectionClosed);
    tmpPeer->startConnection(t_host, t_port);
  }

  void TcpSystem::onClientConnected(XiQNetPeer *t_networkPeer)
  {
    connect(t_networkPeer, &XiQNetPeer::sigMessageReceived, this, &TcpSystem::onMessageReceived);
    t_networkPeer->setWrapper(m_protoWrapper);
    m_waitingAuth.append(t_networkPeer);

    connect(t_networkPeer, &XiQNetPeer::sigConnectionClosed, this, &TcpSystem::onClientDisconnected);

    /** @todo implement authentication */
    protobuf::VeinProtocol *protoAuth = new protobuf::VeinProtocol();
    t_networkPeer->sendMessage(protoAuth);
    delete protoAuth;
  }

  void TcpSystem::onConnectionEstablished()
  {
    /// @todo add the caller as parameter and trash this useless cast
    XiQNetPeer *tmpPeer = qobject_cast<XiQNetPeer *>(QObject::sender());
    if(tmpPeer)
    {
      m_waitingAuth.append(tmpPeer);

      connect(tmpPeer, &XiQNetPeer::sigMessageReceived, this, &TcpSystem::onMessageReceived);
      tmpPeer->setWrapper(m_protoWrapper);

      /** @todo implement authentication */
      protobuf::VeinProtocol *protoAuth = new protobuf::VeinProtocol();
      tmpPeer->sendMessage(protoAuth);
      delete protoAuth;
    }
  }

  void TcpSystem::onConnectionClosed()
  {
    XiQNetPeer *tmpPPeer = qobject_cast<XiQNetPeer *>(QObject::sender());

    int tmpPeerId = tmpPPeer->getPeerId();

    vCDebug(VEIN_NET_TCP) << "Disconnected from server with ID:" << tmpPeerId;
    m_waitingAuth.removeAll(tmpPPeer);
    m_peerList.remove(tmpPeerId);
  }



  void TcpSystem::onClientDisconnected()
  {
    XiQNetPeer * tmpPPeer = qobject_cast<XiQNetPeer *>(QObject::sender());
    int tmpPeerId = tmpPPeer->getPeerId();

    NetworkStatusEvent *sEvent = new NetworkStatusEvent(NetworkStatusEvent::NetworkStatus::NSE_DISCONNECTED, tmpPeerId);
    emit sigSendEvent(sEvent);

    vCDebug(VEIN_NET_TCP) << "Client disconnected with ID:" << tmpPeerId << "sent NetworkStatusEvent:" << sEvent;
    m_waitingAuth.removeAll(tmpPPeer);
    m_peerList.remove(tmpPeerId);
    emit sigClientDisconnected(tmpPeerId);
    delete tmpPPeer;
  }

  void TcpSystem::onMessageReceived(google::protobuf::Message *t_protobufMessage)
  {
    XiQNetPeer *tmpPPeer = qobject_cast<XiQNetPeer *>(QObject::sender());
    if(tmpPPeer != 0 && t_protobufMessage != 0)
    {
      protobuf::VeinProtocol *proto = 0;
      proto = static_cast<protobuf::VeinProtocol *>(t_protobufMessage);
      if(proto)
      {
        //vCDebug(VEIN_NET_TCP)  << "Message received:" << proto->DebugString().c_str();
        if(m_waitingAuth.contains(tmpPPeer))
        {
          int newId = m_peerList.append(tmpPPeer);
          vCDebug(VEIN_NET_TCP) << "New connection with id:" << newId;
          tmpPPeer->setPeerId(newId);

          m_waitingAuth.removeAll(tmpPPeer);
          emit sigConnnectionEstablished(newId);
        }
        ProtocolEvent *tmpEvent = new ProtocolEvent(false);
        tmpEvent->setProtobuf(proto);
        tmpEvent->setPeerId(tmpPPeer->getPeerId());
        if(proto->command_size()>0)
        {
          vCDebug(VEIN_NET_TCP_VERBOSE) << "Received protocol event of type:" << proto->command(0).datatype();
        }
        emit sigSendEvent(tmpEvent);
      }
    }
  }

  void TcpSystem::onSocketError(QAbstractSocket::SocketError t_socketError)
  {
    XiQNetPeer *tmpPPeer = qobject_cast<XiQNetPeer *>(QObject::sender());
    if(tmpPPeer)
    {
      NetworkStatusEvent *sEvent = new NetworkStatusEvent(NetworkStatusEvent::NetworkStatus::NSE_SOCKET_ERROR, tmpPPeer->getPeerId());
      sEvent->setError(t_socketError);
      qCCritical(VEIN_NET_TCP) << "Connection error on network with id:" << tmpPPeer->getPeerId() << "error:" << tmpPPeer->getErrorString() << "sent NetworkStatusEvent:" << sEvent;
      emit sigSendEvent(sEvent);
    }
  }

  bool TcpSystem::processEvent(QEvent *t_event)
  {
    bool retVal = false;
    ProtocolEvent *pEvent=0;
    if(t_event->type()==ProtocolEvent::getEventType())
    {
      pEvent = static_cast<ProtocolEvent *>(t_event);

      //do not process protocol events from foreign systems, that is the job of NetworkSystem
      if(pEvent && pEvent->isOfLocalOrigin() == true)
      {
        //send to all
        if(pEvent->receivers().isEmpty())
        {
          QList<XiQNetPeer *> tmpPeerlist = m_peerList.values();
          vCDebug(VEIN_NET_TCP_VERBOSE) << "Sending ProtocolEvent" << pEvent << "to receivers:" << tmpPeerlist;// << pEvent->protobuf()->DebugString().c_str();
          foreach(XiQNetPeer *tmpPeer, tmpPeerlist)
          {
            tmpPeer->sendMessage(pEvent->protobuf());
          }
        }
        else //send to all explicit receivers
        {
          foreach (int receiverId, pEvent->receivers())
          {
            XiQNetPeer *tmpPeer = m_peerList.value(receiverId,0);
            if(tmpPeer)
            {
              vCDebug(VEIN_NET_TCP_VERBOSE) << "Sending ProtocolEvent" << pEvent << "to receiver:" << tmpPeer;// << pEvent->protobuf()->DebugString().c_str();
              tmpPeer->sendMessage(pEvent->protobuf());
            }
          }
        }
      }
    }
    return retVal;
  }
}
