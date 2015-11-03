#include "vn_protocolevent.h"

#include <vfcore.pb.h>

namespace VeinNet
{
  ProtocolEvent::ProtocolEvent(const bool &t_fromLocalOrigin) :
    QEvent(static_cast<QEvent::Type>(getEventType())),
    m_localOrigin(t_fromLocalOrigin)
  {
    this->setAccepted(false);
  }

  ProtocolEvent::~ProtocolEvent()
  {
    delete m_protobuf;
  }

  int ProtocolEvent::getEventType()
  {
    return m_eventType;
  }

  protobuf::VeinProtocol *ProtocolEvent::protobuf() const
  {
    return m_protobuf;
  }

  void ProtocolEvent::setProtobuf(protobuf::VeinProtocol *t_protobuf)
  {
    Q_ASSERT(t_protobuf != 0);

    m_protobuf =t_protobuf;
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

  const int ProtocolEvent::m_eventType = QEvent::registerEventType();
}
