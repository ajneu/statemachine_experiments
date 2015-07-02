#include "tptimer.h"
#include <QDateTime>


TpTimer::TpTimer(QObject *parent) : QTimer{parent}, expireMillisFromEpoch{0}, passedTimepointsTrigger{false} {
  connect(this, &QTimer::timeout, [&]() { if (isActive())
        {
          /* singleShot is false */
          expireMillisFromEpoch += interval();
        }});
}

void TpTimer::start() {
  expireMillisFromEpoch = QDateTime::currentMSecsSinceEpoch() + interval();
  QTimer::start();
}

void TpTimer::start(int msec) {
  expireMillisFromEpoch = QDateTime::currentMSecsSinceEpoch() + msec;
  QTimer::start(msec);
}

qint64 TpTimer::expiryTimePoint() const {
  return (expireMillisFromEpoch);
}

void TpTimer::startToTimePoint(qint64 millisSinceEpoch) {
  // set timer to expire at this millis-count-relative-to-epoch
  expireMillisFromEpoch = millisSinceEpoch;
  const qint64 interval = expireMillisFromEpoch - QDateTime::currentMSecsSinceEpoch();
  if (interval >= 0)
    QTimer::start(interval);
  else {
    if (passedTimepointsTrigger) {
      QTimer::start(0);
    }
  }
}

void TpTimer::resumeToTimePoint() {
  startToTimePoint(expireMillisFromEpoch);
}

void TpTimer::setExpiryTimePoint(qint64 millisSinceEpoch) {
  expireMillisFromEpoch = millisSinceEpoch;
}

void TpTimer::setPassedTimepointsTrigger(bool trigger) {
  passedTimepointsTrigger = trigger;
}
