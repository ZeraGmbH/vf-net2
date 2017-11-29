#include "vn_networkstatusevent.h"

namespace VeinNet
{
  NetworkStatusEvent::NetworkStatusEvent(NetworkStatus t_status, QUuid t_peerId) :
    QEvent(static_cast<QEvent::Type>(getEventType())),
    m_status(t_status),
    m_socketError(),
    m_peerId(t_peerId)
  {
    this->setAccepted(false);
  }

  int NetworkStatusEvent::getEventType()
  {
    return s_eventType;
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
  QUuid NetworkStatusEvent::getPeerId() const
  {
    return m_peerId;
  }


  const int NetworkStatusEvent::s_eventType = QEvent::registerEventType();
} // namespace VeinNet
