#ifndef VEINNET_GLOBAL_H
#define VEINNET_GLOBAL_H

#include <QtCore/qglobal.h>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(VEIN_NET)
Q_DECLARE_LOGGING_CATEGORY(VEIN_NET_VERBOSE)
Q_DECLARE_LOGGING_CATEGORY(VEIN_NET_TCP)
Q_DECLARE_LOGGING_CATEGORY(VEIN_NET_TCP_VERBOSE)
Q_DECLARE_LOGGING_CATEGORY(VEIN_NET_INTRO)
Q_DECLARE_LOGGING_CATEGORY(VEIN_NET_INTRO_VERBOSE)


#if defined(VEINNET_LIBRARY)
#  define VEINNETSHARED_EXPORT Q_DECL_EXPORT
#else
#  define VEINNETSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // VEINNET_GLOBAL_H
