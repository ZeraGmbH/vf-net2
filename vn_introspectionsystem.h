#ifndef VEINNET_INTROSPECTIONSYSTEM_H
#define VEINNET_INTROSPECTIONSYSTEM_H

#include <ve_eventsystem.h>
#include "veinnet_global.h"

#include <QHash>
#include <QJsonObject>

namespace VeinEvent
{
  class StorageSystem;
}


namespace VeinNet
{
  struct EntityIntrospection;
  /**
   * @brief Collates VeinStorage data structure informations into VeinComponent::IntrospectionData for remote introspection
   */
  class VEINNETSHARED_EXPORT IntrospectionSystem : public VeinEvent::EventSystem
  {
    Q_OBJECT

  public:
    explicit IntrospectionSystem(QObject *t_parent=nullptr);
    static constexpr QLatin1String s_nameComponent = QLatin1String("EntityName");

    VeinEvent::StorageSystem *storage() const;

    // EventSystem interface
  public:
    bool processEvent(QEvent *t_event) override;


  private:
    /**
     * @brief Retursn the introspection in JSON format
     * @param t_entityId
     * @return
     */
    QJsonObject getJsonIntrospection(int t_entityId) const;

    QHash<int, EntityIntrospection*> m_introspectionData;
  };
} // namespace VeinNet

#endif // VEINNET_INTROSPECTIONSYSTEM_H
