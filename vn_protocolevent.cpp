#include "vn_protocolevent.h"

namespace VeinNet
{
  ProtocolEvent::ProtocolEvent(EventOrigin t_fromLocalOrigin):
    QEvent(static_cast<QEvent::Type>(getEventType())),
    m_localOrigin(static_cast<bool>(t_fromLocalOrigin))
  {
    this->setAccepted(false);
  }

  int ProtocolEvent::getEventType()
  {
    return s_eventType;
  }

  QByteArray ProtocolEvent::buffer() const
  {
    return m_flatBuffer;
  }

  void ProtocolEvent::setBuffer(QByteArray t_buffer)
  {
    Q_ASSERT(t_buffer.isEmpty() == false);

    m_flatBuffer = t_buffer;
  }

  QList<int> ProtocolEvent::receivers() const
  {
    return m_receivers;
  }

  void ProtocolEvent::setReceivers(const QList<int> &t_receivers)
  {
    m_receivers = t_receivers;
  }

  bool ProtocolEvent::isOfLocalOrigin() const
  {
    return m_localOrigin;
  }

  int ProtocolEvent::peerId() const
  {
    return m_peerId;
  }

  void ProtocolEvent::setPeerId(int t_peerId)
  {
    VF_ASSERT(t_peerId >= 0, "Peer id must be >= 0");
    m_peerId = t_peerId;
  }

  const int ProtocolEvent::s_eventType = QEvent::registerEventType();
}
