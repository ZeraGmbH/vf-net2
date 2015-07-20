#include "vn_introspectionsystem.h"

#include <QEvent>

#include <ve_commandevent.h>
#include <ve_storagesystem.h>

#include <vcmp_componentdata.h>
#include <vcmp_entitydata.h>
#include <vcmp_introspectiondata.h>
#include <vcmp_errordata.h>

Q_LOGGING_CATEGORY(VEIN_NET_INTRO, "\e[1;35m<Vein.Network.Introspection>\033[0m")
Q_LOGGING_CATEGORY(VEIN_NET_INTRO_VERBOSE, "\e[0;35m<Vein.Network.Introspection>\033[0m")

using namespace VeinEvent;
using namespace VeinComponent;

namespace VeinNet
{
  IntrospectionSystem::IntrospectionSystem(QObject *t_parent) :
    VeinEvent::EventSystem(t_parent)
  {

  }

  StorageSystem *IntrospectionSystem::storage() const
  {
    return m_storage;
  }

  void IntrospectionSystem::setStorage(StorageSystem *t_storage)
  {
    if(t_storage)
    {
      m_storage = t_storage;
    }
  }

  bool IntrospectionSystem::processEvent(QEvent *t_event)
  {
    bool retVal = false;

    if(t_event->type()==CommandEvent::eventType())
    {
      CommandEvent *cEvent = 0;
      cEvent = static_cast<CommandEvent *>(t_event);

      if(cEvent != 0 && cEvent->eventSubtype() == CommandEvent::EventSubtype::NOTIFICATION)
      {
        if (cEvent->eventData()->type() == EntityData::dataType())
        {
          EntityData *eData=0;
          eData = static_cast<EntityData *>(cEvent->eventData());

          if(eData->eventCommand() == VeinComponent::EntityData::ECMD_SUBSCRIBE)
          {
            qCDebug(VEIN_NET_INTRO_VERBOSE) << "Processing command event:" << cEvent;
            IntrospectionData *newData=0;
            QJsonObject tmpObject;

            tmpObject = getJsonIntrospection(eData->entityId());
            if(tmpObject.isEmpty() == false)
            {
              newData = new IntrospectionData();
              newData->setEntityId(eData->entityId());
              newData->setJsonData(tmpObject);
              newData->setEventOrigin(IntrospectionData::EventOrigin::EO_LOCAL);
              newData->setEventTarget(IntrospectionData::EventTarget::ET_ALL);

              CommandEvent *newEvent = new CommandEvent(CommandEvent::EventSubtype::NOTIFICATION, newData);
              /// @note sets the peer id to be the sender peer id, used for unicasting the message
              newEvent->setPeerId(cEvent->peerId());

              qCDebug(VEIN_NET_INTRO) << "Sending introspection event:" << newEvent;

              emit sigSendEvent(newEvent);

              retVal = true;
            }
            else
            {
              QString tmpErrorString = tr("No introspection available for requested entity, entity id: %1").arg(eData->entityId());
              t_event->accept();
              qCWarning(VEIN_NET_INTRO) << tmpErrorString;


              ErrorData *errData = new ErrorData();

              errData->setEntityId(eData->entityId());
              errData->setOriginalData(eData);
              errData->setEventOrigin(EventData::EventOrigin::EO_LOCAL);
              errData->setEventTarget(eData->eventTarget());
              errData->setErrorDescription(tmpErrorString);

              CommandEvent *tmpCommandEvent = new CommandEvent(CommandEvent::EventSubtype::NOTIFICATION, errData);
              tmpCommandEvent->setPeerId(cEvent->peerId());
              emit sigSendEvent(tmpCommandEvent);
            }
          }
        }
      }
    }

    return retVal;
  }

  QJsonObject IntrospectionSystem::getJsonIntrospection(int t_entityId) const
  {
    QJsonObject retVal;
    if(m_storage && m_storage->hasEntity(t_entityId))
    {
      const QHash<QString, QVariant> *tmpData = m_storage->getEntityDataCopy(t_entityId);

      foreach(QString tmpKey, tmpData->keys())
      {
        retVal.insert(tmpKey, QJsonValue::fromVariant(tmpData->value(tmpKey)));
      }
    }
    return retVal;
  }

} // namespace VeinNet

