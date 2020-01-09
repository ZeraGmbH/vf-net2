#ifndef VEINNET_NETWORKSTATUSEVENT_H
#define VEINNET_NETWORKSTATUSEVENT_H

#include <QEvent>
#include <QUuid>
#include "globalIncludes.h"

#include <QAbstractSocket>

namespace VeinNet
{
  /**
   * @brief Notifies about network status changes
   * @note at the time the event is received the corresponding peer could already be deleted
   */
  class VFNET2_EXPORT NetworkStatusEvent : public QEvent
  {
  public:
    enum class NetworkStatus : int
    {
      NSE_DISCONNECTED = 0,
      NSE_SOCKET_ERROR = 1
    };



    NetworkStatusEvent(NetworkStatus t_status, QUuid t_peerId);

    /**
     * @brief On the first call this randomly assigns a QEvent::Type for this class
     * @return
     */
    static int getEventType();

    NetworkStatus getStatus() const;

    void setError(QAbstractSocket::SocketError t_socketError);
    QAbstractSocket::SocketError getError() const;

    QUuid getPeerId() const;

  private:
    /**
     * @brief 'Randomly' assigned static event type (QEvent::Type)
     */
    static const int s_eventType;

    /**
     * @brief indicates the current status of the network peer
     */
    const NetworkStatus m_status;

    /**
     * @brief the socket error as Qt identifier in case of NSE_SOCKET_ERROR status
     */
    QAbstractSocket::SocketError m_socketError;

    /**
     * @brief the socket identifier of the network peer
     */
    QUuid m_peerId;
  };

} // namespace VeinNet

#endif // VEINNET_NETWORKSTATUSEVENT_H
