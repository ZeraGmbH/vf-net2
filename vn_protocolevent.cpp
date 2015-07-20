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
    if(m_eventType==0)
    {
      m_eventType = QEvent::registerEventType();
      qCDebug(VEIN_NET) << "Registered ProtocolEvent as event type:"<<m_eventType;
    }
    return m_eventType;
  }

  protobuf::VeinProtocol *ProtocolEvent::protobuf() const
  {
    return m_protobuf;
  }

  void ProtocolEvent::setProtobuf(protobuf::VeinProtocol *t_protobuf)
  {
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
    if(t_peerId>=0)
    {
      m_peerId = t_peerId;
    }
  }
  
  int ProtocolEvent::m_eventType = 0;
}
