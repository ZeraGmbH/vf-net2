#include "vn_introspectionsystem.h"

#include <QEvent>
#include <QJsonArray>

#include <ve_commandevent.h>

#include <vcmp_componentdata.h>
#include <vcmp_entitydata.h>
#include <vcmp_introspectiondata.h>
#include <vcmp_remoteproceduredata.h>
#include <vcmp_errordata.h>

Q_LOGGING_CATEGORY(VEIN_NET_INTRO, VEIN_DEBUGNAME_NET_INTRO)
Q_LOGGING_CATEGORY(VEIN_NET_INTRO_VERBOSE, VEIN_DEBUGNAME_NET_INTRO_VERBOSE)

using namespace VeinEvent;
using namespace VeinComponent;

namespace VeinNet
{
  struct EntityIntrospection
  {
  public:
    QSet<QString> m_components;
    QSet<QString> m_procedures;
  };

  IntrospectionSystem::IntrospectionSystem(QObject *t_parent) :
    VeinEvent::EventSystem(t_parent)
  {
    const auto listToClean = m_introspectionData.values();
    for(EntityIntrospection *toDelete : qAsConst(listToClean))
    {
      delete toDelete;
    }
    m_introspectionData.clear();
  }

  const QString IntrospectionSystem::s_nameComponent = QLatin1String("EntityName");

  bool IntrospectionSystem::processEvent(QEvent *t_event)
  {
    Q_ASSERT(t_event != nullptr);
    bool retVal = false;

    if(t_event->type()==CommandEvent::eventType())
    {
      CommandEvent *cEvent = nullptr;
      EventData *evData = nullptr;
      cEvent = static_cast<CommandEvent *>(t_event);
      Q_ASSERT(cEvent != nullptr);

      evData = cEvent->eventData();
      Q_ASSERT(evData != nullptr);

      if(cEvent->eventSubtype() == CommandEvent::EventSubtype::NOTIFICATION)
      {
        switch(evData->type())
        {
          case EntityData::dataType():
          {
            EntityData *eData=nullptr;
            eData = static_cast<EntityData *>(evData);
            Q_ASSERT(eData != nullptr);

            switch(eData->eventCommand())
            {
              case EntityData::Command::ECMD_ADD:
              {
                if(eData->eventOrigin() == VeinEvent::EventData::EventOrigin::EO_LOCAL)
                {
                  if(m_introspectionData.contains(eData->entityId()))
                  {
                    //remove the old entry to prevent leaking
                    delete m_introspectionData.value(eData->entityId());
                  }
                  m_introspectionData.insert(eData->entityId(), new EntityIntrospection());
                }
                break;
              }
              case EntityData::Command::ECMD_SUBSCRIBE:
              {
                vCDebug(VEIN_NET_INTRO_VERBOSE) << "Processing command event:" << cEvent << "with command ECMD_SUBSCRIBE, entityId:" << eData->entityId();
                IntrospectionData *newData=nullptr;
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

                  vCDebug(VEIN_NET_INTRO_VERBOSE) << "Sending introspection event:" << newEvent;

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
                break;
              }
              default:
                break;
            }
            break;
          }
          case ComponentData::dataType():
          {
            ComponentData *cData = nullptr;
            cData = static_cast<ComponentData *>(evData);
            Q_ASSERT(cData != nullptr);
            if(cData->eventOrigin() == VeinEvent::EventData::EventOrigin::EO_LOCAL)
            {
              switch(cData->eventCommand())
              {
                case ComponentData::Command::CCMD_ADD:
                {
                  Q_ASSERT(m_introspectionData.contains(cData->entityId()));
                  m_introspectionData.value(cData->entityId())->m_components.insert(cData->componentName());
                  break;
                }
                case ComponentData::Command::CCMD_REMOVE:
                {
                  Q_ASSERT(m_introspectionData.contains(cData->entityId()));
                  m_introspectionData.value(cData->entityId())->m_components.remove(cData->componentName());
                  break;
                }
                default:
                  break;
              }
            }
            break;
          }
          case RemoteProcedureData::dataType():
          {
            RemoteProcedureData *rpcData = nullptr;
            rpcData = static_cast<RemoteProcedureData *>(evData);
            Q_ASSERT(rpcData != nullptr);
            if(rpcData->eventOrigin() == VeinEvent::EventData::EventOrigin::EO_LOCAL)
            {
              if(rpcData->command() == RemoteProcedureData::Command::RPCMD_REGISTER)
              {
                Q_ASSERT(m_introspectionData.contains(rpcData->entityId()));
                m_introspectionData.value(rpcData->entityId())->m_procedures.insert(rpcData->procedureName());
              }
            }
          }
          default:
            break;
        }
      }
    }

    return retVal;
  }

  QJsonObject IntrospectionSystem::getJsonIntrospection(int t_entityId) const
  {
    QJsonObject retVal;

    if(m_introspectionData.contains(t_entityId))
    {
      retVal.insert(QString("components"), QJsonArray::fromStringList(m_introspectionData.value(t_entityId)->m_components.values()));
      retVal.insert(QString("procedures"), QJsonArray::fromStringList(m_introspectionData.value(t_entityId)->m_procedures.values()));
    }
    return retVal;
  }

} // namespace VeinNet
