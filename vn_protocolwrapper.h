#ifndef VEINPROTOCOLWRAPPER_H
#define VEINPROTOCOLWRAPPER_H

#include "veinnet_global.h"
#include <xiqnetwrapper.h>

/**
 * @brief namespace for Custom google::protobuf based protocols
 */
namespace protobuf {} // only used for doxygen documentation

/**
 * @brief Google namespace
 */
namespace google {
  /**
   * @brief Google protocol buffer namespace https://developers.google.com/protocol-buffers/
   */
  namespace protobuf {} // only used for doxygen documentation
}

namespace VeinNet
{
  /**
   * @brief A Wrapper implementation for protobuf::VeinProtocol
   */
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
