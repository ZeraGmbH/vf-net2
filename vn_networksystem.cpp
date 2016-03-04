#include "vn_networksystem.h"

#include "vn_protocolevent.h"
#include "vn_networkstatusevent.h"
#include <vcmp_componentdata.h>
#include <vcmp_entitydata.h>
#include <vcmp_errordata.h>
#include <vcmp_introspectiondata.h>
#include <ve_commandevent.h>
#include <vfcore.pb.h>

Q_LOGGING_CATEGORY(VEIN_NET, "\e[1;32m<Vein.Network>\033[0m")
Q_LOGGING_CATEGORY(VEIN_NET_VERBOSE, "\e[0;32m<Vein.Network>\033[0m")

using namespace VeinEvent;
using namespace VeinComponent;

namespace VeinNet
{
  /**
   * @brief PIMPL for VeinNet::NetworkSystem
   */
  class NetworkSystemPrivate
  {

    // stands for QHash<"entity descriptor", QList<"network id"> *>
    template <typename T>
    using SubscriptionStorage = QHash<T, QList<int>>;


    explicit NetworkSystemPrivate(NetworkSystem *t_qPtr) :
      q_ptr(t_qPtr)
    {

    }

    void processProtoEvent(ProtocolEvent *t_pEvent)
    {
      Q_ASSERT(t_pEvent != 0);
      //do not process messages from this instance
      if(t_pEvent->isOfLocalOrigin() == false)
      {
        protobuf::VeinProtocol *protoEnvelope = t_pEvent->protobuf();
        const int commandSize = protoEnvelope->command_size();
        for(int i=0; i<commandSize; ++i)
        {
          const protobuf::Vein_Command protoCmd = protoEnvelope->command(i);

          VeinEvent::EventData *evData = 0;
          CommandEvent *tmpEvent = 0;

          switch(protoCmd.datatype())
          {
            case VeinComponent::EntityData::dataType():
            {
              VeinComponent::EntityData * tmpData = new VeinComponent::EntityData();
              tmpData->deserialize(QByteArray(protoCmd.payload().data(), protoCmd.payload().size()));

              evData = tmpData;
              break;
            }
            case VeinComponent::ErrorData::dataType():
            {
              VeinComponent::ErrorData *tmpData = new VeinComponent::ErrorData();
              tmpData->deserialize(QByteArray(protoCmd.payload().data(), protoCmd.payload().size()));

              evData = tmpData;
              break;
            }
            case VeinComponent::ComponentData::dataType():
            {
              VeinComponent::ComponentData * tmpData = new VeinComponent::ComponentData();
              tmpData->deserialize(QByteArray(protoCmd.payload().data(), protoCmd.payload().size()));

              evData = tmpData;
              break;
            }
            case VeinComponent::IntrospectionData::dataType():
            {
              VeinComponent::IntrospectionData *tmpData = new VeinComponent::IntrospectionData();
              tmpData->deserialize(QByteArray(protoCmd.payload().data(), protoCmd.payload().size()));

              evData = tmpData;
              break;
            }
          }
          VF_ASSERT(evData != 0, "Unhandled event datatype");

          if(evData->isValid())
          {
            evData->setEventOrigin(VeinEvent::EventData::EventOrigin::EO_FOREIGN);
            switch(protoCmd.type())
            {
              case protobuf::Vein_Command_CommandType_VC_NOTIFICATION:
              {
                tmpEvent = new CommandEvent(CommandEvent::EventSubtype::NOTIFICATION, evData);
                break;
              }
              case protobuf::Vein_Command_CommandType_VC_TRANSACTION:
              {
                tmpEvent = new CommandEvent(CommandEvent::EventSubtype::TRANSACTION, evData);
                break;
              }
            }
            Q_ASSERT(tmpEvent != 0);

            tmpEvent->setPeerId(t_pEvent->peerId());
            vCDebug(VEIN_NET_VERBOSE) << "Processing ProtocolEvent:" << t_pEvent << "new event:" << tmpEvent;
            emit q_ptr->sigSendEvent(tmpEvent);
          }
          else
          {
            qCWarning(VEIN_NET) << "Received invalid event from protobuf:" << protoCmd.DebugString().c_str();
          }
        }
      }
    }


    bool handleSubscription(VeinComponent::EntityData *t_eData, int t_peerId)
    {
      Q_ASSERT(t_eData != 0);

      bool retVal = false;
      switch(t_eData->eventCommand())
      {
        case VeinComponent::EntityData::ECMD_SUBSCRIBE:
        {
          QList<int> tmpCurrentSubscriptions = m_subscriptions.value(t_eData->entityId());
          if(tmpCurrentSubscriptions.contains(t_peerId) == false)
          {
            tmpCurrentSubscriptions.append(t_peerId);
          }
          m_subscriptions.insert(t_eData->entityId(), tmpCurrentSubscriptions);

          vCDebug(VEIN_NET) << "Added subscription for entity:" << t_eData->entityId() << "network peer:" << t_peerId;
          retVal = true;
          break;
        }
        case VeinComponent::EntityData::ECMD_UNSUBSCRIBE:
        {
          QList<int> tmpCurrentSubscriptions = m_subscriptions.value(t_eData->entityId());
          tmpCurrentSubscriptions.removeAll(t_peerId);
          m_subscriptions.insert(t_eData->entityId(), tmpCurrentSubscriptions);
          vCDebug(VEIN_NET) << "Removed subscription for entity:" << t_eData->entityId() << "network peer:" << t_peerId;
          retVal = true;
          break;
        }
        default:
          break;
      }
      return retVal;
    }

    void handleNetworkStatusEvent(NetworkStatusEvent *t_sEvent)
    {
      vCDebug(VEIN_NET_VERBOSE) << "processing NetworkStatusEvent:" << t_sEvent;
      if(t_sEvent->getStatus() == NetworkStatusEvent::NetworkStatus::NSE_DISCONNECTED)
      {
        int tmpPeerId = t_sEvent->getPeerId();
        foreach(int tmpKey, m_subscriptions.keys())
        {
          QList<int> tmpSubscribers = m_subscriptions.value(tmpKey);
          if(tmpSubscribers.contains(tmpPeerId))
          {
            tmpSubscribers.removeAll(tmpPeerId);
            m_subscriptions.insert(tmpKey, tmpSubscribers);
            vCDebug(VEIN_NET) << "Removed subscription for entity:" << tmpKey << "for disconnected network peer:" << tmpPeerId;
          }
        }
      }
      else if(t_sEvent->getStatus() == NetworkStatusEvent::NetworkStatus::NSE_SOCKET_ERROR)
      {
        switch(t_sEvent->getError())
        {
          case QAbstractSocket::RemoteHostClosedError:
            break; /// @todo reconnect here?
          default:
            break;
        }
      }
    }

    protobuf::VeinProtocol *prepareEnvelope(VeinEvent::CommandEvent *t_cEvent)
    {
      Q_ASSERT(t_cEvent != 0);

      protobuf::VeinProtocol *retVal = 0;
      protobuf::Vein_Command *protoCmd = 0;
      VeinEvent::EventData * evData = 0;
      QByteArray tmpData;

      retVal = new protobuf::VeinProtocol();
      protoCmd = retVal->add_command();
      switch(t_cEvent->eventSubtype())
      {
        case CommandEvent::EventSubtype::NOTIFICATION:
        {
          protoCmd->set_type(protobuf::Vein_Command_CommandType_VC_NOTIFICATION);
          break;
        }
        case CommandEvent::EventSubtype::TRANSACTION:
        {
          protoCmd->set_type(protobuf::Vein_Command_CommandType_VC_TRANSACTION);
          break;
        }
      }

      evData = t_cEvent->eventData();
      Q_ASSERT(evData != 0);

      protoCmd->set_datatype(evData->type());

      tmpData = evData->serialize();

      protoCmd->set_payload(tmpData.data(), tmpData.size());
      return retVal;
    }

    void sendNetworkEvent(QList<int> t_receivers, protobuf::VeinProtocol *t_data)
    {
      Q_ASSERT(t_data != 0);

      ProtocolEvent *protoEvent = new ProtocolEvent(true); //create a new event of local origin
      protoEvent->setProtobuf(t_data);
      protoEvent->setReceivers(t_receivers);

      emit q_ptr->sigSendEvent(protoEvent);
    }

    NetworkSystem::OperationMode m_operationMode=NetworkSystem::VNOM_SUBSCRIPTION;

    /**
     * @brief stores current subscribers
     */
    SubscriptionStorage<int> m_subscriptions;

    NetworkSystem *q_ptr;


    friend class NetworkSystem;
  };



  NetworkSystem::NetworkSystem(QObject * t_parent) :
    EventSystem(t_parent),
    d_ptr(new NetworkSystemPrivate(this))
  {
    vCDebug(VEIN_NET) << "Initialized network system";
  }

  NetworkSystem::~NetworkSystem()
  {
    vCDebug(VEIN_NET) << "Deinitialized network system";
    delete d_ptr;
  }

  NetworkSystem::OperationMode NetworkSystem::operationMode()
  {
    return d_ptr->m_operationMode;
  }

  void NetworkSystem::setOperationMode(const NetworkSystem::OperationMode &t_operationMode)
  {
    d_ptr->m_operationMode = t_operationMode;
  }

  bool NetworkSystem::processEvent(QEvent *t_event)
  {
    Q_ASSERT(t_event != 0);
    bool retVal = false;
    VeinEvent::EventData *evData = 0;

    if(t_event->type() == ProtocolEvent::getEventType())
    {
      ProtocolEvent *pEvent=0;
      pEvent = static_cast<ProtocolEvent *>(t_event);
      Q_ASSERT(pEvent != 0);

      //      if(pEvent->eventOrigin() == ProtocolEvent::CO_FOREIGN ) //< this is checked differently in processProtoEvent
      //      {
      retVal = true;
      d_ptr->processProtoEvent(pEvent);
      //      }
    }
    else if(t_event->type() == CommandEvent::eventType())
    {
      switch (d_ptr->m_operationMode)
      {
        case VeinNet::NetworkSystem::VNOM_DEBUG:
        {
          vCDebug(VEIN_NET_VERBOSE) << "Debug mode is enabled, dropped event:" << t_event;
          t_event->accept();
          retVal = true;
          break;
        }
        case VeinNet::NetworkSystem::VNOM_PASS_THROUGH:
        {
          VeinEvent::CommandEvent *cEvent = 0;

          cEvent = static_cast<VeinEvent::CommandEvent *>(t_event);
          Q_ASSERT(cEvent != 0);

          evData = cEvent->eventData();
          Q_ASSERT(evData != 0);

          if(evData->eventOrigin() == VeinEvent::EventData::EventOrigin::EO_LOCAL
             && evData->eventTarget() == VeinEvent::EventData::EventTarget::ET_ALL)
          {
            protobuf::VeinProtocol *protoEnvelope = d_ptr->prepareEnvelope(cEvent);
            QList<int> protoReceivers;

            if(cEvent->peerId() >= 0)
            {
              protoReceivers = QList<int>() << cEvent->peerId();
            }

            d_ptr->sendNetworkEvent(protoReceivers, protoEnvelope);


            retVal = true;
          }
          break;
        }
        case VeinNet::NetworkSystem::VNOM_SUBSCRIPTION:
        {
          // check if the event is a notification event with entity command subscribe/unsubscribe
          //   drop the event and add/remove the sender to/from the subscriber list
          // or else
          //   send the event to all active subscribers
          VeinEvent::CommandEvent *cEvent = 0;
          cEvent = static_cast<VeinEvent::CommandEvent *>(t_event);
          Q_ASSERT(cEvent != 0);

          evData = cEvent->eventData();
          Q_ASSERT(evData != 0);

          if(evData->eventOrigin() == VeinEvent::EventData::EventOrigin::EO_LOCAL
             && evData->eventTarget() == VeinEvent::EventData::EventTarget::ET_ALL)
          {
            bool isDiscarded = false;
            if(cEvent->eventSubtype() == CommandEvent::EventSubtype::NOTIFICATION && evData->type() == VeinComponent::EntityData::dataType())
            {
              isDiscarded = d_ptr->handleSubscription(static_cast<VeinComponent::EntityData *>(evData), cEvent->peerId());
            }

            if(isDiscarded) //the current event is addressed to this system so do not send it over the network
            {
              cEvent->setAccepted(true);
              retVal = true;
            }
            else if(d_ptr->m_subscriptions.contains(evData->entityId()))
            {
              QList<int> protoReceivers=d_ptr->m_subscriptions.value(evData->entityId());

              if(protoReceivers.isEmpty() == false)
              {
                protobuf::VeinProtocol *protoEnvelope = d_ptr->prepareEnvelope(cEvent);
                vCDebug(VEIN_NET_VERBOSE) << "Processing command event:" << cEvent << "type:" << static_cast<qint8>(cEvent->eventSubtype());// << "new event:" << protoEvent;
                d_ptr->sendNetworkEvent(protoReceivers, protoEnvelope);
                retVal = true;
              }
            }
          }
          break;
        }
      }
    }
    else if(t_event->type() == NetworkStatusEvent::getEventType())
    {
      NetworkStatusEvent *sEvent = 0;
      sEvent=static_cast<NetworkStatusEvent *>(t_event);
      Q_ASSERT(sEvent != 0);

      retVal = true;
      d_ptr->handleNetworkStatusEvent(sEvent);
    }
    return retVal;
  }
}
