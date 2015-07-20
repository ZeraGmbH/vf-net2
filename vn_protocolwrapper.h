#ifndef VEINPROTOCOLWRAPPER_H
#define VEINPROTOCOLWRAPPER_H

#include "veinnet_global.h"
#include <xiqnetwrapper.h>

namespace VeinNet
{
  class VEINNETSHARED_EXPORT ProtocolWrapper : public XiQNetWrapper
  {
  public:
    ProtocolWrapper();
    virtual ~ProtocolWrapper();

    google::protobuf::Message *byteArrayToProtobuf(QByteArray t_byteArray) override;

    QByteArray protobufToByteArray(google::protobuf::Message *t_protobufMessage) override;

  };
}
#endif // VEINPROTOCOLWRAPPER_H
