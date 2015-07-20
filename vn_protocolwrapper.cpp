#include "vn_protocolwrapper.h"

#include <veincore.pb.h>


namespace VeinNet
{

  ProtocolWrapper::ProtocolWrapper()
  {
  }

  ProtocolWrapper::~ProtocolWrapper()
  {
  }

  google::protobuf::Message *ProtocolWrapper::byteArrayToProtobuf(QByteArray t_byteArray)
  {
    protobuf::VeinProtocol *proto = new protobuf::VeinProtocol();
    if(!proto->ParseFromArray(t_byteArray, t_byteArray.size()))
    {
      qCWarning(VEIN_NET) <<"(ProtocolWrapper) Error parsing protobuf";
      Q_ASSERT(false);
    }
    return proto;
  }

  QByteArray ProtocolWrapper::protobufToByteArray(google::protobuf::Message *t_protobufMessage)
  {
    QByteArray retVal;
    retVal.resize(t_protobufMessage->ByteSize());
    if(t_protobufMessage->SerializeToArray(retVal.data(), retVal.size()))
    {
      return retVal;
    }
    else
    {
      qCWarning(VEIN_NET) <<"(ProtocolWrapper) Error serializing protobuf";
      return QByteArray();
    }
  }
}
