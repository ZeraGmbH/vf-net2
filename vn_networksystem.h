#ifndef CONNECTIONMANAGER_H
#define CONNECTIONMANAGER_H

#include "globalIncludes.h"

#include <ve_eventsystem.h>

namespace VeinEvent {
  class StorageSystem;
}

/**
 * @brief Namespace for network based transportation of VeinEvent and VeinComponent synchronization
 */
namespace VeinNet
{
  class NetworkSystemPrivate;
  /**
   * @brief Turns all sort of events into ProtocolEvents and sends them depending on the OperationMode
   *
   * Also translates remote ProtocolEvents into regular events and posts them to the EventHandler
   */
  class VFNET2_EXPORT NetworkSystem : public VeinEvent::EventSystem
  {
    Q_OBJECT

  public:
    explicit NetworkSystem(QObject *t_parent=nullptr);
    virtual ~NetworkSystem();

    /**
     * @brief describes whether the events are sent over to no one/subscribers/anyone
     */
    enum OperationMode {
      VNOM_DEBUG =0, /**< do nothing and only print the debug message */
      VNOM_PASS_THROUGH, /**< pass all events to the other site */
      VNOM_SUBSCRIPTION /**< [default] only pass events when the other site subscribed to it */
    };

    OperationMode operationMode() const;
    void setOperationMode(const OperationMode &t_operationMode);

    // EventSystem interface
  public:
    bool processEvent(QEvent *t_event) override;



  private:
    NetworkSystemPrivate *d_ptr = nullptr;
  };
}

#endif // CONNECTIONMANAGER_H
