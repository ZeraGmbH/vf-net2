#include "vn_networkstatusevent.h"

namespace VeinNet
{
  NetworkStatusEvent::NetworkStatusEvent(NetworkStatus t_status, int t_peerId) :
    QEvent(static_cast<QEvent::Type>(getEventType())),
    m_status(t_status),
    m_socketError(),
    m_peerId(t_peerId)
  {
    this->setAccepted(false);
  }

  int NetworkStatusEvent::getEventType()
  {
    if(m_eventType==0)
    {
      m_eventType = QEvent::registerEventType();
      qCDebug(VEIN_NET) << "Registered NetworkStatusEvent as event type:"<<m_eventType;
    }
    return m_eventType;
  }

  NetworkStatusEvent::NetworkStatus NetworkStatusEvent::getStatus() const
  {
    return m_status;
  }

  void NetworkStatusEvent::setError(QAbstractSocket::SocketError t_socketError)
  {
    m_socketError = t_socketError;
  }

  QAbstractSocket::SocketError NetworkStatusEvent::getError() const
  {
    return m_socketError;
  }
  int NetworkStatusEvent::getPeerId() const
  {
    return m_peerId;
  }


  int NetworkStatusEvent::m_eventType=0;
} // namespace VeinNet
