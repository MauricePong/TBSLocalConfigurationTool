#ifndef TBSFUNC_H
#define TBSFUNC_H
#define RWNO  128
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
extern u8  tuner;
extern u8  tunercpy;
extern u16 tbschipid;
extern u8 rwbuf[RWNO];
extern int tunerFd;
extern u32 typeId;

//////////////////////////////
#include <QDebug>
#include <QDialog>
#include <QTreeWidgetItem>
#include <QMessageBox>
#include <QTime>
#include <winsock2.h>
#include <windows.h>
#include <dshow.h>
#include <commctrl.h>

//#include <tchar.h>
#include "dsenum.h"
#include "Version.h"
#include <stdio.h>
#include <conio.h>
#include <string.h>
#include <initguid.h>
#include <ks.h>
#include <ksmedia.h>
#include <bdatypes.h>
#include <bdamedia.h>
#include <bdaiface.h>
#include <tuner.h>
#include <atlbase.h>
#include <comdef.h>
#include <dbt.h>
#include <initguid.h>
#include <objbase.h>
//#include <uuids.h>
#include <ksproxy.h>
#include <mmreg.h>
#include <msacm.h>
#include <streams.h>
//udp/rtp
#include <process.h>
#include <tchar.h>
#include <iostream>

#include "VBufferT.h "
#include "VBuffer.h "
#include "Udp.h"
#include "Rtp.h"
#include "TLock.h"
#include "Socket.h"


#define  WINDOWSSPACE 1
const GUID KSPROPSETID_BdaTunerExtensionProperties =
{ 0xfaa8f3e5, 0x31d4, 0x4e41,{ 0x88, 0xef, 0xd9, 0xeb, 0x71, 0x6f, 0x6e, 0xc9 } };

typedef enum {
	KSPROPERTY_BDA_DISEQC_MESSAGE = 0,  //Custom property for Diseqc messaging
	KSPROPERTY_BDA_DISEQC_INIT,         //Custom property for Intializing Diseqc.
	KSPROPERTY_BDA_SCAN_FREQ,           //Not supported
	KSPROPERTY_BDA_CHANNEL_CHANGE,      //Custom property for changing channel
	KSPROPERTY_BDA_DEMOD_INFO,          //Custom property for returning demod FW state and version
	KSPROPERTY_BDA_EFFECTIVE_FREQ,      //Not supported
	KSPROPERTY_BDA_SIGNAL_STATUS,       //Custom property for returning signal quality, strength, BER and other attributes
	KSPROPERTY_BDA_LOCK_STATUS,         //Custom property for returning demod lock indicators
	KSPROPERTY_BDA_ERROR_CONTROL,       //Custom property for controlling error correction and BER window
	KSPROPERTY_BDA_CHANNEL_INFO,        //Custom property for exposing the locked values of frequency,symbol rate etc after
										//corrections and adjustments
	KSPROPERTY_BDA_NBC_PARAMS,
	KSPROPERTY_BDA_BLIND_SCAN,
	KSPROPERTY_BDA_GET_MEDIAINFO,
	KSPROPERTY_BDA_STREAMTYPE_PARAMS,
	KSPROPERTY_BDA_INPUTMULTISTREAMID,//added 2011 01 27
	KSPROPERTY_BDA_I2CACCESS,//added 2010 12 07
	KSPROPERTY_BDA_FPGA_RDID = 100,
	KSPROPERTY_BDA_FPGA_ERASE_WriteCtrl = 101,
	KSPROPERTY_BDA_FPGA_WRITEONEPAGE = 102,
	KSPROPERTY_BDA_FPGA_VERIFY = 103,

	KSPROPERTY_BDA_HDMI_CAPTURE_FORMAT = 104,
	KSPROPERTY_BDA_MCU_FW_WR = 105,
	KSPROPERTY_BDA_CAPTURE_ACCESS = 106,

} KSPROPERTY_BDA_TUNER_EXTENSION;


#ifndef WRITE
#define WRITE            0
#endif
#ifndef READ
#define READ             1
#endif
//spi flash
#define SPI_CONFIG      	 0x10
#define SPI_STATUS      	 0x10
#define SPI_COMMAND     	 0x14
#define SPI_WT_DATA     	 0x18
#define SPI_RD_DATA     	 0x1c
#define SPI_ENABLE       	 0x1c
#define GPIO_VERSION         0x20
//HDMI the folloing address( Write:User input video format, Read: true format from 6604)   very import: High and low is reversed
#define HDMI_RST  		0x44      //system reset
#define HDMI_AUTO_MODE  0x48  	  //Auto_mode (1) meaningful. System to identify video format parameters or user input video format parameters
#define HDMI_HYNC_VYNC  0x48      //bit[31:16]Hync, bit[15:0]Vync
#define HDMI_SCAN_FREQ	0x4c      //bit[31:24] Video format frequency parameters, Bit[16]: user input video format is  (0:interleave): 1:progressive) scanning
//===============================================
//-------------24cXX --------------------------------------------------------
#define BASE_ADDRESS_24CXX  	0x4000
#define CMD_24CXX	      0*4   //write only
#define DATA0_24CXX       1*4   //W/R
#define DATA1_24CXX       2*4   //W/R
#define CS_RELEASE        3*4   //write only

#define STATUS_MAC16_24CXX	 0*4   //read only
#define MAC32_24CXX	 		 3*4   //read only


#define  ADDRESS_HDMI(n)  (0xff00+0x40 *(n))
#define  ADDRESS_HDMI0    0xff00
#define  ADDRESS_HDMI1    0xff40
#define  ADDRESS_HDMI2    0xff80
#define  ADDRESS_HDMI3    0xffc0
#define  OFFSET_START     0*0


#define GPIOBASE_ADDR     0x0000
#define CHIP_OFFSET_ADDR  4*8
//pid and program_number
#define  PID(n)           (0x70+(n)*0x80)
#define  PMT_MSB          0x70
#define  PMT_LSB          0x71
#define  PCR_MSB          0x72
#define  PCR_LSB          0x73
#define  VPID_MSB         0x74
#define  VPID_LSB         0x75
#define  APID_MSB         0x76
#define  APID_LSB         0x77
#define  PROGRAM_START_NO(n) (0x0f+0x80*(n))

// HardRST
#define  BASE_ADDRESS_HARDRST          0x0000
#define  OFFSET_ADDRESS_HARDRST        1*4
//#define
//mac:
#define MAC_ADDRESS_2     0*4
#define MAC_ADDRESS_4     3*4

//#define TBS6301ID_13         (0x13000163)
//#define TBS6301ID_01         (0x01000163)


#define MAP_FPGA_BASEADDR  	0x4000//write
#define WRITE_CMD 			0*8
#define WRITE_DATA 			1*8
#define RELEASE_CS 			2*8
//read
#define HOST_STATUS 		0*8
// {5'd0,mcu_busy,write_error,mcu_done,56'd0};
#define READ_DATA 			1*8
#define READ_MAC 			2*8

#define WTEMPTYPKT(n)                   ((12+(n))*4)
#define RDEMPTYPKT(n)					((16+(n))*4)
#define AUDIIO_SELECT_OFFSETADDR        0x17     //bit[56]
#define AUDIIO_FREQ_SELECT_OFFSETADDR   0x18     //bit[48:49]

#define  BASE_SETTING_NET_ADDR			0x0100
#define  NET_LOCAL_MAC_OFFSETADDR		0x00   //0x00 ~ 0x05
#define  NET_LOCAL_IP_OFFSETADDR        0x08   //0x08~0x0b
#define  NET_SERVER_PORT_OFFSETADDR     0x0c   //0x0c~0x0d
#define  SELECT_UDP_OR_RTP_OFFSETADDR   0x0e
//0x10~0x27:	one RTP parameter(Total 24 bytes)
#define  RTP_PARAMETER_OFFSETADDR       0x10
/*
rtp_gate;  		//bit 7;
all_ts_through;  	//bit6
position;			//bit5~3
ts_port;			//bit2~0
*/
#define  RTP_PID_EN_OFFSETADDR	        0x11
#define  UDP_PORT_OFFSETADDR	        0x12   //2 bytes
#define  RTP_IP_ADDR_OFFSETADDR	        0x14   //4 bytes
#define  RTP_TV_PID_OFFSETADDR	        0x18   //16 bytes

#define  I2C_CHIP_ADDRESS               0x50
#define  BUS_IS_FREE_OFFSETADDR         0x02
#define  CHECK_I2C_BUS_ADDR             0x11
#define  RE_RUN_NETWORK_ADDR            0xfe00


#define TBS6304ID            (0x0463)
#define TBS6301ID            (0x0163)
#define DEVAPI_MAX_DEVICES   32

class Msg{
public:
    bool ok;
    u8 type;
    u32 progress;
};
class CDumpInputPin;
class CPullInputPin;

typedef struct
{
	char szName[256];
	int TunerType;
	QTreeWidgetItem *Item;
	int deviceno;
} TDevAPIDevInfo, *PDevAPIDevInfo;

typedef struct
{
	int Count;
	TDevAPIDevInfo Devices[DEVAPI_MAX_DEVICES];
} TDevAPIDevList, *PDevAPIDevList;

struct Access_24cxx_data {
	ULONG address;
	ULONG data;
};

typedef struct {
	unsigned long ChannelFrequency;//
	unsigned long ulLNBLOFLowBand;
	unsigned long ulLNBLOFHighBand;
	unsigned long SymbolRate;
	unsigned char Polarity;

	UCHAR           LNB_POWER;              // LNB_POWER_ON | LNB_POWER_OFF
	UCHAR           HZ_22K;                 // HZ_22K_OFF | HZ_22K_ON
	UCHAR           Tone_Data_Burst;        // Data_Burst_ON | Tone_Burst_ON |Tone_Data_Disable
	UCHAR           DiSEqC_Port;            // DiSEqC_NULL | DiSEqC_A | DiSEqC_B | DiSEqC_C | DiSEqC_D

	unsigned char motor[5];

	unsigned char ir_code;
	unsigned char lock;
	unsigned char strength;
	unsigned char quality;

	unsigned char reserved[256];
} QBOXDVBSCMD, *PQBOXDVBSCMD;

extern CComPtr<IGraphBuilder> builder;
extern CComPtr<IBaseFilter> provider;
extern CComPtr<IMediaControl> control;
extern IDVBTuningSpace2 * tuningspace;
extern CComPtr<IBaseFilter> CaptureFilter, TunerFilter, TSDump;
extern CComPtr <IPin> m_pTunerPin;                  // the tuner pin on the tuner/demod filter
//extern QString g_strDevName;
extern IKsPropertySet *m_pKsCtrl;
extern HANDLE hIOMutex;
extern CComPtr<IBaseFilter> demux, infinite, tif;
extern IKsPropertySet *m_pVCKsCtrl;
extern int col;
extern PDevAPIDevList DL;

/*recorder*/
extern BOOL IsOpenFileFlg;
extern BOOL IsStartFlg;
extern u32 SecCount;
extern u32 HrCount;
extern u32 MINCount;
extern u32 KbCount;
extern u32 MbCount;
extern u32 GbCount;
extern u32 MaxFileSize;
extern u32 TimeSize;
extern BOOL IsSizeOrTimeFlg;
extern QBOXDVBSCMD m_CmdData;
extern QString pathName;
extern BOOL noSingleFlg;
extern BOOL isRtp;
extern BOOL isSendTs;
extern Udp* g_udp;
extern Rtp* g_rtp;


extern CCritSec m_Lock;
extern FILE *w_file_fd;
extern volatile u8 mode;

#define LNB_POWER_OFF                       0
#define LNB_POWER_ON                        1

#define HZ_22K_OFF                          0
#define HZ_22K_ON                           1

#define Value_Data_Burst_ON                       1
#define Value_Tone_Burst_ON                       0
#define Value_Burst_OFF	                        2

#define POLARITY_H                          0
#define POLARITY_V                          1

#define DiSEqC_NULL                         0
#define DiSEqC_A                            1
#define DiSEqC_B                            2
#define DiSEqC_C                            3
#define DiSEqC_D                            4

#define DVBS_TUNING_SPACE_NAME		_bstr_t("turbosight dvb-s")

#define DVBT_TUNING_SPACE_NAME		_bstr_t("turbosight dvb-t")

#define TUNING_SPACE_NAME			_bstr_t("turbosight-c")

struct Access_usb_ctrl {
	unsigned char chip_addr;
	unsigned char reg_addr;
	unsigned char num;
	unsigned char buf[8];
};

class tbsfunc
{
public:
	explicit tbsfunc();
	~tbsfunc();

	unsigned char spi_CheckFree(unsigned char OpbyteNum);
	void spi_WriteEnable();
	void spi_WriteDisable();
	unsigned char spi_GetFlashID(unsigned char *flashID);
	unsigned char spi_ChipErase();
	unsigned char spi_ChipEraseStatus();
	bool spi_read32bit(unsigned char * addr24bit, u32 *data32bit);
	bool spi_read1page(unsigned char * addr24bit, unsigned char *data256bytes);
	bool spi_Write32bit(unsigned char * addr24bit, u32 *data32bit);
	bool spi_Write1page(unsigned char * addr24bit, unsigned char *data256byte);
	void spi_EnterDeepPowerDown();
	unsigned char read_mcu_status();
	void erase_mcu(unsigned int sectorAddr);
	void write_mcu(unsigned int subAddr, unsigned char *wtbuffer);
	void read_mcu(unsigned int subAddr, unsigned char *rdbuffer);
	void read_M23Register(int num, int* m23_data);
	void write_M23Register(int num, int* m23_addr, int* m23_data);
	void mcu_software_reset();


	void HardRST();
	void restartHdmi();
	int  monopolizeCpu_w(int m_addr, unsigned char *wtbuff, unsigned char num);
	bool monopolizeCpu_r(int m_addr, unsigned char *rdbuff, unsigned char num);

	int  monopolizeCpu_w_REG32(int m_addr, unsigned char *wtbuff, unsigned char num);
	bool monopolizeCpu_r_REG32(int m_addr, unsigned char *rdbuff, unsigned char num);

	int  monopolizeCpu_w_REG64(int m_addr, unsigned char *wtbuff, unsigned char num);
	bool monopolizeCpu_r_REG64(int m_addr, unsigned char *rdbuff, unsigned char num);

	BOOL i2c_write(unsigned char chipAddr, unsigned char regAddr, unsigned char num, unsigned char *buf);
	ULONG i2c_read(unsigned char chipAddr, unsigned char regAddr, unsigned char num, unsigned char *buf);
	void usb_REG64_rd(int subAddr, unsigned char num, unsigned char *rdbuffer);
	void usb_REG64_wt(int subAddr, unsigned char num, unsigned char *wtbuffer);
	void REG64_rd(int subAddr, unsigned char num, unsigned char *rdbuffer);
	void REG64_wt(int subAddr, unsigned char num, unsigned char *wtbuffer);
	unsigned char read_from_external_memory(int addr, unsigned char num, unsigned char *buff);
	unsigned char write_to_external_memory(int addr, unsigned char num, unsigned char *buff);
	unsigned char check_host_free_REG64();
	unsigned char Check_host_status_REG64(unsigned char status);
	BOOL wrReg32(u32 uAddr, u32 udata);
	u32 rdReg32(u32 uAddr);
	u32 tbs_read(u8 address);
	BOOL tbs_write(u8 address, u32 data);
	u32 tbs_24cxx_rd(u8 address);
	BOOL tbs_24cxx_wr(u8 address, u32 data);
	BOOL Check_host_status(unsigned char cs);
	BOOL wait_for_host_workdone();
	BOOL ext_one_time_wt(int m_addr,
		unsigned char *wtbuff,
		unsigned char num);
	BOOL ext_one_time_wt_REG32(int m_addr,
		unsigned char *wtbuff,
		unsigned char num);
	BOOL ext_ram_write(int ram_addr,
		unsigned char *wtbff,
		int wt_size);
	BOOL ext_one_time_rd(int m_addr,
		unsigned char *rdbuff,
		unsigned char num);
	BOOL ext_one_time_rd_REG32(int m_addr,
		unsigned char *rdbuff,
		unsigned char num);
	BOOL ext_ram_read(int ram_addr,
		unsigned char *rdbff,
		int rd_size);
	BOOL target_ext_memory_rd_wt(unsigned char tg_mode,
		int tg_addr,
		unsigned char *buff,
		int tg_size);

	bool wrReg32_Extern(u32 uAddr, u32 udata);
	u32  rdReg32_Extern(u32 uAddr);
	int usb_checkI2cBusStatus();

	int checkI2cBusStatus();


};


class CDestFilter : public CBaseFilter
{
public:
	DWORD			m_Pid;
	CComPtr <IMPEG2PIDMap>		m_pMpeg2PidMap;
	CDumpInputPin	*m_pPin;

	// Constructor
	CDestFilter(LPUNKNOWN pUnk,
		CCritSec *pLock,
		HRESULT *phr);

	// Pin enumeration
	CBasePin * GetPin(int n);
	int GetPinCount();
	STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** ppv);
};

//////////////////////////////////////////////////////////////////////////////////////

class CDumpInputPin : public CBaseInputPin
{

	CDestFilter    * const m_pFilter;           // Main renderer object
	CCritSec * const m_pReceiveLock;    // Sample critical section

public:

	void		RefreshConnect() {
		m_tmpBufferLen = 0;
		m_bOpened = true;
	}

	bool		m_bOpened;

	byte		m_tmpBuffer[188];
	int			m_tmpBufferLen;

	CDumpInputPin(CDestFilter *pDump,
		LPUNKNOWN pUnk,
		CBaseFilter *pFilter,
		CCritSec *pLock,
		HRESULT *phr);

	// Do something with this media sample
	STDMETHODIMP Receive(IMediaSample *pSample);
	STDMETHODIMP EndOfStream(void);
	STDMETHODIMP ReceiveCanBlock();
	//HRESULT WriteStringInfo(IMediaSample *pSample);

	// Check if the pin can support this specific proposed type and format
	HRESULT CheckMediaType(const CMediaType *);
	HRESULT GetMediaType(int iPosition, CMediaType *pMediaType);

	// Break connection
	HRESULT BreakConnect();

	// Track NewSegment
	STDMETHODIMP NewSegment(REFERENCE_TIME tStart,
		REFERENCE_TIME tStop,
		double dRate);
};
extern CDestFilter *m_pFilterDest;

#endif // TBSFUNC_H
