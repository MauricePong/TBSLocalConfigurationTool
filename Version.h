
#pragma once
#include "tbsfunc.h"
//define the product version
//#define  TBS6921
//#define  TBS5921
//#define  TBS8921

//6925和 6922 是一样的
//#define  TBS6925
//#define  TBS6922

//#define TBS6984
//#define TBS6904

#define TBSNULL 0
#define TBS6904 1
#define TBS6908 2

#define TBS6910 3
#define TBS6001 4
#define TBS6991 5

//dvbc

#define TBS6514DVBC 7

//dvbt
#define TBS6514DTMB 8
#define TBS6205DVBT 9

//dvbc
#define TBS6205DVBC 10
#define TBS6205MCNS 11

//dvbt
#define TBS6814ISDBT 12
#define TBS6704ATSC 13

#define TBS6905 14

#define TBS6903 15

#define TBS6909 16

#define TBS6902 17

#define TBS6522 18

#define TBS6522_dvbt 19

#define TBS6590 20

#define TBS6290_dvbt 21

#define TBS6301_dvbs 23
#define TBS6302_dvbs 24
#define TBS6304_dvbs 25
#if 1

static WCHAR *TunerName = (WCHAR *)L"TBS 6904 DVBS/S2 Tuner 0";
static WCHAR *CaptuerName = (WCHAR *)L"TBS 6904 DVBS/S2 Capture 0";
static QString strTBSProductName = (QString)("Update Firmware Tools");

static WCHAR *TBS6904TunerName = (WCHAR *)L"TBS 6904 DVBS/S2 Tuner 0";
static WCHAR *TBS6904CaptuerName = (WCHAR *)L"TBS 6904 DVBS/S2 Capture 0";

static WCHAR *TBS6908TunerName = (WCHAR *)L"TBS 6908 DVBS/S2 Tuner 0";
static WCHAR *TBS6908CaptuerName = (WCHAR *)L"TBS 6908 DVBS/S2 Capture 0";

static WCHAR *TBS6905TunerName = (WCHAR *)L"TBS 6905 DVBS/S2 Tuner 0";
static WCHAR *TBS6905CaptuerName = (WCHAR *)L"TBS 6905 DVBS/S2 Capture 0";


static WCHAR *TBS6910TunerName = (WCHAR *)L"TBS 6910 DVBS/S2 Tuner 0";
static WCHAR *TBS6910CaptuerName = (WCHAR *)L"TBS 6910 DVBS/S2 Capture 0";

static WCHAR *TBS6001TunerName = (WCHAR *)L"TBS 6001 DVBS/S2 Tuner A";
static WCHAR *TBS6001CaptuerName = (WCHAR *)L"TBS 6001 DVBS/S2 Capture A";


static WCHAR *TBS6991TunerName = (WCHAR *)L"TBS 6991 DVBS/S2 Tuner A";
static WCHAR *TBS6991CaptuerName = (WCHAR *)L"TBS 6991 DVBS/S2 Capture A";

static WCHAR *TBS6514DVBCTunerName = (WCHAR *)L"TBS 6514 DVBC Tuner 0";
static WCHAR *TBS6514DVBCCaptuerName = (WCHAR *)L"TBS 6514 DVBC Capture 0";

static WCHAR *TBS6514DTMBTunerName = (WCHAR *)L"TBS 6514 DTMB Tuner 0";
static WCHAR *TBS6514DTMBCaptuerName = (WCHAR *)L"TBS 6514 DTMB Capture 0";

static WCHAR *TBS6205DVBTTunerName = (WCHAR *)L"TBS 6205 DVB-T/T2 Tuner 0";
static WCHAR *TBS6205DVBTCaptuerName = (WCHAR *)L"TBS 6205 DVB-T/T2 Capture 0";

static WCHAR *TBS6205DVBCTunerName = (WCHAR *)L"TBS 6205 DVBC Tuner 0";
static WCHAR *TBS6205DVBCCaptuerName = (WCHAR *)L"TBS 6205 DVBC Capture 0";

static WCHAR *TBS6205MCNSTunerName = (WCHAR *)L"TBS 6205 MCNS Tuner 0";
static WCHAR *TBS6205MCNSCaptuerName = (WCHAR *)L"TBS 6205 MCNS Capture 0";

static WCHAR *TBS6814ISDBTTunerName = (WCHAR *)L"TBS 6814 ISDB-T Tuner 0";
static WCHAR *TBS6814ISDBTCaptuerName = (WCHAR *)L"TBS 6814 ISDB-T Capture 0";

static WCHAR *TBS6704ATSCTunerName = (WCHAR *)L"TBS 6704 ATSC Tuner 0";
static WCHAR *TBS6704ATSCCaptuerName = (WCHAR *)L"TBS 6704 ATSC Capture 0";


static WCHAR *TBS6903TunerName = (WCHAR *)L"TBS 6903 DVBS/S2 Tuner 0";
static WCHAR *TBS6903CaptuerName = (WCHAR *)L"TBS 6903 DVBS/S2 Capture 0";

static WCHAR *TBS6909TunerName = (WCHAR *)L"TBS 6909 DVBS/S2 Tuner 0";
static WCHAR *TBS6909CaptuerName = (WCHAR *)L"TBS 6909 DVBS/S2 Capture 0";

static WCHAR *TBS6902TunerName = (WCHAR *)L"TBS 6902 DVBS/S2 Tuner 0";
static WCHAR *TBS6902CaptuerName = (WCHAR *)L"TBS 6902 DVBS/S2 Capture 0";

static WCHAR *TBS6522TunerName = (WCHAR *)L"TBS 6522 DVBS/S2 Tuner 0";
static WCHAR *TBS6522CaptuerName = (WCHAR *)L"TBS 6522 DVBS/S2 Capture 0";

static WCHAR *TBS6590TunerName = (WCHAR *)L"TBS 6590 DVBS/S2 Tuner 0";
static WCHAR *TBS6590CaptuerName = (WCHAR *)L"TBS 6590 DVBS/S2 Capture 0";

static WCHAR *TBS6522_DVBT_TunerName = (WCHAR *)L"TBS 6522 DVBT Tuner 0";
static WCHAR *TBS6522_DVBT_CaptuerName = (WCHAR *)L"TBS 6522 DVBT Capture 0";

static WCHAR *TBS6290_DVBT_TunerName = (WCHAR *)L"TBS 6290 DVBT/T2 Tuner A";
static WCHAR *TBS6290_DVBT_CaptuerName = (WCHAR *)L"TBS 6290 DVBT/T2 Capture A";

static WCHAR *TBS2990TunerName = (WCHAR *)L"TBS 2990 DVBS/S2 Tuner 0";
static WCHAR *TBS2990CaptuerName = (WCHAR *)L"TBS 2990 DVBS/S2 Capture 0";

static WCHAR *TBS6301TunerName = (WCHAR *)L"TBS 6301 HDMI Capture 0";
static WCHAR *TBS6301CaptuerName = (WCHAR *)L"TBS 6301 HDMI Node 0";

static WCHAR *TBS6302TunerName = (WCHAR *)L"TBS 6302 HDMI Capture 0";
static WCHAR *TBS6302CaptuerName = (WCHAR *)L"TBS 6302 HDMI Node 0";

static WCHAR *TBS6304TunerName = (WCHAR *)L"TBS 6304 HDMI Capture 0";
static WCHAR *TBS6304CaptuerName = (WCHAR *)L"TBS 6304 HDMI Node 0";

#endif

