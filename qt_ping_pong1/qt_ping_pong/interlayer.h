#ifndef INTERLAYER_H
#define INTERLAYER_H

#include <QObject>
#include "userevents.h"

class Interlayer : public QObject {
  Q_OBJECT
 public:
  Interlayer(QObject *parent = nullptr) : QObject{parent} {}
  signals:
  void signalEvent(const UserEvent &);
  void signalDataEvent(const UserDEventTimeout &);
};

extern Interlayer interlayer;

#endif
