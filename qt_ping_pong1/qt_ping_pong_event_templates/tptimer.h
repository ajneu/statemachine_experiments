#ifndef TPTIMER_H
#define TPTIMER_H

#include <QTimer>
#include <QDateTime>


/////////////////////////////////
// TpTimer: TimePointTimer without drift!
////////////////////////////////
class TpTimer : public QTimer {
  Q_OBJECT
 public:
  TpTimer(QObject *parent = nullptr);

  public slots:
    void   start();               /* Stops and restarts the timer: timeout interval given in interval - func setInterval */
    void   start(int msec);       /* Stops and restarts the timer: timeout interval of msec milliseconds */

    qint64 expiryTimePoint() const; /* If timer is running or started:
                                              Returns planned expiry-timepoint
                                              [can also be in the past - see setPassedTimepointsTrigger]).
                                       Else if timer was stopped: 
                                              Returns the last planned expiry-timepoint (e.g. last expiry-timepoint, 
                                              or planned expiry-timepoint of a stopped(!) timer)
                                       Else if timer has never run:
                                              Returns 0
                                    */

    void startToTimePoint(qint64 millisSinceEpoch); /* start timer to timeout at expiry-timepoint relative to Epoch
                                                       (milliseconds since Epoch [1970-01-01T00:00:00.000]) */

    void resumeToTimePoint();   /* starts the timer, to timeout at the expiry-timepoint,
                                   (as given by function expiryTimePoint()) */
    
    void setExpiryTimePoint(qint64 millisSinceEpoch); /* set expiry-timepoint without starting the timer. 
                                                         resumeToTimePoint() will start the timer towards that timepoint */

    static inline qint64 nowTimePoint() {           /* return current milliseconds since Epoch */
      return QDateTime::currentMSecsSinceEpoch();
    }
    
    void setPassedTimepointsTrigger(bool trigger); /* If true, then expiry-timepoints can lie in the past, and if timer is started
                                                      (either with startToTimePoint(millisSinceEpoch) or resumeToTimePoint())
                                                      will cause immediate timeout
                                                   */
    
 private:
    qint64 expireMillisFromEpoch; /* stores planned expiry-timepoint coming up (if timer is running 
                                     [can also be in the past - see passedTimepointsTrigger]).
                                     Else it keeps the last value it had (e.g. planned expiry of stopped timer, or last expiry, or 0)
                                  */

    bool passedTimepointsTrigger;  // if true, then expiresAt() can take timepoints that lie in the past and causes immediate timeout
};


#endif
