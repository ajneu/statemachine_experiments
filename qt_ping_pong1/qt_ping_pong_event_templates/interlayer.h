#ifndef INTERLAYER_H
#define INTERLAYER_H

#include <QObject>
#include "userevents.h"

class Interlayer : public QObject {
  Q_OBJECT
 public:
  Interlayer(QObject *parent = nullptr) : QObject{parent} {}
  signals:
  void signalEventX(const UserEventX &);
  void signalEventO(const UserEventO &);
  void signalEventI(const UserEventI &);
  void signalEventT(const UserEventT &);
};

extern Interlayer interlayer;

#endif
