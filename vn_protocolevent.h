#ifndef VN_PROTOCOLEVENT_H
#define VN_PROTOCOLEVENT_H

#include "veinnet_global.h"
#include <QEvent>

namespace protobuf
{
  class VeinProtocol;
}

namespace VeinNet
{
  class VEINNETSHARED_EXPORT ProtocolEvent : public QEvent
  {
    Q_DISABLE_COPY(ProtocolEvent)
  public:
    explicit ProtocolEvent(const bool &t_fromLocalOrigin);

    ~ProtocolEvent();

    /**
     * @brief On the first call this randomly assigns a QEvent::Type for this class
     * @return
     */
    static int getEventType();


    protobuf::VeinProtocol *protobuf() const;
    void setProtobuf(protobuf::VeinProtocol *t_protobuf);

    QList<int> receivers() const;
    void setReceivers(const QList<int> &t_receivers);

    bool isOfLocalOrigin() const;

    int peerId() const;
    void setPeerId(int t_peerId);

  private:
    const bool m_localOrigin;
    /**
     * @brief stored protobuf data
     * @note will be deleted in event destructor
     */
    protobuf::VeinProtocol *m_protobuf = 0;

    /**
     * @brief in case of unicast or multicast (in contrast to broadcast) the receivers will be explicitly listed
     */
    QList<int> m_receivers;

    /**
     * @brief 'Randomly' assigned static event type (QEvent::Type)
     */
    static int m_eventType;

    /**
     * @brief for remote events this is the unique network id
     */
    int m_peerId = -1;
  };
}

#endif // VN_PROTOCOLEVENT_H
