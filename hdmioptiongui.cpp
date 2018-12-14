#pragma execution_character_set("utf-8")

#include "hdmioptiongui.h"
#include "iconhelper.h"
#include "ui_hdmioptiongui.h"
#define ITMEL(n) ITMEL##n
#define SetInputFmt(n) ui->lin_InFmt->setText(n)
#define GetProgrameName ui->lin_PgmNa->text()
#define SetProgrameName(n) ui->lin_PgmNa->setText(n)
#define GetProgrameNo ui->lin_PgmNo->text().toInt()
#define SetProgrameNo(n) ui->lin_PgmNo->setText(n)
#define GetVpid ui->lin_Vpid->text().toInt()
#define SetVpid(n) ui->lin_Vpid->setText(n)
#define GetApid ui->lin_Apid->text().toInt()
#define setApid(n) ui->lin_Apid->setText(n)
#define GetPcr ui->lin_Pcr->text().toInt()
#define SetPcr(n) ui->lin_Pcr->setText(n)
#define GetPmt ui->lin_Pmt->text().toInt()
#define SetPmt(n) ui->lin_Pmt->setText(n)
#define GetSn ui->lin_Sn->text().toInt()
#define SetSn(n) ui->lin_Sn->setText(n)
#define GetHprofile ui->com_Hprofile->currentIndex()
#define SetHprofile(n) ui->com_Hprofile->setCurrentIndex(n)
#define GetHlevel ui->com_Hlevel->currentIndex()
#define SetHlevel(n) ui->com_Hlevel->setCurrentIndex(n)
#define GetW ui->lin_Wid->text().toInt()
#define SetW(n) ui->lin_Wid->setText(n)
#define GetH ui->lin_Hig->text().toInt()
#define SetH(n) ui->lin_Hig->setText(n)
#define GetAvBR ui->lin_AvBitRte->text().toInt()
#define SetAvBR(n) ui->lin_AvBitRte->setText(n)
#define GetFrame ui->lin_Enco->text().toInt()
#define SetFrame(n) ui->lin_Enco->setText(n)
#define SetMute(n) \
  ui->che_Mute->setCheckState(((n) ? (Qt::Checked) : (Qt::Unchecked)))  //设置
#define GetMute ui->che_Mute->isChecked()  // == Qt::Checked
#define SetAout(n) ui->com_Aout->setCurrentIndex(n)
#define GetAout ui->com_Aout->currentIndex()
#define SetAFre(n) ui->com_AFre->setCurrentIndex(n)
#define GetAFre ui->com_AFre->currentIndex()

// int couCpu = 0;

int tunerflg = 0;

Hdmioptiongui::Hdmioptiongui(QWidget *parent)
    : QDialog(parent), ui(new Ui::Hdmioptiongui) {
  ui->setupUi(this);
  this->initForm();
  go_win();
  readConfig();
}

Hdmioptiongui::~Hdmioptiongui() {
  swflg = 1;
  u8 a = 0;
  if (1 == tunerflg) {
    a = 0;
    tbs.monopolizeCpu_w(0x00, &a, 1);  // set 0
    StopDevice();
  }
  CoUninitialize();
  free(DL);
  delete qtimer;
  if (1 == IsStartFlg) {
    stopBtn();
    UnInit_RTP_OR_UDP();
  }

  delete ui;
}

void Hdmioptiongui::go_win() {
  DL = (PDevAPIDevList)malloc(sizeof(TDevAPIDevList));
  DeviceGetList();
  CoInitializeEx(NULL, COINIT_MULTITHREADED | COINIT_SPEED_OVER_MEMORY);
}

int Hdmioptiongui::addTreeIteml(char *bf, char **bf_c, PDevAPIDevInfo imch) {
  ui->tree_Dev->setColumnCount(1);             //设置列数
  ui->tree_Dev->setHeaderLabel(tr("Device"));  //设置头的标题
  ui->tree_Dev->setIconSize(QSize(25, 25));
  QTreeWidgetItem *im =
      new QTreeWidgetItem(ui->tree_Dev, QStringList(QString(bf)));
  im->setIcon(0, QIcon(":/image/pci.png"));
  int i = 0;
  while (bf_c[i] != NULL) {
    imch[i].Item = new QTreeWidgetItem(im, QStringList(QString(bf_c[i])));
    imch[i].Item->setIcon(0, QIcon(":/qss/psblack/chun.png"));
    im->addChild(imch[i].Item);
    i++;
  }
  ui->tree_Dev->expandAll();
  return 0;
}

int Hdmioptiongui::readBuffer() {
  // 1.inputfmt
  Iputfmt();
  // 2.sn
  getSn();
#if 1
  // 3.outputfmt
  outputfmt();
  // 4.mute
  getMute();
  // 5.name
  getName();
  // 6.no
  getNo();
  // 7.pid
  getPid();
  // 8getAudioSourceAndFreq
  if ((5301 == typeId) || (6308 == typeId) || (6304 == typeId)) {
    if ((6308 == typeId) || (6304 == typeId)) {
      ui->com_Aout->clear();
      ui->com_Aout->addItem("HDMI", 0);
      getEpkt();
    } else if (5301 == typeId) {
      ui->com_Aout->clear();
      ui->com_Aout->addItem("HDMI", 0);
      ui->com_Aout->addItem("Audio Line(L/R)", 1);
    }
    ui->com_Aout->setEnabled(true);
    ui->com_AFre->setEnabled(true);
    ui->lab_AFrom->setEnabled(true);
    ui->lab_AFreq->setEnabled(true);

    getAudioSourceAndFreq();

  } else {
    ui->com_Aout->setEnabled(false);
    ui->com_AFre->setEnabled(false);
    ui->lab_AFrom->setEnabled(false);
    ui->lab_AFreq->setEnabled(false);
  }

#endif
  return 0;
}

int Hdmioptiongui::Iputfmt() {
  char showbuf[32];
  unsigned char tmpbuf[8];
  unsigned short VRow, VColumn;
  unsigned short VFreq;
  unsigned short VScan;
  char cScan;
  tbs.target_ext_memory_rd_wt(READ, ADDRESS_HDMI(tuner) + OFFSET_START, tmpbuf,
                              8);
  VColumn = ((tmpbuf[6] << 8) + tmpbuf[7]);
  VRow = ((tmpbuf[4] << 8) + tmpbuf[5]);
  VFreq = tmpbuf[2];
  VScan = tmpbuf[1] & 0x1;
  if (VFreq >= 57 && VFreq <= 60) VFreq = 60;
  if (VFreq >= 47 && VFreq <= 50) VFreq = 50;
  if (VFreq == 23) VFreq = 24;
  cScan = (VScan == 1) ? 'I' : 'p';
  sprintf(showbuf, "%d*%d%c  %dHZ", VRow, VColumn, cScan, VFreq);

  if (0 == tmpbuf[0]) {
    SetInputFmt(QString("No input source"));
    ui->com_EcFr->clear();
    noSingleFlg = 1;
  } else {
    SetInputFmt(showbuf);
    ui->com_EcFr->clear();
    if ((30 == VFreq) || (60 == VFreq)) {
      ui->com_EcFr->addItem("30", 0);
      ui->com_EcFr->addItem("60", 1);
    } else if ((25 == VFreq) || (50 == VFreq)) {
      ui->com_EcFr->addItem("25", 0);
      ui->com_EcFr->addItem("50", 1);
    } else if (24 == VFreq) {
      ui->com_EcFr->addItem("24", 0);
    }
    noSingleFlg = 0;
  }
  return 0;
}

int Hdmioptiongui::getSn() {
  u8 bytes[6] = {0};
  char showbuf[32];
  tbs.target_ext_memory_rd_wt(READ, 0x80 * tuner + 0x08, bytes, 6);
  sprintf(showbuf, "MAC:%02x:%02x:%02x:%02x:%02x:%02x", bytes[0], bytes[1],
          bytes[2], bytes[3], bytes[4], bytes[5]);
  SetSn(showbuf);
  return 0;
}

int Hdmioptiongui::outputfmt() {
  u8 iwrNum[8] = {0};
  int outsettingFormat[11] = {0};
  tbs.target_ext_memory_rd_wt(READ, 0x80 * tuner + 0x14, iwrNum, 1);

  if (iwrNum[0] == 1) {  // auto or not
    qDebug("the latest setting is Auto mode!");
    iwrNum[0] = 1;
    iwrNum[1] = 0;
    iwrNum[2] = 0;
    ui->rad_Aut->setChecked(1);
    ui->sta_Mode->setCurrentIndex(0);
    workMode = 1;
    return 0;
  }
  tbs.target_ext_memory_rd_wt(READ, 0x80 * tuner + 0x15, iwrNum, 1);
  if (iwrNum[0] != 0) {  // manual or not
    ui->rad_Man->setChecked(1);
    ui->sta_Mode->setCurrentIndex(1);
    manNo = iwrNum[0];
    workMode = 2;
    switch (manNo) {
      case 1:
        qDebug("super 1920*1080");
        ui->rad_Su->setChecked(1);
        break;
      case 2:
        ui->rad_Ult->setChecked(1);
        qDebug("ultra 1440*1080 ");
        break;
      case 3:
        ui->rad_Exc->setChecked(1);

        qDebug("excellent 1280*720");
        break;
      case 4:
        ui->rad_Good->setChecked(1);
        qDebug("good 1120*640");
        break;
      case 5:
        ui->rad_Fair->setChecked(1);
        qDebug("fair 960*540");
        break;
      case 6:
        ui->rad_Poor->setChecked(1);
        qDebug("poor 800*480");
        break;
      case 7:
        ui->rad_Mob->setChecked(1);
        qDebug("mobile 480*272");
        break;
      default:
        qDebug("no");
        break;
    }
    return 0;
  }
  tbs.target_ext_memory_rd_wt(READ, 0x80 * tuner + 0x10, iwrNum, 2);
  if (iwrNum[1] == 11) {
    workMode = 3;
    qDebug("the latest setting is Advance mode!");
    int level[6] = {30, 31, 32, 40, 41, 42};
    tbs.read_M23Register(11, outsettingFormat);
    ui->rad_Adv->setChecked(1);
    ui->sta_Mode->setCurrentIndex(2);

    SetW(QString("%1").arg(outsettingFormat[1]));
    SetH(QString("%1").arg(outsettingFormat[2]));
    // SetFrame(QString("%1").arg(outsettingFormat[9] /2));
    if (((outsettingFormat[9] / 2) >= 24) && ((outsettingFormat[9] / 2) < 35)) {
      ui->com_EcFr->setCurrentIndex(0);
    } else if ((outsettingFormat[9] / 2) >= 45) {
      ui->com_EcFr->setCurrentIndex(1);
    }
    ui->lin_Sysbr->setText(QString("%1").arg(outsettingFormat[7]));
    float igvr = ((float)outsettingFormat[7] - 300.0) / (float)outsettingFormat[5];
    qDebug("outsettingFormat[7] = %d, outsettingFormat[5] = %d igvr :%f",
           outsettingFormat[7], outsettingFormat[5],igvr);
    ui->lin_Imgvr->setText(QString::number(igvr, 'f', 2));
    if (outsettingFormat[0] > 512) {
      SetHprofile(1);
      switch (outsettingFormat[0] - 512) {
        case 30:
          SetHlevel(0);
          break;
        case 31:
          SetHlevel(1);
          break;
        case 32:
          SetHlevel(2);
          break;
        case 40:
          SetHlevel(3);
          break;
        case 41:
          SetHlevel(4);
          break;
        case 42:
          SetHlevel(5);
          break;
        default:
          break;
      }
    } else {
      SetHprofile(0);
      switch (outsettingFormat[0] - 256) {
        case 30:
          SetHlevel(0);
          break;
        case 31:
          SetHlevel(1);
          break;
        case 32:
          SetHlevel(2);
          break;
        case 40:
          SetHlevel(3);
          break;
        case 41:
          SetHlevel(4);
          break;
        case 42:
          SetHlevel(5);
          break;
        default:
          break;
      }
    }
  }
  return 0;
}

int Hdmioptiongui::getMute() {
  unsigned char temdate[4] = {0};
  tbs.target_ext_memory_rd_wt(READ, 0x80 * tuner + 0x16, temdate, 1);
  if (1 == temdate[0]) {
    SetMute(1);
  } else {
    SetMute(0);
  }
  return 0;
}

int Hdmioptiongui::getName() {
  unsigned char temdate[4] = {0};
  int len = 0;
  int i = 0;
  tbs.target_ext_memory_rd_wt(READ, 0x80 * tuner + 0x12, temdate, 1);
  if (0x60 != temdate[0]) {
    return 0;
  }
  tbs.target_ext_memory_rd_wt(READ, 0x80 * tuner + 0x13, temdate, 1);
  len = temdate[0];
  char programe[64] = {'0'};

  for (i = 0; i < len; i++) {
    tbs.target_ext_memory_rd_wt(READ, 0x80 * tuner + 0x60 + i,
                                (u8 *)&programe[i], 1);
  }
  tbs.target_ext_memory_rd_wt(READ, 0x80 * tuner + 0x60, (u8 *)programe, len);
  SetProgrameName(programe);

  return 0;
}

int Hdmioptiongui::getNo() {
  ui->lin_PgmNo->setDisabled(0);
  u8 num = 0;
  tbs.target_ext_memory_rd_wt(READ, PROGRAM_START_NO(tuner), &num, 1);
  SetProgrameNo(QString("%1").arg(num));
  return 0;
}

int Hdmioptiongui::getPid() {
  ui->lin_Vpid->setDisabled(0);
  ui->lin_Apid->setDisabled(0);
  ui->lin_Pcr->setDisabled(0);
  ui->lin_Pmt->setDisabled(0);
  u8 tmpbuf[8] = {0};
  for (int i = 0; i < 8; i++) {
    tbs.target_ext_memory_rd_wt(READ, PID(tuner) + i, &tmpbuf[i], 1);
  }

  u8 eachpid[2];
  eachpid[1] = tmpbuf[0];
  eachpid[0] = tmpbuf[1];
  SetPmt(QString("%1").arg(*(u16 *)eachpid));
  eachpid[1] = tmpbuf[2];
  eachpid[0] = tmpbuf[3];
  SetPcr(QString("%1").arg(*(u16 *)eachpid));
  eachpid[1] = tmpbuf[4];
  eachpid[0] = tmpbuf[5];
  SetVpid(QString("%1").arg(*(u16 *)eachpid));
  eachpid[1] = tmpbuf[6];
  eachpid[0] = tmpbuf[7];
  setApid(QString("%1").arg(*(u16 *)eachpid));
  return 0;
}

int Hdmioptiongui::getAudioSourceAndFreq() {
  u8 tmpbuf[8];
  tbs.target_ext_memory_rd_wt(
      READ, GPIOBASE_ADDR + AUDIIO_SELECT_OFFSETADDR + 0x80 * tuner, tmpbuf, 2);
  u8 audioSrc = tmpbuf[0] & 0x01;
  qDebug() << "audioSrc:" << audioSrc;
  SetAout(audioSrc);
  u8 audioFreq = tmpbuf[1] & 0x03;
  qDebug() << "audioFreq:" << audioFreq;
  SetAFre(audioFreq);
  return 0;
}

int Hdmioptiongui::getEpkt() {
  u8 tmpbuf[8];
  tbs.target_ext_memory_rd_wt(READ, RDEMPTYPKT(tuner), tmpbuf, 1);
  qDebug("epkt:%d",tmpbuf[0]);
  ui->com_Epkt->setCurrentIndex(tmpbuf[0] & 0x01);
  return 0;
}

int Hdmioptiongui::readConfig() {
  char buf[64] = {'\0'};
  if ((configfp = fopen(".tbs", "r")) == NULL) {
    ui->lin_TarIp->setText(QString("192.168.1.188"));
    ui->lin_TarPort->setText(QString("5442"));
    ui->com_Net->setCurrentIndex(0);
    qDebug("fail to fopen:%s", strerror(errno));
    return -1;
  }
  int n = fread(buf, 1, 64, configfp);
  if (n < 15) {
    qDebug("fail to fread:%s", strerror(errno));
    ui->lin_TarIp->setText(QString("192.168.1.188"));
    ui->lin_TarPort->setText(QString("5442"));
    ui->com_Net->setCurrentIndex(0);
    return -1;
  }
  QString targetIp = QString(buf).section(':', 0, 0).trimmed();
  int targetPort = QString(buf).section(':', 1, 1).trimmed().toInt();
  int protocol = QString(buf).section(':', 2, 2).trimmed().toInt();
  ui->lin_TarIp->setText(targetIp);
  ui->lin_TarPort->setText(QString("%1").arg(targetPort));
  ui->com_Net->setCurrentIndex(protocol);
  fclose(configfp);
  configfp = NULL;
  return 0;
}

int Hdmioptiongui::writeConfig() {
  if ((configfp = fopen(".tbs", "w")) == NULL) {
    qDebug("fail to fopen:%s", strerror(errno));
    return -1;
  }
  char *tarIpbuf = ui->lin_TarIp->text().toLatin1().data();
  int tarPortInt = ui->lin_TarPort->text().toInt();
  int pNet = ui->com_Net->currentIndex();
  char wbuf[64] = {'\0'};
  sprintf(wbuf, "%s:%d:%d", tarIpbuf, tarPortInt, pNet);
  qDebug() << QString(wbuf);
  int n = fwrite(wbuf, 1, 64, configfp);
  if (n < 15) {
    qDebug("fail to fwrite:%s", strerror(errno));
    return -1;
  }
  fclose(configfp);
  configfp = NULL;
  return 0;
}

bool Hdmioptiongui::eventFilter(QObject *watched, QEvent *event) {
  if (event->type() == QEvent::MouseButtonDblClick) {
    if (watched == ui->widgetTitle) {
      on_btnMenu_Max_clicked();
      return true;
    }
  }

  return QWidget::eventFilter(watched, event);
}

void Hdmioptiongui::initForm() {
  this->setProperty("form", true);
  this->setProperty("canMove", true);
  this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowSystemMenuHint |
                       Qt::WindowMinMaxButtonsHint);

  // QPixmap icon(":/image/logo.png");
  QPixmap icon(":/qss/psblack/uilogo/logo.png");
  ui->labIco->setPixmap(icon);
  ui->labIco->resize(icon.width(), icon.height());
  // IconHelper::Instance()->setIcon(ui->labIco, QChar(0xf099), 30);
  IconHelper::Instance()->setIcon(ui->btnMenu_Min, QChar(0xf068));
  IconHelper::Instance()->setIcon(ui->btnMenu_Max, QChar(0xf067));
  IconHelper::Instance()->setIcon(ui->btnMenu_Close, QChar(0xf00d));

  IconHelper::Instance()->setIcon(ui->lab_enico, QChar(0xf069), 30);

  // ui->widgetMenu->setVisible(false);
  ui->widgetTitle->installEventFilter(this);
  ui->widgetTitle->setProperty("form", "title");
  ui->widgetTop->setProperty("nav", "top");
  // ui->labTitle->setText("TBS CAPTURE");
  ui->labTitle->setFont(QFont("Microsoft Yahei", 20));
  this->setWindowTitle(ui->labTitle->text());

  // ui->stackedWidget->setStyleSheet("QLabel{font:10pt;}");

  //单独设置指示器大小
  int addWidth = 20;
  int addHeight = 10;
  int rbtnWidth = 15;
  int ckWidth = 13;
  int scrWidth = 12;
  int borderWidth = 3;

  QStringList qss;
  qss.append(QString("QComboBox::drop-down,QDateEdit::drop-down,QTimeEdit::"
                     "drop-down,QDateTimeEdit::drop-down{width:%1px;}")
                 .arg(addWidth));
  qss.append(QString("QComboBox::down-arrow,QDateEdit[calendarPopup=\"true\"]::"
                     "down-arrow,QTimeEdit[calendarPopup=\"true\"]::down-arrow,"
                     "QDateTimeEdit[calendarPopup=\"true\"]::down-arrow{width:%"
                     "1px;height:%1px;right:2px;}")
                 .arg(addHeight));
  qss.append(QString("QRadioButton::indicator{width:%1px;height:%1px;}")
                 .arg(rbtnWidth));
  qss.append(
      QString("QCheckBox::indicator,QGroupBox::indicator,QTreeWidget::"
              "indicator,QListWidget::indicator{width:%1px;height:%1px;}")
          .arg(ckWidth));
  qss.append(QString("QScrollBar:horizontal{min-height:%1px;border-radius:%2px;"
                     "}QScrollBar::handle:horizontal{border-radius:%2px;}"
                     "QScrollBar:vertical{min-width:%1px;border-radius:%2px;}"
                     "QScrollBar::handle:vertical{border-radius:%2px;}")
                 .arg(scrWidth)
                 .arg(scrWidth / 2));
  qss.append(QString("QWidget#widget_top>QToolButton:pressed,QWidget#widget_"
                     "top>QToolButton:hover,"
                     "QWidget#widget_top>QToolButton:checked,QWidget#widget_"
                     "top>QLabel:hover{"
                     "border-width:0px 0px %1px 0px;}")
                 .arg(borderWidth));
  qss.append(QString("QWidget#widgetleft>QPushButton:checked,QWidget#"
                     "widgetleft>QToolButton:checked,"
                     "QWidget#widgetleft>QPushButton:pressed,QWidget#"
                     "widgetleft>QToolButton:pressed{"
                     "border-width:0px 0px 0px %1px;}")
                 .arg(borderWidth));

  this->setStyleSheet(qss.join(""));

  // QMessageBox{ color: red }
  //设置MODE按钮
  QList<QRadioButton *> rads = ui->gro_Mode->findChildren<QRadioButton *>();
  foreach (QRadioButton *rd, rads) {
    rd->setCheckable(true);
    connect(rd, SIGNAL(clicked()), this, SLOT(radioClick()));
  }
  //设置Manual按钮
  QList<QRadioButton *> mas = ui->pag_Man->findChildren<QRadioButton *>();
  foreach (QRadioButton *ma, mas) {
    ma->setCheckable(true);
    connect(ma, SIGNAL(clicked()), this, SLOT(radioClick_M()));
  }

  //设置
  QList<QRadioButton *> radsC = ui->grpBox->findChildren<QRadioButton *>();
  foreach (QRadioButton *rdC, radsC) {
    rdC->setCheckable(true);
    connect(rdC, SIGNAL(clicked()), this, SLOT(radioClick_C()));
  }

  ui->rad_Encode->click();
  ui->rad_Aut->click();
  connect(ui->tree_Dev, SIGNAL(itemClicked(QTreeWidgetItem *, int)), this,
          SLOT(showSelectedImage(QTreeWidgetItem *, int)));
  emflg = 0;
  loadflg = 0;
  th = new threadTbs();
  connect(th, SIGNAL(sendMsg(Msg *)), this, SLOT(show_status(Msg *)));
  connect(th, SIGNAL(sendFileSize(int, int, int)), this,
          SLOT(progressBarShow(int, int, int)));
  th->start();
  width_s = this->size().width();
  height_s = this->size().height();
  width = width_s;
  height = height_s;

  // recorder
  IsOpenFileFlg = 0;
  IsStartFlg = 0;
  IsSizeOrTimeFlg = 0;
  ui->tabWidget->setCurrentIndex(0);
  qtimer = new QTimer(this);
  connect(qtimer, SIGNAL(timeout()), this, SLOT(timeServiceFunc()));

  ui->stackedWidget_RS->setCurrentIndex(0);
  isSendTs = 0;
  isRtp = 0;
  fontsize = 0;
  ui->btn_Start->setEnabled(true);
  ui->btn_Cancel->setEnabled(false);
}

void Hdmioptiongui::radioClick() {
  QRadioButton *r = (QRadioButton *)sender();
  QString name = r->text();

  QList<QRadioButton *> rads = ui->gro_Mode->findChildren<QRadioButton *>();
  foreach (QRadioButton *ra, rads) {
    if (ra == r) {
      ra->setChecked(true);
    } else {
      ra->setChecked(false);
    }
  }

  if (name == "Auto") {
    ui->sta_Mode->setCurrentIndex(0);
    workMode = 1;
  } else if (name == "Manual") {
    ui->sta_Mode->setCurrentIndex(1);
    workMode = 2;
  } else if (name == "Advanced") {
    ui->sta_Mode->setCurrentIndex(2);
    workMode = 3;
  }
}

void Hdmioptiongui::radioClick_M() {
  QRadioButton *rb = (QRadioButton *)sender();
  QString name = rb->text();

  QList<QRadioButton *> mas = ui->pag_Man->findChildren<QRadioButton *>();
  foreach (QRadioButton *ma, mas) {
    if (ma == rb) {
      ma->setChecked(true);
    } else {
      ma->setChecked(false);
    }
  }

  if (name == "Super(1920*1080)") {
    qDebug() << "s";
    manNo = 1;
  } else if (name == "Ultra(1440*1080)") {
    qDebug() << "u";
    manNo = 2;
  } else if (name == "Excellent(1280*720)") {
    qDebug() << "e";
    manNo = 3;
  } else if (name == "Good(1120*640)") {
    qDebug() << "g";
    manNo = 4;
  } else if (name == "Fair(960*540)") {
    qDebug() << "f";
    manNo = 5;
  } else if (name == "Poor(800*480)") {
    qDebug() << "p";
    manNo = 6;
  } else if (name == "Mobile(480*272)") {
    qDebug() << "m";
    manNo = 7;
  }
}

void Hdmioptiongui::radioClick_C() {
  QRadioButton *r = (QRadioButton *)sender();
  QString name = r->text();

  QList<QRadioButton *> rads = ui->grpBox->findChildren<QRadioButton *>();
  foreach (QRadioButton *ra, rads) {
    if (ra == r) {
      ra->setChecked(true);
    } else {
      ra->setChecked(false);
    }
  }

  if (name == "Encoding") {
    ui->sta_DevWdeget->setCurrentIndex(0);
  } else if (name == "Recording") {
    ui->sta_DevWdeget->setCurrentIndex(1);
    ui->stackedWidget_RS->setCurrentIndex(0);

    ui->gbox_record->setEnabled(false);
    ui->lab_FileName->setText(QString(""));
    ui->lab_FileUnit->setText(QString(""));
    ui->lin_SizeAndTime->setText(QString(""));
    ui->bar_Record->setValue(0);
    if (0 == IsStartFlg) {
      isSendTs = 0;  // 0,record; 1.sendts
    }
  } else if (name == "Streaming") {
    ui->sta_DevWdeget->setCurrentIndex(1);
    ui->stackedWidget_RS->setCurrentIndex(1);
    if (0 == IsStartFlg) {
      isSendTs = 1;  // 0,record; 1.sendts
    }
  }
}

void Hdmioptiongui::showSelectedImage(QTreeWidgetItem *item, int column) {
  QTreeWidgetItem *parent = item->parent();
  if (NULL == parent)  //注意：最顶端项是没有父节点的，双击这些项时注意(陷阱)
    return;
  u8 a = 0;
  int g = 0;
  tbs.monopolizeCpu_w(0x00, &a, 1);

  if (1 == tunerflg) {
    if ((typeId == 5301) || (typeId == 6304) || (typeId == 6308)) {
      if (1 == IsStartFlg) {
        stopBtn();
        UnInit_RTP_OR_UDP();
      }
      StopDevice();
    }
  }

  if ("TBS5301" == parent->text(0)) {
    typeId = 5301;
  } else if ("TBS6304(4 tuner)" == parent->text(0)) {
    typeId = 6304;
  } else if ("TBS6304(8 tuner)" == parent->text(0)) {
    typeId = 6308;
  } else {
    typeId = 0;
    return;
  }
  SetSn(QString(""));

  // qDebug() <<"DLcount:"<< DL->Count;
  for (int i = 0; i < DL->Count; i++) {
    DL->Devices[i].Item->setIcon(0, QIcon(":/qss/psblack/chun.png"));
    if (DL->Devices[i].Item == item) {
      col = i;
      g = 1;
    }
  }
  if (0 == g) {
    return;
  }

  tuner = DL->Devices[col].TunerType;
  tunercpy = tuner;
  qDebug() << "deviceno:" << DL->Devices[col].deviceno;
  // col = parent->indexOfChild(item); //item在父项中的节点行号(从0开始)

  StartDevice(DL->Devices[col].deviceno);
  m_pTunerDevice = TunerFilter;
  m_pKsCtrl = NULL;
  m_pTunerPin = NULL;
  char input[7] = {'I', 'n', 'p', 'u', 't', '0', '\0'};
  m_pTunerPin = FindPinOnFilter(m_pTunerDevice, input);
  int ll_init = 0;
  HRESULT hr = S_OK;
  if (!ll_init && m_pTunerPin != NULL) {
    hr = m_pTunerPin->QueryInterface(IID_IKsPropertySet,
                                     reinterpret_cast<void **>(&m_pKsCtrl));
    if (FAILED(hr)) {
      ll_init = 1;
      qDebug("m_pTunerDevice QueryInterface Failed");
      m_pKsCtrl = NULL;
      return;
    }
  }
  m_pVCKsCtrl = NULL;
  if (6308 == typeId) {
    if (tuner >= 4) {
      tuner = tuner % 4;
      u32 tmp = tbs.rdReg32_Extern(0x20);
      qDebug("tmp:%#x", tmp);
      if (0x4000863 != tmp) {
        StopDevice();
        typeId = 0;
        if (QMessageBox::Ok ==
            QMessageBox::warning(this, tr("warn"),
                                 tr("Undetected equipment                  "),
                                 QMessageBox::Ok, QMessageBox::Ok)) {
          return;
        }
        return;
      }
    }
  }
  a = 1;
  tbs.monopolizeCpu_w(0x00, &a, 1);  // set 1
  readBuffer();
  lockSingle();
  item->setIcon(0, QIcon(":/qss/psblack/ch.png"));
  tunerflg = 1;
}

IPin *Hdmioptiongui::FindPinOnFilter(IBaseFilter *pBaseFilter, char *pPinName) {
  HRESULT hr;
  IEnumPins *pEnumPin = NULL;
  ULONG CountReceived = 0;
  IPin *pPin = NULL, *pThePin = NULL;
  char String[80];
  char *pString;
  PIN_INFO PinInfo;
  int length;

  if (!pBaseFilter || !pPinName) return NULL;

  // enumerate of pins on the filter
  hr = pBaseFilter->EnumPins(&pEnumPin);
  if (hr == S_OK && pEnumPin) {
    pEnumPin->Reset();
    while (pEnumPin->Next(1, &pPin, &CountReceived) == S_OK && pPin) {
      memset(String, NULL, sizeof(String));

      hr = pPin->QueryPinInfo(&PinInfo);
      if (hr == S_OK) {
        length = wcslen(PinInfo.achName) + 1;
        pString = new char[length];

        // get the pin name
        WideCharToMultiByte(CP_ACP, 0, PinInfo.achName, -1, pString, length,
                            NULL, NULL);

        // strcat (String, pString);
        StringCbCatA(String, strlen(String) + strlen(pString) + 1, pString);

        // is there a match
        if (strstr(String, pPinName))
          pThePin = pPin;  // yes
        else
          pPin = NULL;  // no

        delete pString;

      } else {
        // need to release this pin
        pPin->Release();
      }

    }  // end if have pin

    // need to release the enumerator
    pEnumPin->Release();
  }

  // return address of pin if found on the filter
  return pThePin;
}

void Hdmioptiongui::ConnectPins(IGraphBuilder *builder, IBaseFilter *out_from,
                                IBaseFilter *in_to, WCHAR *from_name,
                                WCHAR *to_name) {
  CComPtr<IEnumPins> pins;
  CComPtr<IPin> out_pin;
  CComPtr<IPin> in_pin;
  CComPtr<IPin> temp;
  PIN_INFO info;
  HRESULT hResult;
  BOOL bConnected;

  if (builder == NULL) {
    qDebug("ConnectPins builder == NULL");
    return;
  }
  if (out_from == NULL) {
    qDebug("ConnectPins out_from == NULL");
    return;
  }
  if (in_to == NULL) {
    qDebug("ConnectPins in_to == NULL");
    return;
  }

  // Find output pin of out_from
  out_from->EnumPins(&pins);
  pins->Reset();
  while (pins->Next(1, &out_pin, NULL) == S_OK) {
    out_pin->ConnectedTo(&temp);
    bConnected = temp.p != NULL;
    temp.Release();
    if (!bConnected) {
      out_pin->QueryPinInfo(&info);
      if ((info.dir == PINDIR_OUTPUT) &&
          ((from_name == NULL) ? TRUE : (wcscmp(info.achName, from_name) == 0)))
        break;
    }
    out_pin.Release();
  }
  pins.Release();
  if (!out_pin) {
    qDebug("CBDAGraph::ConnectPins() - No output pin found on filter");
    return;
  }
  // Find input pin of in_to
  in_to->EnumPins(&pins);
  pins->Reset();
  while (pins->Next(1, &in_pin, NULL) == S_OK) {
    in_pin->ConnectedTo(&temp);
    bConnected = temp.p != NULL;
    temp.Release();
    if (!bConnected) {
      in_pin->QueryPinInfo(&info);
      if ((info.dir == PINDIR_INPUT) &&
          ((to_name == NULL) ? TRUE : (wcscmp(info.achName, to_name) == 0)))
        break;
    }
    in_pin.Release();
  }
  pins.Release();
  if (!in_pin) {
    qDebug("CBDAGraph::ConnectPins() - No input pin found on filter");
    return;
  }
  // Join them together
  if ((hResult = builder->Connect(out_pin, in_pin)) != S_OK) {
    qDebug(
        "CBDAGraph::ConnectPins() - Could not connect pins together. Error "
        "was: ");
  }
}

void Hdmioptiongui::lockSingle() {
  // OnButtonAll
  HRESULT hr = S_OK;
  int m_nLNBSwitch = 20000;  //_ttoi(ls_lnbswitch);
  m_CmdData.LNB_POWER = LNB_POWER_ON;
  m_CmdData.HZ_22K = HZ_22K_OFF;
  m_CmdData.Tone_Data_Burst = Value_Burst_OFF;
  m_CmdData.DiSEqC_Port = DiSEqC_A;

  m_CmdData.motor[0] = 0;
  m_CmdData.motor[1] = 0;
  m_CmdData.motor[2] = 0;
  m_CmdData.motor[3] = 0;
  m_CmdData.motor[4] = 0;

  m_CmdData.Polarity = POLARITY_H;
  m_CmdData.ChannelFrequency = 12538;  //_ttoi(ls_freq);
  m_CmdData.ulLNBLOFLowBand = 11300;   //_ttoi(ls_lnblow);
  m_CmdData.ulLNBLOFHighBand = 11300;  //_ttoi(ls_lnbhigh);
  m_CmdData.SymbolRate = 41250;        //_ttoi(ls_symbol);
  m_CmdData.LNB_POWER = LNB_POWER_ON;

  /*
  do something
  */
  WaitForSingleObject(hIOMutex, INFINITE);
  // 1.CreateDVBTuneRequest
  CComPtr<IDVBTuneRequest> pTuneRequest;
  CComPtr<ITuneRequest> pNewTuneRequest;
  if (tuningspace == NULL) {
    qDebug() << "CreateTuneRequest: Can't create tune request";
    return;
  }
  //  Create an instance of the DVB tuning space
  ///////////////  - satelite -
  CComQIPtr<IDVBSTuningSpace> pDVBSTuningSpace(tuningspace);

  if (!pDVBSTuningSpace) qDebug() << "Cannot QI for an IDVBSTuningSpace";

  hr = pDVBSTuningSpace->put__NetworkType(CLSID_DVBSNetworkProvider);
  hr = pDVBSTuningSpace->put_LNBSwitch(m_nLNBSwitch *
                                       1000);  // updated 2011 12 31 liuy
  hr = pDVBSTuningSpace->put_HighOscillator(m_CmdData.ulLNBLOFHighBand * 1000);
  hr = pDVBSTuningSpace->put_LowOscillator(m_CmdData.ulLNBLOFLowBand * 1000);

  //  Create an empty tune request.
  hr = pDVBSTuningSpace->CreateTuneRequest(&pNewTuneRequest);

  if (FAILED(hr)) {
    qDebug() << "CreateTuneRequest: Can't create tune request.";
    return;
  }
  // query for an IDVBChannelTuneRequest interface pointer
  CComQIPtr<IDVBTuneRequest> pDVBTuneRequest(pNewTuneRequest);
  if (!pDVBTuneRequest) {
    qDebug() << "CreateDVBTuneRequest: Can't QI for IChannelTuneRequest.";
    return;
  }
  /*
  if(p_CmdData){
  hr=pDVBTuneRequest->put_SID(ch->m_SID);
  hr=pDVBTuneRequest->put_TSID(ch->m_TID);
  hr=pDVBTuneRequest->put_ONID(ch->m_NIT);
  };
  */
  ///////////////  - satelite -
  CComPtr<IDVBSLocator> pDVBSLocator;

  hr = pDVBSLocator.CoCreateInstance(CLSID_DVBSLocator);
  if (FAILED(hr)) {
    qDebug() << "Cannot create the DVBS locator failed";
    return;
  }
  if (&m_CmdData) {
    pDVBSLocator->put_OuterFEC(BDA_FEC_VITERBI);

    BinaryConvolutionCodeRate rates[10] = {
        BDA_BCC_RATE_NOT_DEFINED, BDA_BCC_RATE_1_2,  BDA_BCC_RATE_2_3,
        BDA_BCC_RATE_3_4,         BDA_BCC_RATE_3_5,  BDA_BCC_RATE_4_5,
        BDA_BCC_RATE_5_6,         BDA_BCC_RATE_5_11, BDA_BCC_RATE_7_8,
        BDA_BCC_RATE_NOT_DEFINED};
    /*
    if(ch->m_dwStructSize==sizeof(*ch))
    {
    pDVBSLocator->put_OuterFECRate(rates[ch->m_FEC]);
    if(ch->m_IsDVB_S2) pDVBSLocator->put_Modulation(BDA_MOD_8VSB);
    };
    */
    hr = pDVBSLocator->put_CarrierFrequency(m_CmdData.ChannelFrequency * 1000);
    hr = pDVBSLocator->put_SignalPolarisation(m_CmdData.Polarity
                                                  ? BDA_POLARISATION_LINEAR_V
                                                  : BDA_POLARISATION_LINEAR_H);
    hr = pDVBSLocator->put_SymbolRate(m_CmdData.SymbolRate);
  };
  hr = pDVBTuneRequest->put_Locator(pDVBSLocator);
  if (FAILED(hr)) {
    qDebug() << "Cannot put the locator";
    return;
  }
  hr = pNewTuneRequest.QueryInterface(&pTuneRequest);
  if (FAILED(hr)) {
    qDebug() << "Error on create pTuneRequest2";
    return;
  }
  // 2.
  CComQIPtr<ITuner> pTuner(provider);
  hr = pTuner->put_TuneRequest(pTuneRequest);
  //  ReleaseMutex(hIOMutex);
  control->Run();

  if (FAILED(hr)) {
    qDebug() << "KSPROPERTY_CTRL_TUNER set channel_change false";
  } else {
    qDebug() << "lock ok";
  }

  ReleaseMutex(hIOMutex);
}

void Hdmioptiongui::startBtn() {
  if (1 == IsStartFlg) {
    return;
  }
  KbCount = 0;
  MbCount = 0;
  GbCount = 0;
  SecCount = 0;
  HrCount = 0;
  MINCount = 0;
  if (0 == isSendTs) {
    MaxFileSize = (u32)(ui->lin_FileSize->text().toInt());
    TimeSize = (u32)(ui->lin_Timesize->text().toInt());
    QString Name = ui->lin_FilePath->text();
    if (control) {
      if (1 == IsOpenFileFlg) {
        return;
      }
      w_file_fd = fopen(Name.toLatin1().data(), "wb");
      if (NULL == w_file_fd) {
        qDebug("errno open file!!");
        IsOpenFileFlg = 0;
        return;
      } else {
        IsOpenFileFlg = 1;
      }
      if (m_pFilterDest) {
        m_pFilterDest->m_pPin->RefreshConnect();
      }
      control->Run();
      if (2 == IsSizeOrTimeFlg) {
        qtimer->start(1000);  // 1s
      } else if (0 == IsSizeOrTimeFlg) {
        qtimer->start(1000);  // 1s
      }
      qDebug() << "start record";
      IsStartFlg = 1;
    }
  } else {
    if (control) {
      if (m_pFilterDest) {
        m_pFilterDest->m_pPin->RefreshConnect();
      }
      control->Run();
      qDebug() << "start send Ts";
      IsStartFlg = 1;
    }
  }

  if (1 == isSendTs) {
    ui->showStreaming->setText("Start Streaming...");
    ui->rad_Encode->setEnabled(false);
    ui->rad_Recorder->setEnabled(false);
    ui->rad_Streaming->setEnabled(true);
  } else {
    ui->rad_Encode->setEnabled(false);
    ui->rad_Recorder->setEnabled(true);
    ui->rad_Streaming->setEnabled(false);
  }
  ui->btn_Start->setEnabled(false);
  ui->btn_Cancel->setEnabled(true);
  if (0 == IsSizeOrTimeFlg) {
    ui->No_limt->setEnabled(true);
    ui->Size_Limit->setEnabled(false);
    ui->Time_Limit->setEnabled(false);
  } else if (1 == IsSizeOrTimeFlg) {
    ui->No_limt->setEnabled(false);
    ui->Size_Limit->setEnabled(true);
    ui->Time_Limit->setEnabled(false);
  } else if (2 == IsSizeOrTimeFlg) {
    ui->No_limt->setEnabled(false);
    ui->Size_Limit->setEnabled(false);
    ui->Time_Limit->setEnabled(true);
  }
}

void Hdmioptiongui::stopBtn() {
  if (0 == IsStartFlg) {
    return;
  }
  if (0 == isSendTs) {
    if (control) {
      if (0 == IsOpenFileFlg) {
        return;
      } else {
        control->Stop();
        /****************/
        StopDevice();
        StartDevice(DL->Devices[col].deviceno);
        m_pTunerDevice = TunerFilter;
        m_pKsCtrl = NULL;
        m_pTunerPin = NULL;
        char input[7] = {'I', 'n', 'p', 'u', 't', '0', '\0'};
        m_pTunerPin = FindPinOnFilter(m_pTunerDevice, input);
        int ll_init = 0;
        HRESULT hr = S_OK;
        if (!ll_init && m_pTunerPin != NULL) {
          hr = m_pTunerPin->QueryInterface(
              IID_IKsPropertySet, reinterpret_cast<void **>(&m_pKsCtrl));
          if (FAILED(hr)) {
            ll_init = 1;
            qDebug("m_pTunerDevice QueryInterface Failed");
            m_pKsCtrl = NULL;
            return;
          }
        }
        m_pVCKsCtrl = NULL;
        lockSingle();
        /****************/
        if (2 == IsSizeOrTimeFlg) {
          qtimer->stop();
        } else if (0 == IsSizeOrTimeFlg) {
          qtimer->stop();
        }
        if (NULL != w_file_fd) {
          fclose(w_file_fd);
          w_file_fd = NULL;
        }
        IsOpenFileFlg = 0;

        IsStartFlg = 0;
        qDebug() << "stop record";
      }
    }
  } else {
    control->Stop();
    /****************/
    StopDevice();
    StartDevice(DL->Devices[col].deviceno);
    m_pTunerDevice = TunerFilter;
    m_pKsCtrl = NULL;
    m_pTunerPin = NULL;
    char input[7] = {'I', 'n', 'p', 'u', 't', '0', '\0'};
    m_pTunerPin = FindPinOnFilter(m_pTunerDevice, input);
    int ll_init = 0;
    HRESULT hr = S_OK;
    if (!ll_init && m_pTunerPin != NULL) {
      hr = m_pTunerPin->QueryInterface(IID_IKsPropertySet,
                                       reinterpret_cast<void **>(&m_pKsCtrl));
      if (FAILED(hr)) {
        ll_init = 1;
        qDebug("m_pTunerDevice QueryInterface Failed");
        m_pKsCtrl = NULL;
        return;
      }
    }
    m_pVCKsCtrl = NULL;
    lockSingle();
    /****************/
    IsStartFlg = 0;
    qDebug() << "stop send ts";
  }
  if (1 == isSendTs) {
    ui->showStreaming->setText("Stop Streaming...");
  }
  ui->rad_Encode->setEnabled(true);
  ui->rad_Recorder->setEnabled(true);
  ui->rad_Streaming->setEnabled(true);
  ui->btn_Start->setEnabled(true);
  ui->btn_Cancel->setEnabled(false);

  ui->No_limt->setEnabled(true);
  ui->Size_Limit->setEnabled(true);
  ui->Time_Limit->setEnabled(true);
}

void Hdmioptiongui::Init_RTP_OR_UDP() {
  string bind_ip;
  INT bind_port;
  string target_ip;
  INT target_port;
  INT mtu;
  BOOL is_rtp;
  vector<string> vIPList;
  if (Socket::GetLocalIPList(vIPList) == FALSE) {
    qDebug("No ip address!");
    return;
  }
  bind_ip = vIPList[0];
  bind_port = 0;
  // target_ip = "239.0.0.1";
  // target_ip = "192.168.8.100";

  target_ip = ui->lin_TarIp->text().toStdString();
  target_port = ui->lin_TarPort->text().toInt();
  mtu = 1472;
  if (1 == isSendTs) {
    if (0 == isRtp) {  // udp
      g_udp = new Udp(mtu);
      g_udp->Open(bind_ip, bind_port);
      g_udp->Connect(target_ip, target_port);
    } else {  // rtp
      g_rtp = new Rtp(mtu);
      g_rtp->Open(bind_ip, bind_port);
      g_rtp->Connect(target_ip, target_port);
    }
  }
}

void Hdmioptiongui::UnInit_RTP_OR_UDP() {
  if (1 == isSendTs) {
    if (0 == isRtp) {  // udp
      delete g_udp;
      g_udp = NULL;
    } else {  // rtp
      delete g_rtp;
      g_rtp = NULL;
    }
  }
}

void Hdmioptiongui::progressBarShow(int begin, int end, int progress) {
  if (0 == IsSizeOrTimeFlg) {  // no
    if (KbCount >= 1024) {
      KbCount = KbCount - 1024;
      MbCount++;
      if (MbCount >= 1024) {
        MbCount = MbCount - 1024;
        GbCount++;
      }
    }
    if (SecCount >= 60) {
      SecCount = SecCount - 60;
      MINCount++;
      if (MINCount >= 60) {
        MINCount = MINCount - 60;
        HrCount++;
      }
    }
    ui->lin_SizeCount->setText(
        QString("%1GB %2MB %3KB").arg(GbCount).arg(MbCount).arg(KbCount));
    ui->lin_TimeCount->setText(
        QString("%1H %2MIN %3S").arg(HrCount).arg(MINCount).arg(SecCount));
    return;
  } else if (1 == IsSizeOrTimeFlg) {  // size
    ui->lin_SizeAndTime->setText(QString("%1").arg(KbCount));
  } else if (2 == IsSizeOrTimeFlg) {  // time
    ui->lin_SizeAndTime->setText(QString("%1").arg(SecCount));
  }
  if (progress >= end) {
    progress = end;
    stopBtn();
  }
  ui->bar_Record->setRange(begin, end);
  ui->bar_Record->setValue(progress);

  return;
}

void Hdmioptiongui::show_status(Msg *g) {
  qDebug() << "show";
#if 1
  if (1 == g->type) {
    // test
    qbox = new QMessageBox();
    qbox->setWindowTitle(tr("information"));
    qbox->setText(QString("Device is configuring,please wait.."));
    qbox->setStandardButtons(0);
    qbox->setFont(QFont("Microsoft Yahei", fontsize));
    qbox->setProperty("Form", true);
    qbox->exec();
    return;
  }
  if (2 == g->type) {
    qbox->close();
    delete qbox;
    return;
  }
  if (4 == g->type) {
    if (QMessageBox::Ok ==
        QMessageBox::information(
            this, tr("information"),
            tr("Success                                    "), QMessageBox::Ok,
            QMessageBox::Ok)) {
      // if (1 == loadflg) {
      readBuffer();
      loadflg = 0;
      //}
      return;
    }
    return;
  }
#endif
}

void Hdmioptiongui::timeServiceFunc() {
  ++SecCount;
  if (2 == IsSizeOrTimeFlg) {  // no
    int timebar = (int)((SecCount * 100) / (TimeSize - 0));
    progressBarShow(0, 100, timebar);
  } else if (0 == IsSizeOrTimeFlg) {
    progressBarShow(0, 100, 0);
  }
}

void Hdmioptiongui::on_btnMenu_Min_clicked() { showMinimized(); }

void Hdmioptiongui::on_btnMenu_Max_clicked() {
  static bool max = false;
  static QRect location = this->geometry();

  if (max) {
    this->setGeometry(location);
    width = width_s;
    height = height_s;
  } else {
    location = this->geometry();
    this->setGeometry(qApp->desktop()->availableGeometry());
    width = this->size().width();
    height = this->size().height();
  }

  this->setProperty("canMove", max);
  max = !max;
}

void Hdmioptiongui::on_btnMenu_Close_clicked() { close(); }

void Hdmioptiongui::on_tabWidget_currentChanged(int idx) {
  if (0 == IsStartFlg) {
    IsSizeOrTimeFlg = idx;  // 0,no limt ;1, size;2 ,timer
    if (0 == idx) {
      ui->gbox_record->setEnabled(false);
      ui->lab_FileName->setText(QString(""));
      ui->lab_FileUnit->setText(QString(""));
      ui->lin_SizeAndTime->setText(QString(""));
      ui->bar_Record->setValue(0);
    } else if (1 == idx) {
      ui->gbox_record->setEnabled(true);
      ui->lab_FileName->setText(QString("Record File Size:"));
      ui->lab_FileUnit->setText(QString("KB"));
      ui->lin_SizeAndTime->setText(QString(""));
      ui->bar_Record->setValue(0);
    } else if (2 == idx) {
      ui->gbox_record->setEnabled(true);
      ui->lab_FileName->setText(QString("Record Time:"));
      ui->lab_FileUnit->setText(QString("S"));
      ui->lin_SizeAndTime->setText(QString(""));
      ui->bar_Record->setValue(0);
    }
  }
}

void Hdmioptiongui::on_com_Aout_currentIndexChanged(int idx) {
  if (0 == idx) {  // hdmi
    ui->com_AFre->clear();
    ui->com_AFre->addItem("48k", 0);
    ui->com_AFre->addItem("44.1k", 1);
    ui->com_AFre->addItem("32k", 2);
    ui->com_AFre->addItem("Auto", 3);
  } else if (1 == idx) {  // audio
    ui->com_AFre->clear();
    ui->com_AFre->addItem("48k", 0);
    ui->com_AFre->addItem("44.1k", 1);
    ui->com_AFre->addItem("32k", 2);

  } else {
    ui->com_AFre->clear();
  }
}

void Hdmioptiongui::test() {
  // tbs.wrReg32_Extern(0x00001234,0x55667788);
  // u32 tmp = tbs.rdReg32_Extern(0x00001234);
  // u32 tmp = tbs.rdReg32_Extern(0x20);
  // u32 tmp = tbs.rdReg32_Extern(0x4000);
  // u32 tmp = tbs.rdReg32(0x20);
  // qDebug("test:tmp=%#x",tmp);
}

int Hdmioptiongui::DeviceGetList() {
  TDSEnum *penumBDATuners = enum_create(KSCATEGORY_BDA_NETWORK_TUNER);
  IBaseFilter *pFilter = NULL;
  ZeroMemory(DL, sizeof(TDevAPIDevList));
  int i = 0;
  int j = 0;
  int xnum = 0;
  int cnum = 0;
  int DLCount = 0;
  char bf[32];
  memset(bf, '0', 32);
  char **bfch = new char *[16];
  for (i = 0; i < 16; i++) {
    bfch[i] = NULL;
  }
  i = 0;
  while (enum_next(penumBDATuners) == S_OK) {
    enum_get_name(penumBDATuners);
    qDebug("DEVICE:%s", penumBDATuners->szName);
    // pick out hdmi capture device
    if (strstr(penumBDATuners->szName, "TBS 6308 HDMI Capture 0") > 0) {
      strcpy(DL->Devices[0 + 8 * xnum].szName, penumBDATuners->szName);
      DL->Devices[0 + 8 * xnum].TunerType = 0;
      DL->Devices[0 + 8 * xnum].deviceno = cnum;  // actually identifier num
      DL->Count++;
    } else if (strstr(penumBDATuners->szName, "TBS 6308 HDMI Capture 1") > 0) {
      strcpy(DL->Devices[1 + 8 * xnum].szName, penumBDATuners->szName);
      DL->Devices[1 + 8 * xnum].TunerType = 1;
      DL->Devices[1 + 8 * xnum].deviceno = cnum;  // actually identifier num
      DL->Count++;
    } else if (strstr(penumBDATuners->szName, "TBS 6308 HDMI Capture 2") > 0) {
      strcpy(DL->Devices[2 + 8 * xnum].szName, penumBDATuners->szName);
      DL->Devices[2 + 8 * xnum].TunerType = 2;
      DL->Devices[2 + 8 * xnum].deviceno = cnum;  // actually identifier num
      DL->Count++;
    } else if (strstr(penumBDATuners->szName, "TBS 6308 HDMI Capture 3") > 0) {
      strcpy(DL->Devices[3 + 8 * xnum].szName, penumBDATuners->szName);
      DL->Devices[3 + 8 * xnum].TunerType = 3;
      DL->Devices[3 + 8 * xnum].deviceno = cnum;  // actually identifier num
      DL->Count++;
    } else if (strstr(penumBDATuners->szName, "TBS 6308 HDMI Capture 4") > 0) {
      strcpy(DL->Devices[4 + 8 * xnum].szName, penumBDATuners->szName);
      DL->Devices[4 + 8 * xnum].TunerType = 4;
      DL->Devices[4 + 8 * xnum].deviceno = cnum;  // actually identifier num
      DL->Count++;
    } else if (strstr(penumBDATuners->szName, "TBS 6308 HDMI Capture 5") > 0) {
      strcpy(DL->Devices[5 + 8 * xnum].szName, penumBDATuners->szName);
      DL->Devices[5 + 8 * xnum].TunerType = 5;
      DL->Devices[5 + 8 * xnum].deviceno = cnum;  // actually identifier num
      DL->Count++;
    } else if (strstr(penumBDATuners->szName, "TBS 6308 HDMI Capture 6") > 0) {
      strcpy(DL->Devices[6 + 8 * xnum].szName, penumBDATuners->szName);
      DL->Devices[6 + 8 * xnum].TunerType = 6;
      DL->Devices[6 + 8 * xnum].deviceno = cnum;  // actually identifier num
      DL->Count++;
    } else if (strstr(penumBDATuners->szName, "TBS 6308 HDMI Capture 7") > 0) {
      strcpy(DL->Devices[7 + 8 * xnum].szName, penumBDATuners->szName);
      DL->Devices[7 + 8 * xnum].TunerType = 7;
      DL->Devices[7 + 8 * xnum].deviceno = cnum;  // actually identifier num
      DL->Count++;
    }
    if (((DL->Count - DLCount) % 8 == 0) && (0 != (DL->Count - DLCount))) {
      xnum++;
    }
    // qDebug("cnum:%d", cnum);

    cnum++;
  }
  for (; i < DL->Count; i++) {
    // qDebug() << DL->Devices[i].szName;
    if ((i + 1) % 8 == 0) {
      sprintf(bf, "TBS6304(8 tuner)");
      for (j = 0; j < 8; j++) {
        bfch[j] = new char[16];
        sprintf(bfch[j], "Capture%d", j);
      }
      addTreeIteml(bf, bfch, &DL->Devices[i - 7]);
      for (j = 0; j < 8; j++) {
        delete bfch[j];
        bfch[j] = NULL;
      }
    }
  }
  xnum = 0;
  cnum = 0;
  DLCount = DL->Count;
  enum_free(penumBDATuners);
  penumBDATuners = enum_create(KSCATEGORY_BDA_NETWORK_TUNER);
  while (enum_next(penumBDATuners) == S_OK) {
    enum_get_name(penumBDATuners);
    // qDebug("DEVICE:%s", penumBDATuners->szName);
    // pick out hdmi capture device
    if (strstr(penumBDATuners->szName, "TBS 6304 HDMI Capture 0") > 0) {
      strcpy(DL->Devices[DLCount + 0 + 4 * xnum].szName,
             penumBDATuners->szName);
      DL->Devices[DLCount + 0 + 4 * xnum].TunerType = 0;
      DL->Devices[DLCount + 0 + 4 * xnum].deviceno =
          cnum;  // actually identifier num
      DL->Count++;
    } else if (strstr(penumBDATuners->szName, "TBS 6304 HDMI Capture 1") > 0) {
      strcpy(DL->Devices[DLCount + 1 + 4 * xnum].szName,
             penumBDATuners->szName);
      DL->Devices[DLCount + 1 + 4 * xnum].TunerType = 1;
      DL->Devices[DLCount + 1 + 4 * xnum].deviceno =
          cnum;  // actually identifier num
      DL->Count++;
    } else if (strstr(penumBDATuners->szName, "TBS 6304 HDMI Capture 2") > 0) {
      strcpy(DL->Devices[DLCount + 2 + 4 * xnum].szName,
             penumBDATuners->szName);
      DL->Devices[DLCount + 2 + 4 * xnum].TunerType = 2;
      DL->Devices[DLCount + 2 + 4 * xnum].deviceno =
          cnum;  // actually identifier num
      DL->Count++;
    } else if (strstr(penumBDATuners->szName, "TBS 6304 HDMI Capture 3") > 0) {
      strcpy(DL->Devices[DLCount + 3 + 4 * xnum].szName,
             penumBDATuners->szName);
      DL->Devices[DLCount + 3 + 4 * xnum].TunerType = 3;
      DL->Devices[DLCount + 3 + 4 * xnum].deviceno =
          cnum;  // actually identifier num
      DL->Count++;
    }
    if (((DL->Count - DLCount) % 4 == 0) && (0 != (DL->Count - DLCount))) {
      xnum++;
    }
    cnum++;
  }
  for (; i < DL->Count; i++) {
    // qDebug() << DL->Devices[i].szName;
    if ((i + 1) % 4 == 0) {
      sprintf(bf, "TBS6304(4 tuner)");
      for (j = 0; j < 4; j++) {
        bfch[j] = new char[16];
        sprintf(bfch[j], "Capture%d", j);
      }
      addTreeIteml(bf, bfch, &DL->Devices[i - 3]);
      for (j = 0; j < 4; j++) {
        delete bfch[j];
        bfch[j] = NULL;
      }
    }
  }
  xnum = 0;
  cnum = 0;
  DLCount = DL->Count;
  enum_free(penumBDATuners);
  penumBDATuners = enum_create(KSCATEGORY_BDA_NETWORK_TUNER);
  while (enum_next(penumBDATuners) == S_OK) {
    enum_get_name(penumBDATuners);
    // qDebug() << QString(penumBDATuners->szName);
    // pick out hdmi capture device
    if (strstr(penumBDATuners->szName, "TBS 5301 USB HDMI Capture") > 0) {
      strcpy(DL->Devices[DLCount + 0 + 1 * xnum].szName,
             penumBDATuners->szName);
      DL->Devices[DLCount + 0 + 1 * xnum].TunerType = 0;
      DL->Devices[DLCount + 0 + 1 * xnum].deviceno =
          cnum;  // actually identifier num
      DL->Count++;
    }
    if (((DL->Count - DLCount) % 1 == 0) && (0 != (DL->Count - DLCount))) {
      xnum++;
    }
    cnum++;
  }
  for (; i < DL->Count; i++) {
    // qDebug() << DL->Devices[i].szName;
    sprintf(bf, "TBS5301");
    bfch[0] = new char[16];
    sprintf(bfch[0], "Capture0");
    addTreeIteml(bf, bfch, &DL->Devices[i]);
    delete bfch[0];
    bfch[0] = NULL;
  }
  enum_free(penumBDATuners);
  qDebug("DL->Count:%d", DL->Count);
  return 0;
}

int Hdmioptiongui::StartDevice(int dwDeviceNo) {
  wchar_t TunerName[256];
  DWORD type_support = 0;
  int TunerType;
  HRESULT hr;

  if (dwDeviceNo == -1) {
    return FALSE;
  }
  CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER,
                   IID_IGraphBuilder, (void **)&builder);
  builder->QueryInterface(IID_IMediaControl, (void **)&control);
  // Create filters
  provider = CreateDVBSNetworkProvider();

  if (provider) {
    builder->AddFilter(provider, L"Network DVBS Provider");
  } else {
    qDebug("CreateDVBSNetworkProvider Failed");
    return FALSE;
  }

  if (FAILED(create_tuner(builder, dwDeviceNo, &TunerFilter, &TunerName[0],
                          &TunerType))) {
    qDebug("CreateTunerDevice Failed");
  }

  // check the device type than create capture,ir and set mac address

  // char *strName;
  // strName = g_strDevName.GetBuffer(g_strDevName.GetLength());
  // g_strDevName.ReleaseBuffer();

  if (FAILED(ConnectFilters(builder, provider, TunerFilter))) {
    BDACleanUp();
    return 6;
  }

  if
    FAILED(create_captureFilter(builder, TunerFilter, &CaptureFilter)) {
      CaptureFilter = NULL;
      BDACleanUp();
      return 7;
    }
  demux = CreateMPEG2Demultiplexer();
  if (demux) {
    builder->AddFilter(demux, L"MPEG2 Demultiplexer");
  } else {
    qDebug("CreateMPEG2Demultiplexer Failed");
    return 8;
  }

  infinite = CreateInfinitePinTee();
  if (infinite) {
    builder->AddFilter(infinite, L"Infinite pin tee splitter");
  } else {
    qDebug("CreateInfinitePinTee Failed");
    return 9;
  }

  tif = CreateTransportInformationFilter();
  if (tif) {
    builder->AddFilter(tif, L"transport information filter");
  } else {
    qDebug("CreateInfinitePinTee Failed");
    return 10;
  }
  if (NULL == m_pFilterDest) {
    m_pFilterDest = new CDestFilter(0, &m_Lock, &hr);
  } else {
    delete m_pFilterDest;
    m_pFilterDest = NULL;
    m_pFilterDest = new CDestFilter(0, &m_Lock, &hr);
  }

  m_pFilterDest->AddRef();
  hr = builder->AddFilter((IBaseFilter *)m_pFilterDest, L"Dest Filter");

  // ConnectPins(builder,provider,TunerFilter);
  ConnectPins(builder, TunerFilter, CaptureFilter);
  ConnectPins(builder, CaptureFilter, infinite);
  ConnectPins(builder, infinite, demux);
  ConnectPins(builder, infinite, (IBaseFilter *)m_pFilterDest);
  ConnectPins(builder, demux, tif);

  return 0;
}
int Hdmioptiongui::StopDevice() {
  // TODO: Add your specialized code here and/or call the base class

  if (control != NULL) {
    control->Stop();
  }

  if (m_pKsCtrl) {
    WaitForSingleObject(hIOMutex, INFINITE);
    m_pKsCtrl->Release();
    m_pKsCtrl = NULL;
    ReleaseMutex(hIOMutex);
  }
  if (CaptureFilter != NULL) {
    builder->RemoveFilter(CaptureFilter);
  }
  if (TSDump != NULL) {
    builder->RemoveFilter(TSDump);
  }
  if (provider != NULL) {
    builder->RemoveFilter(provider);
  }
  if (control != NULL) {
    control.Release();
    control = NULL;
  }
  if (CaptureFilter != NULL) {
    CaptureFilter.Release();
    CaptureFilter = NULL;
  }
  if (TSDump != NULL) {
    TSDump.Release();
    TSDump = NULL;
  }
  if (demux != NULL) {
    demux.Release();
    demux = NULL;
  }
  if (infinite != NULL) {
    infinite.Release();
    infinite = NULL;
  }
  if (tif != NULL) {
    tif.Release();
    tif = NULL;
  }
  if (provider != NULL) {
    provider.Release();
    provider = NULL;
  }

  if (TunerFilter) {
    builder->RemoveFilter(TunerFilter);
    TunerFilter.Release();
  }

  if (builder != NULL) {
    builder.Release();
    builder = NULL;
  }

  return 0;
}

IBaseFilter *Hdmioptiongui::CreateTransportInformationFilter(void) {
  return FindFilter(KSCATEGORY_BDA_TRANSPORT_INFORMATION, NULL);
}

// Finds a filter based on the name
IBaseFilter *Hdmioptiongui::FindFilter(CLSID clsid, WCHAR *name) {
  HRESULT hr;
  CComPtr<IMoniker> pIMoniker;
  CComPtr<IEnumMoniker> pIEnumMoniker;
  CComPtr<ICreateDevEnum> m_pICreateDevEnum;
  int nl;

  if (name != NULL) nl = wcslen(name);
  if (!m_pICreateDevEnum) {
    hr = m_pICreateDevEnum.CoCreateInstance(CLSID_SystemDeviceEnum);
    if (FAILED(hr)) return NULL;
  }

  hr = m_pICreateDevEnum->CreateClassEnumerator(clsid, &pIEnumMoniker, 0);
  if (FAILED(hr) || (S_OK != hr)) return NULL;

  while (pIEnumMoniker->Next(1, &pIMoniker, 0) == S_OK) {
    CComPtr<IPropertyBag> pBag;
    hr = pIMoniker->BindToStorage(NULL, NULL, IID_IPropertyBag,
                                  reinterpret_cast<void **>(&pBag));
    if (FAILED(hr)) {
      qDebug("FindFilter(): Cannot BindToStorage");
      return NULL;
    }

    CComVariant varBSTR;
    hr = pBag->Read(L"FriendlyName", &varBSTR, NULL);
    if (FAILED(hr)) {
      qDebug("FindFilter(): IPropertyBag->Read method failed");
      pIMoniker = NULL;
      continue;
    }

    // bind the filter
    IBaseFilter *pFilter;
    hr = pIMoniker->BindToObject(NULL, NULL, IID_IBaseFilter,
                                 reinterpret_cast<void **>(&pFilter));
    if (FAILED(hr)) {
      pIMoniker = NULL;
      pFilter = NULL;
      continue;
    }

    // if((name==NULL)||(memcmp(varBSTR.bstrVal,name,nl)==0))
    // return pFilter;

    //      if((name==NULL)||(memcmp(varBSTR.bstrVal,name,nl)==0))
    if ((name == NULL) || wcscmp((const wchar_t *)varBSTR.bstrVal, name) == 0) {
      pIMoniker.Release();
      pIEnumMoniker.Release();
      m_pICreateDevEnum.Release();

      return pFilter;
    }

    pIMoniker = NULL;
    pFilter = NULL;
  }
  return NULL;
}
IBaseFilter *Hdmioptiongui::CreateMPEG2Demultiplexer(void) {
  return LoadFilter(CLSID_MPEG2Demultiplexer);
}

IBaseFilter *Hdmioptiongui::CreateInfinitePinTee(void) {
  return LoadFilter(CLSID_InfTee);
}
// Load a filter for CLSID
IBaseFilter *Hdmioptiongui::LoadFilter(CLSID clsid) {
  IBaseFilter *filter;

  CoCreateInstance(clsid, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter,
                   reinterpret_cast<void **>(&filter));
  return filter;
}

HRESULT Hdmioptiongui::ConnectFilters(IGraphBuilder *pGraph,
                                      IBaseFilter *pFilterUpstream,
                                      IBaseFilter *pFilterDownstream)

{
  HRESULT hr = E_FAIL;

  IPin *pIPinUpstream;

  PIN_INFO PinInfoUpstream;
  PIN_INFO PinInfoDownstream;

  // grab upstream filter's enumerator
  IEnumPins *pIEnumPinsUpstream;
  hr = pFilterUpstream->EnumPins(&pIEnumPinsUpstream);

  if (FAILED(hr)) {
    return hr;
  }

  // iterate through upstream filter's pins
  while (pIEnumPinsUpstream->Next(1, &pIPinUpstream, 0) == S_OK) {
    hr = pIPinUpstream->QueryPinInfo(&PinInfoUpstream);
    if (FAILED(hr)) {
      return hr;
    }

    IPin *pPinDown;
    pIPinUpstream->ConnectedTo(&pPinDown);

    // bail if pins are connected
    // otherwise check direction and connect
    if ((PINDIR_OUTPUT == PinInfoUpstream.dir) && (pPinDown == NULL)) {
      // grab downstream filter's enumerator
      IEnumPins *pIEnumPinsDownstream;
      hr = pFilterDownstream->EnumPins(&pIEnumPinsDownstream);
      if (FAILED(hr)) {
        return hr;
      }

      // iterate through downstream filter's pins
      IPin *pIPinDownstream;
      while (pIEnumPinsDownstream->Next(1, &pIPinDownstream, 0) == S_OK) {
        // make sure it is an input pin
        hr = pIPinDownstream->QueryPinInfo(&PinInfoDownstream);
        if (SUCCEEDED(hr)) {
          IPin *pPinUp;

          // Determine if the pin is already connected.  Note that
          // VFW_E_NOT_CONNECTED is expected if the pin isn't yet connected.
          hr = pIPinDownstream->ConnectedTo(&pPinUp);
          if (FAILED(hr) && hr != VFW_E_NOT_CONNECTED) {
            continue;
          }

          if ((PINDIR_INPUT == PinInfoDownstream.dir) && (pPinUp == NULL)) {
            if (SUCCEEDED(pGraph->Connect(pIPinUpstream, pIPinDownstream))) {
              PinInfoDownstream.pFilter->Release();
              PinInfoUpstream.pFilter->Release();
              return S_OK;
            }
          }
        }

        PinInfoDownstream.pFilter->Release();
        pIPinDownstream = NULL;
      }  // while next downstream filter pin

      // We are now back into the upstream pin loop
    }  // if output pin

    pIPinUpstream = NULL;
    PinInfoUpstream.pFilter->Release();
  }  // while next upstream filter pin

  return E_FAIL;
}

HRESULT Hdmioptiongui::create_captureFilter(IGraphBuilder *pfltgraph,
                                            IBaseFilter *pftuner,
                                            IBaseFilter **pfilter) {
  *pfilter = NULL;

  TDSEnum *penumBDACapture = enum_create(KSCATEGORY_BDA_RECEIVER_COMPONENT);
  if (!penumBDACapture) return E_FAIL;

  while (enum_next(penumBDACapture) == S_OK)
    if (enum_get_filter(penumBDACapture, pfilter) == S_OK) {
      //      IBaseFilter *pf;
      //(*pfilter)->QueryInterface(IID_IBaseFilter, (void **)&pf);
      add_filter(pfltgraph, *pfilter);

      HRESULT hr;

      __try {
        hr = ConnectFilters(pfltgraph, pftuner, *pfilter);
      } __except (EXCEPTION_EXECUTE_HANDLER) {
        hr = E_FAIL;
      }

      if
        SUCCEEDED(hr) {
          enum_free(penumBDACapture);
          return S_OK;
        }
      else {
        (*pfilter)->Release();
        *pfilter = NULL;
        continue;
      }
      remove_filter(pfltgraph, *pfilter);
      // pf->Release();
      (*pfilter)->Release();
    }
  enum_free(penumBDACapture);
  return E_FAIL;
}
HRESULT Hdmioptiongui::add_filter(IGraphBuilder *pfltgraph,
                                  IBaseFilter *pfilter) {
  HRESULT hr;

  if (FAILED(hr = pfltgraph->AddFilter(pfilter, NULL))) {
    qDebug("FAILED: Cannot add the filter to the graph");
  }
  return hr;
}
HRESULT Hdmioptiongui::remove_filter(IGraphBuilder *pfltgraph,
                                     IBaseFilter *pfilter) {
  HRESULT hr;

  if (FAILED(hr = pfltgraph->RemoveFilter(pfilter))) {
    qDebug("FAILED: Cannot remove the filter from the graph");
  }
  return hr;
}

int Hdmioptiongui::BDACleanUp() {
  if (control != NULL) {
    control->Stop();
  }
  if (m_pKsCtrl) {
    WaitForSingleObject(hIOMutex, INFINITE);

    m_pKsCtrl->Release();
    m_pKsCtrl = NULL;

    ReleaseMutex(hIOMutex);
  }

  if (provider != NULL) {
    builder->RemoveFilter(provider);
  }

  if (control != NULL) {
    control.Release();
    control = NULL;
  }

  if (provider != NULL) {
    provider.Release();
    provider = NULL;
  }

  if (TunerFilter) {
    builder->RemoveFilter(TunerFilter);
    TunerFilter.Release();
  }

  if (builder != NULL) {
    builder.Release();
    builder = NULL;
  }
  return 0;
}
HRESULT Hdmioptiongui::create_tuner(IGraphBuilder *pfltgraph, int no,
                                    IBaseFilter **pfilter, wchar_t *TunerName,
                                    int *tuner_type) {
  int i = 0;
  *pfilter = NULL;

  TDSEnum *penumBDATuners = enum_create(KSCATEGORY_BDA_NETWORK_TUNER);
  if (!penumBDATuners) return E_FAIL;

  while (enum_next(penumBDATuners) == S_OK) {
    if (i == no) {
      if (enum_get_filter(penumBDATuners, pfilter) == S_OK) {
        enum_get_name(penumBDATuners);
        // get_tuner_type(*pfilter, tuner_type);
        *tuner_type = 0x00000001;  // dvbS
        // wcscpy_s(wstunername, 255, penumBDATuners->szNameW);
        wcscpy(TunerName, penumBDATuners->szNameW);
        enum_free(penumBDATuners);
        return (pfltgraph->AddFilter(*pfilter, NULL));
      }
    }
    i++;
  }
  enum_free(penumBDATuners);
  return E_FAIL;
}
IBaseFilter *Hdmioptiongui::CreateDVBSNetworkProvider() {
  BSTR name;
  CLSID clsid;
  IBaseFilter *provider;
  tuningspace = LoadDVBSTuningSpace();
  // CComQIPtr<IDVBSTuningSpace> pDVBSTuningSpace(tuningspace) ;
  tuningspace->get_NetworkType(&name);
  CLSIDFromString(name, &clsid);
  SysFreeString(name);
  CoCreateInstance(clsid, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter,
                   reinterpret_cast<void **>(&provider));
  {
    CComPtr<ITuner> tuner;
    CComPtr<ITuneRequest> request;

    tuningspace->CreateTuneRequest(&request);
    provider->QueryInterface(IID_ITuner, (void **)&tuner);
    tuner->put_TuningSpace(tuningspace);
    tuner->put_TuneRequest(request);

    //
  }
  return provider;
}

IDVBTuningSpace2 *Hdmioptiongui::LoadDVBSTuningSpace(void) {
  IEnumTuningSpaces *spaces;
  CComPtr<ITuningSpaceContainer> pITuningSpaceContainer;
  HRESULT hr;
  ITuningSpace *space;
  ULONG l;
  BSTR name;
  BOOL bFound;

  hr = pITuningSpaceContainer.CoCreateInstance(CLSID_SystemTuningSpaces);
  if (FAILED(hr)) return NULL;
  // Check if our tuning space is in the container, and add it if not
  pITuningSpaceContainer->get_EnumTuningSpaces(&spaces);
  spaces->Reset();
  bFound = FALSE;
  while ((!bFound) && (spaces->Next(1, &space, &l) == S_OK)) {
    space->get_UniqueName(&name);
    if (wcscmp(name, DVBS_TUNING_SPACE_NAME) == 0)
      bFound = TRUE;
    else
      space->Release();
    SysFreeString(name);
  }
  spaces->Release();
  if (!bFound) {
    VARIANT v;

    space = CreateDVBSTuningSpace();
    pITuningSpaceContainer->Add(space, &v);
  }
  return (IDVBTuningSpace2 *)space;
}

IDVBTuningSpace2 *Hdmioptiongui::CreateDVBSTuningSpace(void) {
  IDVBSLocator *locator;
  IDVBTuningSpace2 *tuning;

  // Create locator
  CoCreateInstance(CLSID_DVBSLocator, NULL, CLSCTX_INPROC_SERVER,
                   IID_IDVBSLocator, (void **)&locator);

  // Create tuning space
  CoCreateInstance(CLSID_DVBSTuningSpace, NULL, CLSCTX_INPROC_SERVER,
                   IID_IDVBTuningSpace2, (void **)&tuning);
  // Set ITuningSpace variables
  tuning->put__NetworkType(CLSID_DVBSNetworkProvider);
  // Set IDVBTuningSpace variables
  tuning->put_SystemType(DVB_Satellite);
  // Set IDVBTuningSpace2 variables
  //  tuning->put_NetworkID(9018);
  // Set ITuningSpace variables again
  tuning->put_DefaultLocator(locator);
  tuning->put_FrequencyMapping(_bstr_t(""));
  tuning->put_FriendlyName(_bstr_t("Local DVBS Tuner"));
  tuning->put_UniqueName(DVBS_TUNING_SPACE_NAME);
  CComQIPtr<IDVBSTuningSpace> pDVBSTuningSpace(tuning);
  if (pDVBSTuningSpace != NULL) {
    pDVBSTuningSpace->put_HighOscillator(
        -1);  // Sets the high oscillator frequency.
    pDVBSTuningSpace->put_InputRange(
        CComBSTR("-1"));  // Sets an integer indicating which option or switch
                          // contains the requested signal source.
    pDVBSTuningSpace->put_LNBSwitch(-1);  // Sets the LNB switch frequency.
    pDVBSTuningSpace->put_LowOscillator(
        -1);  // Sets the low oscillator frequency.
    pDVBSTuningSpace->put_SpectralInversion(
        BDA_SPECTRAL_INVERSION_NOT_SET);  // Sets an integer indicating the
                                          // spectral inversion.
  }
  // Done
  return tuning;
}

void Hdmioptiongui::on_btn_Browse_clicked() {
  QString file_name = QFileDialog::getSaveFileName(this, tr("Save File"), "",
                                                   tr("TS file(*.ts)"), 0);
  if (!file_name.isNull()) {
    ui->lin_FilePath->setText(file_name);
  }
}

void Hdmioptiongui::on_btn_Start_clicked() {
  BOOL emptyFlg = 0;
  QString qstr = QString("");
  QString pathF = ui->lin_FilePath->text();
  QString sizeF = ui->lin_FileSize->text();
  QString timeF = ui->lin_Timesize->text();
  QString tarIp = ui->lin_TarIp->text();
  QString tarPort = ui->lin_TarPort->text();

  if (1 == isSendTs) {
    if (tarIp.isEmpty() || tarPort.isEmpty()) {
      emptyFlg = 1;
      qstr = QString("Please input the Target ip or port                 ");
    }
  } else {
    if (pathF.isEmpty()) {
      emptyFlg = 1;
      qstr = QString("Please input the file path                     ");
    }
    if (1 == IsSizeOrTimeFlg) {
      if (sizeF.isEmpty()) {
        emptyFlg = 1;
        qstr = QString("Please input the file total size                   ");
      }
    } else if (2 == IsSizeOrTimeFlg) {
      if (timeF.isEmpty()) {
        emptyFlg = 1;
        qstr = QString("Please input the total time                      ");
      }
    }
  }
  if (1 == emptyFlg) {
    if (QMessageBox::Ok ==
        QMessageBox::warning(this, tr("warn"), tr(qstr.toLatin1().data()),
                             QMessageBox::Ok, QMessageBox::Ok)) {
      return;
    }
    return;
  }

  if (1 == noSingleFlg) {
    if (QMessageBox::Ok ==
        QMessageBox::warning(
            this, tr("warn"),
            tr("No input source                                 "),
            QMessageBox::Ok, QMessageBox::Ok)) {
      return;
    }
    return;
  }
  if (tunerflg != 1) {
    if (QMessageBox::Ok ==
        QMessageBox::warning(this, tr("warn"),
                             tr("Device not selected,or open fail"),
                             QMessageBox::Ok, QMessageBox::Ok)) {
      return;
    }
    return;
  }
  if (mode != 0) {
    if (QMessageBox::Ok ==
        QMessageBox::warning(this, tr("warn"),
                             tr("Device is busy,please wait..    "),
                             QMessageBox::Ok, QMessageBox::Ok)) {
      return;
    }
    return;
  }
  /*
      if (QMessageBox::No == QMessageBox::question(this, tr("question"),
          tr("Start?                                  "),
          QMessageBox::No | QMessageBox::Yes,
          QMessageBox::No)) {
          return;
      }
      */
  if (0 == IsStartFlg) {
    isRtp = ui->com_Net->currentIndex();
    qDebug() << "isrtp:" << isRtp;
  }
  writeConfig();
  Init_RTP_OR_UDP();
  startBtn();
}

void Hdmioptiongui::on_btn_Cancel_clicked() {
  BOOL emptyFlg = 0;
  QString qstr = QString("");
  QString pathF = ui->lin_FilePath->text();
  QString sizeF = ui->lin_FileSize->text();
  QString timeF = ui->lin_Timesize->text();
  QString tarIp = ui->lin_TarIp->text();
  QString tarPort = ui->lin_TarPort->text();

  if (1 == isSendTs) {
    if (tarIp.isEmpty() || tarPort.isEmpty()) {
      emptyFlg = 1;
      qstr = QString("Please input the Target ip or port                 ");
    }
  } else {
    if (pathF.isEmpty()) {
      emptyFlg = 1;
      qstr = QString("Please input the file path                     ");
    }
    if (1 == IsSizeOrTimeFlg) {
      if (sizeF.isEmpty()) {
        emptyFlg = 1;
        qstr = QString("Please input the file total size                   ");
      }
    } else if (2 == IsSizeOrTimeFlg) {
      if (timeF.isEmpty()) {
        emptyFlg = 1;
        qstr = QString("Please input the total time                      ");
      }
    }
  }
  if (1 == emptyFlg) {
    if (QMessageBox::Ok ==
        QMessageBox::warning(this, tr("warn"), tr(qstr.toLatin1().data()),
                             QMessageBox::Ok, QMessageBox::Ok)) {
      return;
    }
    return;
  }

  if (1 == noSingleFlg) {
    if (QMessageBox::Ok ==
        QMessageBox::warning(
            this, tr("warn"),
            tr("No input source                                 "),
            QMessageBox::Ok, QMessageBox::Ok)) {
      return;
    }
    return;
  }
  if (tunerflg != 1) {
    if (QMessageBox::Ok ==
        QMessageBox::warning(this, tr("warn"),
                             tr("Device not selected,or open fail"),
                             QMessageBox::Ok, QMessageBox::Ok)) {
      return;
    }
    return;
  }
  if (mode != 0) {
    if (QMessageBox::Ok ==
        QMessageBox::warning(this, tr("warn"),
                             tr("Device is busy,please wait..    "),
                             QMessageBox::Ok, QMessageBox::Ok)) {
      return;
    }
    return;
  }
  /*if (QMessageBox::No == QMessageBox::question(this, tr("question"),
      tr("Stop?                                   "),
      QMessageBox::No | QMessageBox::Yes,
      QMessageBox::No)) {
      return;
  }*/
  stopBtn();
  UnInit_RTP_OR_UDP();
}

void Hdmioptiongui::on_btn_Run_clicked() {
  if (tunerflg != 1) {
    if (QMessageBox::Ok ==
        QMessageBox::warning(this, tr("warn"),
                             tr("Device not selected,or open fail"),
                             QMessageBox::Ok, QMessageBox::Ok)) {
      return;
    }
    return;
  }
  if (mode != 0) {
    if (QMessageBox::Ok ==
        QMessageBox::warning(this, tr("warn"),
                             tr("Device is busy,please wait..    "),
                             QMessageBox::Ok, QMessageBox::Ok)) {
      return;
    }
    return;
  }

  // get ui vaule
  memset(Pname, '\0', sizeof(Pname));
  QString str = GetProgrameName;
  QByteArray ba = str.toLatin1();
  char *ch = ba.data();
  int i = 0;
  while (ch[i] != '\0') {
    Pname[i] = (unsigned char)(ch[i]);
    i++;
  }
  int level[6] = {30, 31, 32, 40, 41, 42};
  PNo = GetProgrameNo;
  Vpid = GetVpid;
  Apid = GetApid;
  Pmt = GetPmt;
  Pcr = GetPcr;
  HpFile = GetHprofile;
  HpLevel = level[GetHlevel];
  PixW = GetW;
  PixH = GetH;
  BitRate = ui->lin_Sysbr->text().toInt();
  epkt = (u8)ui->com_Epkt->currentIndex();
  Frame = ui->com_EcFr->currentText().toInt();
  igvr = ui->lin_Imgvr->text().toFloat();
  qDebug("w igvr:%f",igvr);
  MuteEn = GetMute;
  if (5301 == typeId) {
    // 4.aout and afreq
    audio_parameter[0] = GetAout;
    audio_parameter[1] = GetAFre;
  }
  if (QMessageBox::No ==
      QMessageBox::question(this, tr("question"),
                            tr("Apply?                                      "),
                            QMessageBox::No | QMessageBox::Yes,
                            QMessageBox::No)) {
    return;
  }
  mode = 6;
  return;
}

void Hdmioptiongui::on_sli_H_valueChanged(int value) {
  qDebug() << value;
  fontsize = value;
  this->setStyleSheet(QString("font:%1pt;").arg(value));
  ui->labTitle->setFont(QFont("Microsoft Yahei", 9 + value));
  // qDebug()<<width<<"*"<<height;
  this->resize(width, height);
  ui->tree_Dev->setIconSize(QSize(10 + value, 10 + value));
}

void Hdmioptiongui::on_btn_Res_clicked() {
  if (tunerflg != 1) {
    if (QMessageBox::Ok ==
        QMessageBox::warning(this, tr("warn"),
                             tr("Device not selected,or open fail"),
                             QMessageBox::Ok, QMessageBox::Ok)) {
      return;
    }
    return;
  }
  if (mode != 0) {
    if (QMessageBox::Ok ==
        QMessageBox::warning(this, tr("warn"),
                             tr("Device is busy,please wait..  "),
                             QMessageBox::Ok, QMessageBox::Ok)) {
      return;
    }
    return;
  }

  // get ui vaule
  memset(Pname, '\0', sizeof(Pname));

  workMode = 1;
  epkt = 0;
  char showbuf[64];
  u8 bytes[6];
  tbs.target_ext_memory_rd_wt(READ, 0x80 * tuner + 0x08, bytes, 6);
  sprintf(showbuf, "HDMI%02x%02x%02x", bytes[3], bytes[4], bytes[5]);
  QString str = QString(showbuf);
  // qDebug(str);
  QByteArray ba = str.toLatin1();
  char *ch = ba.data();
  int i = 0;
  while (ch[i] != '\0') {
    Pname[i] = (unsigned char)(ch[i]);
    i++;
  }
  PNo = (u8)(tunercpy + 1);
  Vpid = 260 + 10 * tunercpy;
  Apid = 261 + 10 * tunercpy;
  Pmt = 262 + 10 * tunercpy;
  Pcr = 263 + 10 * tunercpy;
  MuteEn = 0;
  if (5301 == typeId) {
    // 1.aout and afreq
    audio_parameter[0] = 0;
    audio_parameter[1] = 3;
  }
  if (QMessageBox::No ==
      QMessageBox::question(this, tr("question"),
                            tr("Load Defaults?                              "),
                            QMessageBox::No | QMessageBox::Yes,
                            QMessageBox::No)) {
    return;
  }
  loadflg = 1;
  mode = 6;
}
