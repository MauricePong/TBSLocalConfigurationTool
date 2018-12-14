#ifndef HDMIOPTIONGUI_H
#define HDMIOPTIONGUI_H
#include <QDialog>
#include <QTreeWidgetItem>
#include "appinit.h"
#include "tbsfunc.h"
#include "threadtbs.h"
#if WINDOWSSPACE

#endif
namespace Ui {
class Hdmioptiongui;
}

class Hdmioptiongui : public QDialog {
  Q_OBJECT

 public:
  explicit Hdmioptiongui(QWidget *parent = 0);
  ~Hdmioptiongui();
  // win
  void go_win(void);
  void initForm();
  int DeviceGetList();
  int StartDevice(int dwDeviceNo);
  int StopDevice();
  IBaseFilter *CreateDVBSNetworkProvider();
  IDVBTuningSpace2 *LoadDVBSTuningSpace(void);
  IDVBTuningSpace2 *CreateDVBSTuningSpace(void);
  int BDACleanUp();
  HRESULT create_tuner(IGraphBuilder *pfltgraph, int no, IBaseFilter **pfilter,
                       wchar_t *TunerName, int *tuner_type);
  HRESULT ConnectFilters(IGraphBuilder *pGraph, IBaseFilter *pFilterUpstream,
                         IBaseFilter *pFilterDownstream);
  HRESULT create_captureFilter(IGraphBuilder *pfltgraph, IBaseFilter *pftuner,
                               IBaseFilter **pfilter);
  HRESULT add_filter(IGraphBuilder *pfltgraph, IBaseFilter *pfilter);
  HRESULT remove_filter(IGraphBuilder *pfltgraph, IBaseFilter *pfilter);
  IBaseFilter *CreateInfinitePinTee(void);
  IBaseFilter *LoadFilter(CLSID clsid);
  IBaseFilter *CreateMPEG2Demultiplexer(void);
  IBaseFilter *CreateTransportInformationFilter(void);
  IBaseFilter *FindFilter(CLSID clsid, WCHAR *name);
  IPin *FindPinOnFilter(IBaseFilter *pBaseFilter, char *pPinName);
  void ConnectPins(IGraphBuilder *builder, IBaseFilter *out_from,
                   IBaseFilter *in_to, WCHAR *from_name = NULL,
                   WCHAR *to_name = NULL);
  // recorder
  void lockSingle();
  void startBtn();
  void stopBtn();
  void Init_RTP_OR_UDP();
  void UnInit_RTP_OR_UDP();
  int readBuffer(void);
  int addTreeIteml(char *bf, char **bf_c, PDevAPIDevInfo imch);
  int Iputfmt();
  int getSn();
  int outputfmt();
  int getMute();
  int getName();
  int getNo();
  int getPid();
  int getAudioSourceAndFreq();
  int getEpkt();
  int readConfig();
  int writeConfig();

 protected:
  bool eventFilter(QObject *watched, QEvent *event);

 private:
  Ui::Hdmioptiongui *ui;
  tbsfunc tbs;
  threadTbs *th;
  // hardinfor **hf;
  QMessageBox *qbox;
  u8 emflg;
  u8 loadflg;
  int width;
  int height;
  int width_s;
  int height_s;
  int fontsize;
  FILE *configfp;
  QTimer *qtimer;
  CComPtr<IBaseFilter> m_pTunerDevice;
  IDVBTuningSpace2 *tuningspace;
 private slots:
  void radioClick();
  void radioClick_M();
  void radioClick_C();
  void showSelectedImage(QTreeWidgetItem *item,
                         int column);  //点击树节点事件
  void show_status(Msg *g);
  void timeServiceFunc();
  void progressBarShow(int begin, int end, int progress);
  void on_btn_Start_clicked();
  void on_btn_Cancel_clicked();
  void on_btn_Run_clicked();
  void on_sli_H_valueChanged(int value);
  void on_btn_Res_clicked();
  void on_btn_Browse_clicked();
  void on_btnMenu_Min_clicked();
  void on_btnMenu_Max_clicked();
  void on_btnMenu_Close_clicked();
  void on_tabWidget_currentChanged(int idx);
  void on_com_Aout_currentIndexChanged(int idx);
  void test();
};

#endif  //
