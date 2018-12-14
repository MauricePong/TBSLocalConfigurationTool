#include "threadtbs.h"
volatile bool swflg = 0;
volatile u8 mode = 0;
QString i_path;
volatile u8 workMode = 0;
volatile u8 manNo = 0;
u8 PNo = 0;
volatile int Vpid = 0;
volatile int Apid = 0;
volatile int Pmt = 0;
volatile int Pcr = 0;
volatile int HpFile = 0;
volatile int HpLevel = 0;
volatile int PixW = 0;
volatile int PixH = 0;
volatile int BitRate = 0;
volatile int Frame = 0;
volatile int MuteEn = 0;
volatile float igvr = 0.0;
u8 Pname[24] = {'0'};
u8 audio_parameter[2] = {0};
u8 epkt = 0;
threadTbs::threadTbs() : QThread() {
  swflg = 0;
  m = new Msg();
  m->ok = 0;
  m->type = 0;
  m->progress = 0;
}

threadTbs::~threadTbs() {}

void threadTbs::fileSizeLimit() {
  if (1 == IsSizeOrTimeFlg) {
    int kbc = (int)((KbCount * 100) / (MaxFileSize * 1024));
    emit sendFileSize(0, 100, kbc);
  } else if (0 == IsSizeOrTimeFlg) {
    emit sendFileSize(0, 100, 0);
  }
}

void threadTbs::writeBuffer() {
  m->ok = 1;
  m->type = 1;
  m->progress = 0;
  emit sendMsg(m);
  // qDebug() << "work" << workMode;

  switch (workMode) {
    case 1:
      // 1.AutoMode
      autoMode();
      break;
    case 2:
      // 2.ManualMode
      manualMoe();
      break;
    case 3:
      // 3.AdvancedMode
      advancedMode();
      break;
    default:
      break;
  }

  // 4.setMute
  setMute();
  // 5.setProgrameName
  setPorgname();
  // 6.setProgrameNo
  setProgramNo();
  // 7.setProgramePid
  setPid();
  if ((5301 == typeId) || (6308 == typeId) ||(6304 == typeId)) {
    // 8.setAudioSourceAndFreq
    setAudioSourceAndFreq();
    setEPkt();
  }

  et.restartHdmi();
  m->ok = 1;
  m->type = 2;
  m->progress = 0;
  emit sendMsg(m);
  Sleep(1);
  m->ok = 1;
  m->type = 4;
  m->progress = 0;
  emit sendMsg(m);
  return;
}

void threadTbs::autoMode() {
  u8 tepdate[4] = {0};
  // set to 24cxx 0x14 to 1
  tepdate[0] = 1;
  et.target_ext_memory_rd_wt(WRITE, tuner * 0x80 + 0x14, tepdate, 1);
  tepdate[0] = 0;
  et.target_ext_memory_rd_wt(WRITE, tuner * 0x80 + 0x15, tepdate, 1);
}

void threadTbs::manualMoe() {
  u8 tepdate[4] = {0};
  // set to 24cxx 0x14 to 0 and 0x15 to num
  tepdate[0] = 0;
  et.target_ext_memory_rd_wt(WRITE, tuner * 0x80 + 0x14, tepdate, 1);
  tepdate[0] = manNo;
  et.target_ext_memory_rd_wt(WRITE, tuner * 0x80 + 0x15, tepdate, 1);
  // qDebug("set output format to manual mode:");
  // SetCaptureFormat(0,num);
}

void threadTbs::advancedMode() {
  u8 tepdate[4] = {0};
  // set to 24cxx 0x14 to 0 and 0x15 to 0
  tepdate[0] = 0;
  et.target_ext_memory_rd_wt(WRITE, tuner * 0x80 + 0x14, tepdate, 1);
  et.target_ext_memory_rd_wt(WRITE, tuner * 0x80 + 0x15, tepdate, 1);
  // qDebug("set output format to advanced:
  // %d,%d,%d,%d,%d,%d",ad.profileNum,ad.levelNum,ad.H_Num,ad.V_Num,ad.bitrateNum,ad.frameNum);

  int outsettingAddress[11];
  int outsettingFormat[11];

  outsettingAddress[0] = 0x1526;  // profileLevel
  outsettingAddress[1] = 0x152c;  // H
  outsettingAddress[2] = 0x152e;  // V
  outsettingAddress[3] = 0x1532;  // ES
  outsettingAddress[4] = 0x1534;  // Max
  outsettingAddress[5] = 0x1536;  // AVE
  outsettingAddress[6] = 0x1538;  // min
  outsettingAddress[7] = 0x1104;  // sys bit rate
  outsettingAddress[8] = 0x1584;  // framRate
  outsettingAddress[9] = 0x1586;
  outsettingAddress[10] = 0x1588;

  if (HpFile == 1)
    HpLevel += 512;  // Main profile
  else
    HpLevel += 256;  // high profile

  // write data
  outsettingFormat[0] = HpLevel;
  outsettingFormat[1] = PixW;
  outsettingFormat[2] = PixH;

  // 3-7

  outsettingFormat[3] = BitRate - 200;
  outsettingFormat[4] = BitRate - 300;
  outsettingFormat[5] = ((BitRate - 300) / igvr);
  outsettingFormat[6] =
      (outsettingFormat[5] >= 5000)
          ? ((outsettingFormat[5] >= 10000)
                 ? outsettingFormat[5] * 0.7
                 : outsettingFormat[5] *
                       ((outsettingFormat[5] / 5000 -1) * 0.2 + 0.5))
          : outsettingFormat[5] * 0.5;
  outsettingFormat[7] = BitRate;
  qDebug("%d %d %d %d %d", outsettingFormat[3], outsettingFormat[4],
         outsettingFormat[5], outsettingFormat[6], outsettingFormat[7]);
  outsettingFormat[8] = 0;
  outsettingFormat[9] = Frame * 2;
  outsettingFormat[10] = 1;

  unsigned char iwrNum[4] = {0};
  iwrNum[1] = 11;
  et.target_ext_memory_rd_wt(WRITE, 0x80 * tuner + 0x10, iwrNum, 2);
  et.write_M23Register(11, outsettingAddress, outsettingFormat);
}

void threadTbs::setMute() {
  unsigned char temdate[4] = {0};
  temdate[0] = MuteEn;
  et.target_ext_memory_rd_wt(WRITE, 0x80 * tuner + 0x16, temdate, 1);
}

void threadTbs::setPorgname() {
  unsigned char temdate[4] = {0};
  int len = 0;
  int i = 0;
  u8 *tempName = Pname;
  while (*tempName++) len++;
  for (int i = 0; i < len; i++) {
    qDebug() << "Pname:" << (char)Pname[i];
  }
  qDebug("len:%d", len);
  temdate[0] = 0x60;
  et.target_ext_memory_rd_wt(WRITE, 0x80 * tuner + 0x12, temdate, 1);
  temdate[0] = len;  // progname len
  et.target_ext_memory_rd_wt(WRITE, 0x80 * tuner + 0x13, temdate, 1);
  et.target_ext_memory_rd_wt(WRITE, 0x80 * tuner + 0x60, tempName, len);
  for (i = 0; i < len + 1; i++) {
    if (i == len) {
      temdate[0] = '\0';
    } else {
      temdate[0] = *(Pname + i);  // progname len
    }
    qDebug() << "char:" << (char)temdate[0];
    et.target_ext_memory_rd_wt(WRITE, 0x80 * tuner + 0x60 + i, temdate, 1);
  }
}

void threadTbs::setProgramNo() {
  et.target_ext_memory_rd_wt(WRITE, PROGRAM_START_NO(tuner), &PNo, 1);
}

void threadTbs::setPid() {
  u8 tmpbuf[8] = {0};
  tmpbuf[0] = Pmt >> 8 & 0xff;
  tmpbuf[1] = Pmt & 0xff;
  tmpbuf[2] = Pcr >> 8 & 0xff;
  tmpbuf[3] = Pcr & 0xff;
  tmpbuf[4] = Vpid >> 8 & 0xff;
  tmpbuf[5] = Vpid & 0xff;
  tmpbuf[6] = Apid >> 8 & 0xff;
  tmpbuf[7] = Apid & 0xff;
  et.target_ext_memory_rd_wt(WRITE, PID(tuner), tmpbuf, 8);
}

int threadTbs::setAudioSourceAndFreq() {
  // u8 tmpbuf[8];
  // eg: 0x17 = 1; 0x18 = 1;
  // tmpbuf[0] = 0; //0->hdmi 1->audio
  // tmpbuf[1] = 0; //00->48k,01->44.1k,10->32k
  qDebug() << "audio_parameter:" << audio_parameter[0] << ","
           << audio_parameter[1];
  et.target_ext_memory_rd_wt(
      WRITE, GPIOBASE_ADDR + AUDIIO_SELECT_OFFSETADDR + 0x80 * tuner,
      audio_parameter, 2);
  return 0;
}
int threadTbs::setEPkt() {
  u8 tmp[4] = {0};
  tmp[0] = epkt;
  et.target_ext_memory_rd_wt(WRITE, GPIOBASE_ADDR + WTEMPTYPKT(tuner), tmp, 1);
  return 0;
}
void threadTbs::run() {
  while (1 != swflg) {
    if (mode == 0) {
      Sleep(1);
      continue;
    }
    switch (mode) {
      case 6:
        writeBuffer();
        break;
      case 7:
        fileSizeLimit();
        break;
    }
    mode = 0;
  }
  delete m;
  this->quit();
  this->wait();
}
