#ifndef VEINNET_NETWORKSTATUSEVENT_H
#define VEINNET_NETWORKSTATUSEVENT_H

#include <QEvent>
#include "veinnet_global.h"

#include <QAbstractSocket>

namespace VeinNet
{
  /**
   * @brief Notifies about status changes
   * @note at the time the event is received the corresponding peer could already be deleted
   */
  class VEINNETSHARED_EXPORT NetworkStatusEvent : public QEvent
  {
  public:
    enum class NetworkStatus : int
    {
      NSE_DISCONNECTED = 0,
      NSE_SOCKET_ERROR = 1
    };



    NetworkStatusEvent(NetworkStatus t_status, int t_peerId);

    /**
     * @brief On the first call this randomly assigns a QEvent::Type for this class
     * @return
     */
    static int getEventType();

    NetworkStatus getStatus() const;

    void setError(QAbstractSocket::SocketError t_socketError);
    QAbstractSocket::SocketError getError() const;

    int getPeerId() const;

  private:
    static int m_eventType;

    const NetworkStatus m_status;
    QAbstractSocket::SocketError m_socketError;
    int m_peerId = -1;
  };

} // namespace VeinNet

#endif // VEINNET_NETWORKSTATUSEVENT_H
