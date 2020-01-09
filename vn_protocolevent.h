#ifndef VN_PROTOCOLEVENT_H
#define VN_PROTOCOLEVENT_H

#include "globalIncludes.h"
#include <QEvent>
#include <QUuid>

namespace VeinNet
{
  /**
   * @brief Event containing flatbuffer data that is exchanged between VeinNet::NetworkSystem and the wire implementations (e.g. VeinNet::TcpSystem)
   */
  class VFNET2_EXPORT ProtocolEvent : public QEvent
  {
  public:
    enum class EventOrigin : bool {
      EO_REMOTE = false,
      EO_LOCAL = true
    };

    explicit ProtocolEvent(EventOrigin t_eventOrigin);


    /**
     * @brief On the first call this randomly assigns a QEvent::Type for this class
     * @return
     */
    static int getEventType();


    QByteArray buffer() const;
    void setBuffer(QByteArray t_buffer);

    QList<QUuid> receivers() const;
    void setReceivers(const QList<QUuid> &t_receivers);

    bool isOfLocalOrigin() const;

    QUuid peerId() const;
    void setPeerId(QUuid t_peerId);

  private:
    /**
     * @brief this flag is used to distinguish between local and remote events
     * @note the current policy prohibiting retransmissions of non local events may be to restrictive (for e.g. proxies)
     */
    const bool m_localOrigin;

    /**
     * @brief stored flatbuffer data
     */
    QByteArray m_flatBuffer;

    /**
     * @brief in case of unicast or multicast (in contrast to broadcast) the receivers will be explicitly listed
     */
    QList<QUuid> m_receivers;

    /**
     * @brief 'Randomly' assigned static event type (QEvent::Type)
     */
    static const int s_eventType;

    /**
     * @brief for remote events this is the unique network id
     */
    QUuid m_peerId;
  };
}

#endif // VN_PROTOCOLEVENT_H
