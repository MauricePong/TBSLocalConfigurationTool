#ifndef THREADTBS_H
#define THREADTBS_H
#include <QThread>
#include "tbsfunc.h"
extern volatile bool swflg;
extern volatile u8 mode;
// extern const char *i_path;
extern QString i_path;
extern volatile u8 workMode;
extern volatile u8 manNo;
// ui vaule
extern u8 PNo;
extern volatile int Vpid;
extern volatile int Apid;
extern volatile int Pmt;
extern volatile int Pcr;
extern volatile int HpFile;
extern volatile int HpLevel;
extern volatile int PixW;
extern volatile int PixH;
extern volatile int BitRate;
extern volatile int Frame;
extern volatile int MuteEn;
extern u8 Pname[24];
extern u8 audio_parameter[2];
extern volatile float igvr;
extern u8 epkt;
class threadTbs : public QThread {
  Q_OBJECT
 public:
  explicit threadTbs();
  ~threadTbs();
  void fileSizeLimit();
  void writeBuffer();
  void autoMode();
  void manualMoe();
  void advancedMode();
  void setMute();
  void setPorgname();
  void setProgramNo();
  void setPid();

  int setAudioSourceAndFreq();
  int setEPkt();
 signals:
  void sendMsg(Msg *g);
  void sendFileSize(int begin, int end, int bar);

 protected:
  void run();

 private:
  tbsfunc et;
  Msg *m;
};

#endif  // THREADTBS_H
