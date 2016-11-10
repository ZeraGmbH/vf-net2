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
  /**
   * @brief Event containing protobuf::VeinProtocol data that is exchanged between VeinNet::NetworkSystem and the wire implementations (e.g. VeinNet::TcpSystem)
   */
  class VEINNETSHARED_EXPORT ProtocolEvent : public QEvent
  {
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
    /**
     * @brief this flag is used to distinguish between local and remote events
     * @note the current policy prohibiting retransmissions of non local events may be to restrictive (for e.g. proxies)
     */
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
    static const int s_eventType;

    /**
     * @brief for remote events this is the unique network id
     */
    int m_peerId = -1;
  };
}

#endif // VN_PROTOCOLEVENT_H
