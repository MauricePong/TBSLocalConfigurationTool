#include "tbsfunc.h"
u8  tuner = 0;
u8 tunercpy = 0;
u16 tbschipid = 0;
u8  rwbuf[RWNO] = {0};
int tunerFd = 0;
int col = 0;
u32 typeId = 0;

PDevAPIDevList DL;
CComPtr<IGraphBuilder> builder;
CComPtr<IBaseFilter> provider;
CComPtr<IMediaControl> control;
IDVBTuningSpace2 * tuningspace = NULL;
CComPtr<IBaseFilter> CaptureFilter, TunerFilter, TSDump;
//QString g_strDevName;
IKsPropertySet *m_pKsCtrl;
HANDLE hIOMutex;
CComPtr<IBaseFilter> demux, infinite, tif;
CComPtr <IPin> m_pTunerPin;                  // the tuner pin on the tuner/demod filter
IKsPropertySet *m_pVCKsCtrl;

BOOL isRtp = 0;//0,udp; 1,rtp
BOOL isSendTs = 0; //0,record; 1,sendts
BOOL IsOpenFileFlg = 0;
BOOL IsStartFlg = 0;
u32 HrCount = 0;
u32 MINCount = 0;
u32 SecCount = 0;
u32 KbCount = 0;
u32 MbCount = 0;
u32 GbCount = 0;
u32 MaxFileSize = 0;
u32 TimeSize = 0 ;
BOOL IsSizeOrTimeFlg = 0; //0 size 1 time
QBOXDVBSCMD m_CmdData;
CDestFilter	*m_pFilterDest = NULL;
QString pathName;
CCritSec m_Lock;
FILE *w_file_fd = NULL;
BOOL noSingleFlg = 0;
Udp* g_udp;
Rtp* g_rtp;
tbsfunc::tbsfunc()
{

}

tbsfunc::~tbsfunc()
{
}

BOOL tbsfunc::wrReg32(u32 uAddr, u32 udata)
{

	DWORD TypeSupport = 0;
	Access_24cxx_data mcu24cxx;
	HRESULT hr = m_pKsCtrl->QuerySupported(KSPROPSETID_BdaTunerExtensionProperties,
		KSPROPERTY_BDA_CAPTURE_ACCESS,
		&TypeSupport);

	if FAILED(hr)
	{
		qDebug("QuerySupported KSPROPERTY_BDA_CAPTURE_ACCESS error");
		return  0;
	}

	mcu24cxx.address = uAddr;
	mcu24cxx.data = udata;


	hr = m_pKsCtrl->Set(KSPROPSETID_BdaTunerExtensionProperties,
		KSPROPERTY_BDA_CAPTURE_ACCESS,
		&mcu24cxx, sizeof(mcu24cxx),
		&mcu24cxx, sizeof(mcu24cxx));

	return 1;

}

u32 tbsfunc::rdReg32(u32 uAddr)
{

	DWORD TypeSupport = 0;
	unsigned long BytesRead = 0;
	Access_24cxx_data mcu24cxx;
	u32 udata = 0;

	HRESULT hr = m_pKsCtrl->QuerySupported(KSPROPSETID_BdaTunerExtensionProperties,
		KSPROPERTY_BDA_CAPTURE_ACCESS,
		&TypeSupport);

	if FAILED(hr)
	{
		qDebug("QuerySupported KSPROPERTY_BDA_CAPTURE_ACCESS error");
		return 0;
	}

	mcu24cxx.address = uAddr;

	// set address and get data at the same time.
	hr = m_pKsCtrl->Get(KSPROPSETID_BdaTunerExtensionProperties,
		KSPROPERTY_BDA_CAPTURE_ACCESS,
		&mcu24cxx, sizeof(mcu24cxx),
		&mcu24cxx, sizeof(mcu24cxx), &BytesRead);
	if SUCCEEDED(hr)
	{
		udata = (u32)mcu24cxx.data;
	}
	return udata;
}


void tbsfunc::HardRST()
{
	u8 tmpbuf[4] = { 0xff,0xff,0xff,0xff };
	tbs_write(1 * 4, *(u32 *)tmpbuf);
	Sleep(2000);
	tmpbuf[0] = 0x00;
	tbs_write(1 * 4, *(u32 *)tmpbuf);
}



int tbsfunc::monopolizeCpu_w_REG32(int m_addr, unsigned char *wtbuff, unsigned char num)
{
	unsigned char i = 0;
	unsigned char buff[8] = { 0 };

	if (num == 0)
	{
		qDebug(" wt error! (num == 0)  \n");
		return 0;

	}
	else if (num > 8)
	{
		qDebug(" wt error! (num >8) ");
		return 0;

	}
	//qDebug("222222222222222222222222");
	//...check host free -->......................
	if (Check_host_status(1) == 0)
	{
		qDebug(" wait for host free timeout!");
		return 0;
	}
	//qDebug("333333333333333333333");
	//...wt data & cs to low-->...................
	for (i = 0; i < num; i++)
		buff[i] = wtbuff[i];
	tbs_24cxx_wr(2 * 4, *(u32 *)&buff[4]);
	tbs_24cxx_wr(1 * 4, *(u32 *)buff);

	buff[0] = 0x32;
	buff[1] = num;
	buff[2] = (unsigned char)(m_addr >> 8);
	buff[3] = (unsigned char)(m_addr & 255);
	tbs_24cxx_wr(0 * 4, *(u32 *)buff);  //cs low;

										//...ck host received data & process-->........
	if (Check_host_status(0) == 0)
	{
		qDebug(" wait for host active timeout! ");
		//return 0;
	}

	//...cs high................................

	tbs_24cxx_wr(3 * 4, *(u32 *)buff);  //cs high;

										//...check host work done-->..................
	if (wait_for_host_workdone() == 0)
	{
		qDebug(" wait host work done timeout! ");
		return 0;
	}
	return 1;
}

int tbsfunc::monopolizeCpu_w_REG64(int m_addr, unsigned char * wtbuff, unsigned char num)
{
	unsigned char i, tmp_buff[8];

	if (num >8)
	{
		qDebug(" write_to_external_memory parameter error! [num >8]");
		return 0;
	}

	//check_host_free;...............................
	if (check_host_free_REG64() == 0)
	{
		qDebug(" wait for host free timeout!");
		return 0;
	}

	if (num != 0)
	{
		for (i = 0; i<num; i++)
			tmp_buff[i] = wtbuff[i];
		REG64_wt(MAP_FPGA_BASEADDR + WRITE_DATA, num, tmp_buff);
	}

	//write tag;
	tmp_buff[0] = 0x32;
	tmp_buff[1] = num;
	tmp_buff[2] = (unsigned char)(m_addr >> 8);
	tmp_buff[3] = (unsigned char)(m_addr & 255);
	REG64_wt(MAP_FPGA_BASEADDR + WRITE_CMD, 4, tmp_buff);

	//check_host_active;...............................
	if (Check_host_status_REG64(0) == 0)
	{
		qDebug(" wait for host active timeout!");
		//return 0;
	}

	//release cs;
	REG64_wt(MAP_FPGA_BASEADDR + RELEASE_CS, 1, tmp_buff);
	if (-1 == checkI2cBusStatus()) {
		//	return 0;
	}
	//check host work done;...........................
	if (Check_host_status_REG64(1) == 0)
	{
		qDebug("wait for host work done timeout!");
		return 0;
	}

	return 1;
}

int tbsfunc::monopolizeCpu_w(int m_addr, unsigned char * wtbuff, unsigned char num)
{
	switch (typeId) {
	case 5301:
		return monopolizeCpu_w_REG64(m_addr, wtbuff, num);
		break;
	case 6304:
	case 6308:
		return monopolizeCpu_w_REG32(m_addr, wtbuff, num);
		break;
	default:
		break;
	}
	return 0;
}

bool tbsfunc::monopolizeCpu_r_REG64(int m_addr, unsigned char * rdbuff, unsigned char num)
{
	unsigned char i, tmp_buff[8];

	if (num == 0)
	{
		qDebug(" read_from_external_memory parameter error! [num == 0]");
		return 0;
	}
	else if (num >8)
	{
		qDebug(" read_from_external_memory parameter error! [num >8]");
		return 0;
	}
	for (i = 0; i<num; i++)
		rdbuff[i] = 0xff;

	//check_host_free;...............................
	if (check_host_free_REG64() == 0)
	{
		qDebug(" wait for host free timeout! ");
		return 0;
	}

	//read tag;
	tmp_buff[0] = 0x33;
	tmp_buff[1] = num;
	tmp_buff[2] = (unsigned char)(m_addr >> 8);
	tmp_buff[3] = (unsigned char)(m_addr & 255);
	REG64_wt(MAP_FPGA_BASEADDR + WRITE_CMD, 4, tmp_buff);

	//check_host_active;...............................
	if (Check_host_status_REG64(0) == 0)
	{
		qDebug(" wait for host active timeout!");
		//return 0;
	}

	//release cs;
	REG64_wt(MAP_FPGA_BASEADDR + RELEASE_CS, 1, tmp_buff);

	//check host work done;...........................
	if (Check_host_status_REG64(1) == 0)
	{
		qDebug("wait for host work done timeout!");
		return 0;
	}

	//read back;
	REG64_rd(MAP_FPGA_BASEADDR + READ_DATA, num, rdbuff);

	return false;
}

BOOL tbsfunc::i2c_write(unsigned char chipAddr, unsigned char regAddr, unsigned char num, unsigned char *buf)
{

	DWORD TypeSupport = 0;
	Access_usb_ctrl usbCtrlData;
	HRESULT hr = m_pKsCtrl->QuerySupported(KSPROPSETID_BdaTunerExtensionProperties,
		KSPROPERTY_BDA_CAPTURE_ACCESS,
		&TypeSupport);

	if FAILED(hr)
	{
		qDebug("QuerySupported KSPROPERTY_BDA_FLASH_PROGRAM error");
		return  0;
	}

	usbCtrlData.chip_addr = chipAddr;
	usbCtrlData.reg_addr = regAddr;
	usbCtrlData.num = num;

	memcpy(usbCtrlData.buf, buf, num);


	hr = m_pKsCtrl->Set(KSPROPSETID_BdaTunerExtensionProperties,
		KSPROPERTY_BDA_CAPTURE_ACCESS,
		&usbCtrlData, sizeof(usbCtrlData),
		&usbCtrlData, sizeof(usbCtrlData));

	return 1;

}


ULONG tbsfunc::i2c_read(unsigned char chipAddr, unsigned char regAddr, unsigned char num, unsigned char *buf)
{

	DWORD TypeSupport = 0;
	unsigned long BytesRead = 0;
	Access_usb_ctrl usbCtrlData;
	//ULONG udata;

	HRESULT hr = m_pKsCtrl->QuerySupported(KSPROPSETID_BdaTunerExtensionProperties,
		KSPROPERTY_BDA_CAPTURE_ACCESS,
		&TypeSupport);

	if FAILED(hr)
	{
		qDebug("QuerySupported KSPROPERTY_BDA_FLASH_VERIFY error");
		return 0;
	}

	usbCtrlData.chip_addr = chipAddr;
	usbCtrlData.reg_addr = regAddr;
	usbCtrlData.num = num;

	// set address and get data at the same time.
	hr = m_pKsCtrl->Get(KSPROPSETID_BdaTunerExtensionProperties,
		KSPROPERTY_BDA_CAPTURE_ACCESS,
		&usbCtrlData, sizeof(usbCtrlData),
		&usbCtrlData, sizeof(usbCtrlData), &BytesRead);
	if SUCCEEDED(hr)
	{
		memcpy(buf, usbCtrlData.buf, num);
	}

	return 1;


}

void tbsfunc::usb_REG64_rd(int subAddr, unsigned char num, unsigned char *rdbuffer)
{
	unsigned char tmpbuf[2];
	int i = 0;

	if (num == 0)
	{
		qDebug(" REG64_rd parameter error! [num == 0]");
		return;
	}
	else if (num >8)
	{
		qDebug(" REG64_rd parameter error! [num >8]");
		return;
	}

	//write address;
	tmpbuf[0] = (unsigned char)(subAddr >> 8);
	tmpbuf[1] = (unsigned char)(subAddr & 255);
	i2c_write(0x50, 6, 2, tmpbuf);

	//trigger read;
	tmpbuf[0] = 0x88;
	i2c_write(0x50, 3, 1, tmpbuf);

	//get data:
	i2c_read(0x50, 8, num, rdbuffer);
	return;
}


//--------------------------------------------------------------------------------------------
void tbsfunc::usb_REG64_wt(int subAddr, unsigned char num, unsigned char *wtbuffer)
{
	unsigned char tmpbuf[2];

	if (num == 0)
	{
		qDebug(" REG64_wt parameter error! [num == 0]");
		return;
	}
	else if (num >8)
	{
		qDebug(" REG64_wt parameter error! [num >8]");
		return;
	}

	//write address;
	tmpbuf[0] = (unsigned char)(subAddr >> 8);
	tmpbuf[1] = (unsigned char)(subAddr & 255);
	i2c_write(0x50, 6, 2, tmpbuf);


	//write data;
	i2c_write(0x50, 8, num, wtbuffer);


	//trigger write;
	tmpbuf[0] = 0x80;
	i2c_write(0x50, 3, 1, tmpbuf);

	return;
}

void tbsfunc::REG64_rd(int subAddr, unsigned char num, unsigned char * rdbuffer)
{
	switch (typeId) {
	case 5301:
		usb_REG64_rd(subAddr, num, rdbuffer);
		break;
	default:
		break;
	}
	return;
}
void tbsfunc::REG64_wt(int subAddr, unsigned char num, unsigned char * wtbuffer)
{
	switch (typeId) {
	case 5301:
		usb_REG64_wt(subAddr, num, wtbuffer);
		break;
	default:
		break;
	}
	return;
}


bool tbsfunc::monopolizeCpu_r_REG32(int m_addr, unsigned char *rdbuff, unsigned char num)
{
	unsigned char i, buff[8];

	if (num == 0)
	{
		qDebug(" rd error! (num == 0) ");
		return 0;

	}
	else if (num > 8)
	{
		qDebug(" rd error! (num >8) ");
		return 0;

	}

	//...check host free -->......................
	if (Check_host_status(1) == 0)
	{
		qDebug(" wait for host free timeout!");
		return 0;
	}

	//...wt data & cs to low-->...................
	buff[0] = 0x33;
	buff[1] = num;
	buff[2] = (unsigned char)(m_addr >> 8);
	buff[3] = (unsigned char)(m_addr & 255);
	tbs_24cxx_wr(0 * 4, *(u32 *)buff);  //cs low;

										//...ck host received data & process-->........
	if (Check_host_status(0) == 0)
	{
		qDebug(" wait for host active timeout! ");
		//return 0;
	}

	//...cs high................................

	tbs_24cxx_wr(3 * 4, *(u32 *)buff);  //cs high;

										//...check host work done-->..................
	if (wait_for_host_workdone() == 0)
	{
		qDebug(" wait host work done timeout! ");
		return 0;
	}

	*(u32 *)buff = tbs_24cxx_rd(1 * 4);
	*(u32 *)(&buff[4]) = tbs_24cxx_rd(2 * 4);
	for (i = 0; i < num; i++)
		rdbuff[i] = buff[i];

	return 1;
}
bool tbsfunc::monopolizeCpu_r(int m_addr, unsigned char * rdbuff, unsigned char num)
{
	switch (typeId) {
	case 5301:
		return monopolizeCpu_r_REG64(m_addr, rdbuff, num);
		break;
	case 6304:
	case 6308:
		return monopolizeCpu_r_REG32(m_addr, rdbuff, num);
		break;
	default:
		break;
	}
	return false;
}




unsigned char tbsfunc::spi_CheckFree(unsigned char OpbyteNum)
{
	unsigned char tmpbuf[4];
	int i;
	int j = 500;
	if (OpbyteNum == 2)
		j = 400;
	else if (OpbyteNum == 1)
		j = 200;
	
	tmpbuf[0] = 0;
	for (i = 0; (i<j) && (tmpbuf[0] != 1); i++)
	{
		//*(u32 *)tmpbuf = tbs_read(GPIO_BASEADDRESS , SPI_STATUS );
		*(u32 *)tmpbuf = tbs_read(SPI_STATUS);

	}
	//return (tmpbuf[0] == 1);

	if (tmpbuf[0] == 1)
		return true;
	else
	{
		// qDebug("----------spi flash check error! %x, %x, %x, %x",tmpbuf[0], tmpbuf[1],tmpbuf[2],tmpbuf[3]);
		return false;
	}
}
void tbsfunc::spi_WriteEnable()
{
	unsigned char tmpbuf[4];
	//write enable:
	tmpbuf[0] = 0x06;
	tbs_write(SPI_COMMAND, *(u32 *)&tmpbuf[0]);
	tmpbuf[0] = 0xe0;
	tmpbuf[1] = 0x10;
	tbs_write(SPI_CONFIG, *(u32 *)&tmpbuf[0]);
	if (spi_CheckFree(1) == false)
	{
		//  qDebug(" spi_WriteEnable error!");
		return;
	}
	return;

}


void tbsfunc::spi_WriteDisable()
{
	unsigned char tmpbuf[4];
	//write disable:
	tmpbuf[0] = 0x04;
	tbs_write(SPI_COMMAND, *(u32 *)&tmpbuf[0]);

	tmpbuf[0] = 0xe0;
	tmpbuf[1] = 0x10;
	tbs_write(SPI_CONFIG, *(u32 *)&tmpbuf[0]);
	//spi_CheckFree(1);
	if (spi_CheckFree(1) == false)
	{
		//  qDebug(" spi_WriteDisable error!\n");
		return;
	}
	return;

}


unsigned char tbsfunc::spi_GetFlashID(unsigned char *flashID)
{
	unsigned char i;
	unsigned char tmpbuf[4] = { 0 };
	tmpbuf[0] = 1; //active spi bus from "z"
	tbs_write(SPI_ENABLE, *(u32 *)&tmpbuf[0]);
	//release from Deep Power-down mode:
	tmpbuf[0] = 0xab;
	tbs_write(SPI_COMMAND, *(u32 *)&tmpbuf[0]);
	tmpbuf[0] = 0xe0;
	tmpbuf[1] = 0x10;
	tbs_write(SPI_CONFIG, *(u32 *)&tmpbuf[0]);
	//spi_CheckFree(1);
	if (spi_CheckFree(1) == false)
	{
		//  qDebug(" spi_GetFlashID error 00 !\n");
		return 0;
	}
	tmpbuf[0] = 0x9f;  //write one byte "read identification command" ;
	tbs_write(SPI_COMMAND, *(u32 *)&tmpbuf[0]);
	tmpbuf[0] = 0x80;	//cs high to low;
	tmpbuf[0] += 0x40;  //cs low to high;
	tmpbuf[0] += 0x20;  //write command;
	tmpbuf[0] += 0x10;  // read 3 bytes ID
	tmpbuf[1] = 0x10;   //just one byte command for writing;
	tmpbuf[1] += 0x03;   //read 3 bytes ID;
	tbs_write(SPI_CONFIG, *(u32 *)&tmpbuf[0]);
	if (spi_CheckFree(1) == false)
	{
		qDebug(" spi read ID error!\n");
		return 0;
	}
	*(u32 *)tmpbuf = tbs_read(SPI_RD_DATA);
	for (i = 0; i<3; i++)
	{
		flashID[i] = tmpbuf[i];
		//qDebug("ID:0x%x\n",tmpbuf[i]);
	}

	// 20170110 add
	*(u32 *)tmpbuf = tbs_read(GPIO_VERSION);
	for (i = 0; i<4; i++)
	{
		flashID[i + 3] = tmpbuf[i];
		qDebug("version:0x%x\n", tmpbuf[i]);
	}
	//check flash id
	if ((flashID[0] == 0xc2) && (flashID[1] == 0x20) && (flashID[2] == 0x16))
		return 1;
	else if ((flashID[0] == 0xef) && (flashID[1] == 0x40) && (flashID[2] == 0x16))
		return 1;
	else
		return 0;

}

unsigned char tbsfunc::spi_ChipErase()
{
	unsigned char i, wip, tmpbuf[4];
	spi_WriteEnable();
	//Chip Erase:
	tmpbuf[0] = 0xc7;
	tbs_write(SPI_COMMAND, *(u32 *)&tmpbuf[0]);
	tmpbuf[0] = 0xe0;
	tmpbuf[1] = 0x10;
	tbs_write(SPI_CONFIG, *(u32 *)&tmpbuf[0]);
	Sleep(1);
	return 1;

}

unsigned char tbsfunc::spi_ChipEraseStatus()
{
	unsigned char wip, tmpbuf[4];
	//read status register for WIP bit;
	tmpbuf[0] = 0x05;
	tbs_write(SPI_COMMAND, *(u32 *)&tmpbuf[0]);
	tmpbuf[0] = 0xf0;
	tmpbuf[1] = 0x11;
	tbs_write(SPI_CONFIG, *(u32 *)&tmpbuf[0]);
	Sleep(2);
	*(u32 *)tmpbuf = tbs_read(SPI_RD_DATA);
	qDebug("read status register: %x\n", tmpbuf[0]);
	wip = tmpbuf[0] & 1;
	if (wip == 0)
		return 1;
	else
		return 0;

}


bool tbsfunc::spi_read32bit(unsigned char * addr24bit, u32 *data32bit)
{
	unsigned char i, tmpbuf[4];

	tmpbuf[0] = 0x03;  // read command ;
	tmpbuf[1] = addr24bit[0]; //read_address, msb first;
	tmpbuf[2] = addr24bit[1];
	tmpbuf[3] = addr24bit[2];
	tbs_write(SPI_COMMAND, *(u32 *)&tmpbuf[0]);

	tmpbuf[0] = 0xf0;	//cs low,cs high, write, read;
	tmpbuf[1] = 0x40;	// 4 bytes command for writing;
	tmpbuf[1] += 0x04;	 //read 4 bytes data;
	tbs_write(SPI_CONFIG, *(u32 *)&tmpbuf[0]);

	if (spi_CheckFree(2) == false)
	{
		//   qDebug(" spi_read32bit error!");
		return false;
	}

	*data32bit = tbs_read(SPI_RD_DATA);

	return true;

}

bool tbsfunc::spi_read1page(unsigned char * addr24bit, unsigned char *data256bytes)
{

	unsigned char i, tmpbuf[4];
	u32 addressTemp = *(u32 *)addr24bit;
	for (i = 0; i<64; i++)
	{

		*(u32 *)addr24bit = addressTemp + i * 4;
		tmpbuf[0] = 0x03;  // read command ;
		tmpbuf[1] = addr24bit[0]; //read_address, msb first;
		tmpbuf[2] = addr24bit[1];
		tmpbuf[3] = addr24bit[2];
		tbs_write(SPI_COMMAND, *(u32 *)&tmpbuf[0]);

		tmpbuf[0] = 0xf0;	//cs low,cs high, write, read;
		tmpbuf[1] = 0x40;	// 4 bytes command for writing;
		tmpbuf[1] += 0x04;	 //read 4 bytes data;
		tbs_write(SPI_CONFIG, *(u32 *)&tmpbuf[0]);

		if (spi_CheckFree(2) == false)
		{
			// qDebug(" spi_read32bit error!");
			return false;
		}

		*(u32*)&data256bytes[i * 4] = tbs_read(SPI_RD_DATA);
		// qDebug(" spi_read1page addr: %x\n  ( %6x)", *(u32 *)addr24bit,*(u32*)&data256bytes[i*4]);
		Sleep(1);
	}
	return true;

}


bool tbsfunc::spi_Write32bit(unsigned char * addr24bit, u32 *data32bit)
{
	unsigned char i, wip, tmpbuf[4];

	tmpbuf[0] = 0x02;  // PP;
	tmpbuf[1] = addr24bit[0]; //read_address, msb first;
	tmpbuf[2] = addr24bit[1];
	tmpbuf[3] = addr24bit[2];
	tbs_write(SPI_COMMAND, *(u32 *)&tmpbuf[0]);

	tbs_write(SPI_WT_DATA, *data32bit);

	tmpbuf[0] = 0xe0;	//cs low,cs high, write, no read;
	tmpbuf[1] = 0x80;	// 4 bytes command & 2 bytes data for writing;
	tbs_write(SPI_CONFIG, *(u32 *)&tmpbuf[0]);
	if (spi_CheckFree(2) == false)
	{
		//  qDebug(" spi_Write32bit 1 error!");
		return false;
	}

	//read status register for WIP bit;
	wip = 1;
	for (i = 0; (i<50)&wip; i++)
	{
		Sleep(1);  //delay one second;

		tmpbuf[0] = 0x05;
		tbs_write(SPI_COMMAND, *(u32 *)&tmpbuf[0]);
		tmpbuf[0] = 0xf0;
		tmpbuf[1] = 0x11;
		tbs_write(SPI_CONFIG, *(u32 *)&tmpbuf[0]);
		if (spi_CheckFree(1) == false)
		{
			//     qDebug(" spi_Write32bit 2 error!");
			return false;
		}
		*(u32 *)tmpbuf = tbs_read(SPI_RD_DATA);

		wip = tmpbuf[0] & 1;
	}
	if (i == 50)
		qDebug("write 16 bit-------------------------------------------------------- register: %x", tmpbuf[0]);
	return true;

}


bool tbsfunc::spi_Write1page(unsigned char * addr24bit, unsigned char *data256byte)
{
	unsigned char i, j, wip, tmpbuf[8];
	spi_WriteEnable();

	tmpbuf[0] = 0x02;  // PP;
	tmpbuf[1] = addr24bit[0]; //read_address, msb first;
	tmpbuf[2] = addr24bit[1];
	tmpbuf[3] = addr24bit[2];
	tbs_write(SPI_COMMAND, *(u32 *)&tmpbuf[0]);
	tbs_write(SPI_WT_DATA, *(u32 *)&data256byte[0]);

	tmpbuf[0] = 0xa0;	//cs low,cs high, write, no read;
	tmpbuf[1] = 0x80;	// 4 bytes command & 2 bytes data for writing;
	tbs_write(SPI_CONFIG, *(u32 *)&tmpbuf[0]);
	if (spi_CheckFree(2) == false)
	{
		//   qDebug(" spi_Write32bit 1 error!");
		return false;
	}

	for (j = 0; j<31; j++)
	{
		tmpbuf[0] = data256byte[4 + j * 8 + 0];
		tmpbuf[1] = data256byte[4 + j * 8 + 1];
		tmpbuf[2] = data256byte[4 + j * 8 + 2];
		tmpbuf[3] = data256byte[4 + j * 8 + 3];
		tmpbuf[4] = data256byte[4 + j * 8 + 4];
		tmpbuf[5] = data256byte[4 + j * 8 + 5];
		tmpbuf[6] = data256byte[4 + j * 8 + 6];
		tmpbuf[7] = data256byte[4 + j * 8 + 7];

		tbs_write(SPI_COMMAND, *(u32 *)&tmpbuf[0]);
		tbs_write(SPI_WT_DATA, *(u32 *)&tmpbuf[4]);

		tmpbuf[0] = 0x20;	//cs low,cs high, write, no read;
		tmpbuf[1] = 0x80;	// 4 bytes command & 2 bytes data for writing;
		tbs_write(SPI_CONFIG, *(u32 *)&tmpbuf[0]);
		if (spi_CheckFree(2) == false)
		{
			//    qDebug(" spi_Write32bit 1 error!");
			return false;
		}
	}


	tmpbuf[0] = data256byte[252];
	tmpbuf[1] = data256byte[253];
	tmpbuf[2] = data256byte[254];
	tmpbuf[3] = data256byte[255];
	tbs_write(SPI_COMMAND, *(u32 *)&tmpbuf[0]);

	tmpbuf[0] = 0x60;	//cs low,cs high, write, no read;
	tmpbuf[1] = 0x40;	// 4 bytes command & 2 bytes data for writing;
	tbs_write(SPI_CONFIG, *(u32 *)&tmpbuf[0]);
	if (spi_CheckFree(2) == false)
	{
		//  qDebug(" spi_Write32bit 1 error!");
		return false;
	}

	//read status register for WIP bit;
	wip = 1;
	for (i = 0; (i<250)&wip; i++)
	{
		//pauseThread(1);  //delay one second;

		tmpbuf[0] = 0x05;
		tbs_write(SPI_COMMAND, *(u32 *)&tmpbuf[0]);
		tmpbuf[0] = 0xf0;
		tmpbuf[1] = 0x11;
		tbs_write(SPI_CONFIG, *(u32 *)&tmpbuf[0]);
		if (spi_CheckFree(1) == false)
		{
			//    qDebug(" spi_Write32bit 2 error!");
			return false;
		}
		*(u32 *)tmpbuf = tbs_read(SPI_RD_DATA);

		wip = tmpbuf[0] & 1;
	}
	if (i == 250)
	{
		qDebug("write 16 bit-------------------------------------------------------- register: %x", tmpbuf[0]);
		return false;
	}
	return true;
}

void tbsfunc::spi_EnterDeepPowerDown()
{
	unsigned char tmpbuf[4];

	//enter  Deep Power-down mode:
	tmpbuf[0] = 0xb9;
	tbs_write(SPI_COMMAND, *(u32 *)&tmpbuf[0]);
	tmpbuf[0] = 0xe0;
	tmpbuf[1] = 0x10;
	tbs_write(SPI_CONFIG, *(u32 *)&tmpbuf[0]);
	//spi_CheckFree(1);
	if (spi_CheckFree(1) == false)
	{
		//  qDebug(" spi_EnterDeepPowerDown error!");
		return;
	}

	tmpbuf[0] = 0; //spi disable, enter "z" state;
	tbs_write(SPI_ENABLE, *(u32 *)&tmpbuf[0]);
	return;
}

void tbsfunc::mcu_software_reset()
{
	/* unsigned char tmpbuf[4];
	//write command:
	tmpbuf[0] = 0x03; //mcu software_reset;
	tmpbuf[1] = 0;  //num don't care
	tmpbuf[2] = 0;
	tmpbuf[3] = 0; //mcu address;

	tbs_24cxx_wr(  CMD_24CXX, *(u32 *)&tmpbuf[0] );
	check_mcuWorkOK();
	*/
	unsigned char tmpbuf[4];
	//...check host free -->......................
	if (Check_host_status(1) == 0)
	{
		qDebug(" wait for host free timeout! \n");
		return;
	}

	//...wt data & cs to low-->...................

	//write command:
	tmpbuf[0] = 0x03; //mcu software_reset;
	tmpbuf[1] = 0;  //num don't care
	tmpbuf[2] = 0;
	tmpbuf[3] = 0; //mcu address;
	tbs_24cxx_wr(CMD_24CXX, *(u32 *)&tmpbuf[0]); //cs low;

												 //...ck host received data & process-->........
	if (Check_host_status(0) == 0)
	{
		qDebug(" wait for host active timeout! \n");
		return;
	}

	//...cs high................................

	tbs_24cxx_wr(3 * 4, *(u32 *)tmpbuf);  //cs high;
										  //...check host work done-->..................
	if (wait_for_host_workdone() == 0)
	{
		qDebug(" wait host work done timeout! \n");
		return;
	}
	return;
}

void tbsfunc::erase_mcu(unsigned int sectorAddr)
{
	/*unsigned char tmpbuf[4];

	//write command:
	tmpbuf[0] = 0x00; //mcu erase;
	tmpbuf[1] = 0;  //num don't care
	tmpbuf[2] = (sectorAddr & 0xff00)>>8;
	tmpbuf[3] = (sectorAddr & 0xff); //mcu address;

	tbs_24cxx_wr( CMD_24CXX, *(u32 *)&tmpbuf[0] );
	check_mcuWorkOK();
	*/
	unsigned char tmpbuf[4];
	//...check host free -->......................
	if (Check_host_status(1) == 0)
	{
		qDebug(" wait for host free timeout! \n");
		return;
	}

	//...wt data & cs to low-->...................

	//write command:
	tmpbuf[0] = 0x00; //mcu erase;
	tmpbuf[1] = 0;  //num don't care
	tmpbuf[2] = (sectorAddr & 0xff00) >> 8;
	tmpbuf[3] = (sectorAddr & 0xff); //mcu address;

	tbs_24cxx_wr(CMD_24CXX, *(u32 *)&tmpbuf[0]); //cs low;

												 //...ck host received data & process-->........
	if (Check_host_status(0) == 0)
	{
		qDebug(" wait for host active timeout! \n");
		return;
	}

	//...cs high................................

	tbs_24cxx_wr(3 * 4, *(u32 *)tmpbuf);  //cs high;
										  //...check host work done-->..................
	if (wait_for_host_workdone() == 0)
	{
		qDebug(" wait host work done timeout! \n");
		return;
	}
	return;
}

// write 8 byte every time to mcu
void tbsfunc::write_mcu(unsigned int subAddr, unsigned char *wtbuffer)
{
	/*  unsigned char tmpbuf[4];
	//prepare data;
	tbs_24cxx_wr(  DATA0_24CXX, *(u32 *)&wtbuffer[0] );
	tbs_24cxx_wr(  DATA1_24CXX, *(u32 *)&wtbuffer[4]);
	//write command:
	tmpbuf[0] = 0x02; // mcu write;
	tmpbuf[1] = 0x08;  //cmd num
	tmpbuf[2] = (subAddr & 0xff00)>>8;
	tmpbuf[3] =(subAddr & 0xff); //mcu address;

	tbs_24cxx_wr(  CMD_24CXX, *(u32 *)&tmpbuf[0] );
	check_mcuWorkOK();
	*/
	unsigned char tmpbuf[4];
	//...check host free -->......................
	if (Check_host_status(1) == 0)
	{
		qDebug(" wait for host free timeout! \n");
		return;
	}

	//...wt data & cs to low-->...................

	tbs_24cxx_wr(DATA0_24CXX, *(u32 *)&wtbuffer[0]);
	tbs_24cxx_wr(DATA1_24CXX, *(u32 *)&wtbuffer[4]);
	//write command:
	tmpbuf[0] = 0x02; // mcu write;
	tmpbuf[1] = 0x08;  //cmd num
	tmpbuf[2] = (subAddr & 0xff00) >> 8;
	tmpbuf[3] = (subAddr & 0xff); //mcu address;

	tbs_24cxx_wr(CMD_24CXX, *(u32 *)&tmpbuf[0]); //cs low;

												 //...ck host received data & process-->........
	if (Check_host_status(0) == 0)
	{
		qDebug(" wait for host active timeout! \n");
		return;
	}

	//...cs high................................

	tbs_24cxx_wr(3 * 4, *(u32 *)tmpbuf);  //cs high;
										  //...check host work done-->..................
	if (wait_for_host_workdone() == 0)
	{
		qDebug(" wait host work done timeout! \n");
		return;
	}
	return;
}

void tbsfunc::read_mcu(unsigned int subAddr, unsigned char *rdbuffer)
{
	/*unsigned char tmpbuf[4];

	tmpbuf[0] = 0x01; //mcu read;
	tmpbuf[1] = 8;  //how many byte;
	tmpbuf[2] = (subAddr  & 0xff00)>>8;
	tmpbuf[3] = (subAddr & 0xff); //mcu address;

	tbs_24cxx_wr(  CMD_24CXX, *(u32 *)&tmpbuf[0]);
	//wait... until the data are received correctly;
	check_mcuWorkOK();

	//read back to host;
	*(u32 *)rdbuffer = tbs_24cxx_rd(  DATA0_24CXX );
	*(u32 *)&rdbuffer[4] = tbs_24cxx_rd(  DATA1_24CXX );
	*/
	unsigned char tmpbuf[4];
	//...check host free -->......................
	if (Check_host_status(1) == 0)
	{
		qDebug(" wait for host free timeout! \n");
		return;
	}

	//...wt data & cs to low-->...................

	//write command:
	tmpbuf[0] = 0x01; //mcu read;
	tmpbuf[1] = 8;  //how many byte;
	tmpbuf[2] = (subAddr & 0xff00) >> 8;
	tmpbuf[3] = (subAddr & 0xff); //mcu address;

	tbs_24cxx_wr(CMD_24CXX, *(u32 *)&tmpbuf[0]);//cs low;

												//...ck host received data & process-->........
	if (Check_host_status(0) == 0)
	{
		qDebug(" wait for host active timeout! \n");
		return;
	}

	//...cs high................................

	tbs_24cxx_wr(3 * 4, *(u32 *)tmpbuf);  //cs high;
										  //...check host work done-->..................
	if (wait_for_host_workdone() == 0)
	{
		qDebug(" wait host work done timeout! \n");
		return;
	}
	//read back to host;
	*(u32 *)rdbuffer = tbs_24cxx_rd(DATA0_24CXX);
	*(u32 *)&rdbuffer[4] = tbs_24cxx_rd(DATA1_24CXX);
	return;
}

unsigned char tbsfunc::read_mcu_status()
{
	unsigned char tmpbuf[4];

	*(u32 *)tmpbuf = tbs_24cxx_rd(STATUS_MAC16_24CXX);
	//qDebug("mcu read status: %d, %d,%d,%d", tmpbuf[0],tmpbuf[1],tmpbuf[2],tmpbuf[3]);
	return (tmpbuf[0] & 0x4); // bit2==1 mcu busy
}


void tbsfunc::restartHdmi()
{
	u8 tmpbuf[4] = { 0 };
	target_ext_memory_rd_wt(WRITE, ADDRESS_HDMI(tuner) + 0x08, tmpbuf, 1);//
}


void tbsfunc::write_M23Register(int num, int* m23_addr, int* m23_data)
{
	unsigned char tmpM23addr[4];
	unsigned char tmpM23data[4];
	int i = 0;

	for (i = 0; i < num; i++)
	{
		tmpM23addr[0] = (*(m23_addr + i) & 0xff00) >> 8;
		tmpM23addr[1] = *(m23_addr + i) & 0xff;
		tmpM23addr[2] = 0;
		tmpM23addr[3] = 0;


		tmpM23data[0] = (*(m23_data + i) & 0xff00) >> 8;
		tmpM23data[1] = *(m23_data + i) & 0xff;
		tmpM23data[2] = 0;
		tmpM23data[3] = 0;

		//write the address of register
		// write_24cxx( 0x20+i*4,2, tmpM23addr);
		target_ext_memory_rd_wt(WRITE, 0x80 * tuner + 0x20 + i * 4, tmpM23addr, 2);
		//write the data of register
		// write_24cxx(0x22+i*4,2, tmpM23data);
		target_ext_memory_rd_wt(WRITE, 0x80 * tuner + 0x22 + i * 4, tmpM23data, 2);
		qDebug("addr:%04x  data:%04x", *(u16*)tmpM23addr, *(u16*)tmpM23data);

	}

	return;
}

void tbsfunc::read_M23Register(int num, int* m23_data)
{
	unsigned char tmpM23data[4];
	int i = 0;

	for (i = 0; i < num; i++)
	{
		//read the data from register
		//read_24cxx( 0x22+i*4,2, tmpM23data);
		target_ext_memory_rd_wt(READ, 0x80 * tuner + 0x22 + i * 4, tmpM23data, 2);
		*(m23_data + i) = tmpM23data[1] & 0xff;
		*(m23_data + i) += ((tmpM23data[0] << 8) & 0xff00);
	}

	return;
}


u32 tbsfunc::tbs_read(u8 address)
{
	if (6308 == typeId) {
		if (tunercpy >= 4) {
			return rdReg32_Extern(0x0000 + address);
		}
	}
	return rdReg32(0x0000+address);
}

BOOL tbsfunc::tbs_write(u8 address, u32 data)
{
	if (6308 == typeId) {
		if (tunercpy >= 4) {
			return wrReg32_Extern(0x0000 + address, data);
		}
	}
	return wrReg32(0x0000+address,data);
}

u32 tbsfunc::tbs_24cxx_rd(u8 address)
{
	if (6308 == typeId) {
		if (tunercpy >= 4) {
			//qDebug("tbs_24cxx_rd");
			return rdReg32_Extern(0x4000 + address);
		}
	}
	return rdReg32(0x4000 + address);
}

BOOL tbsfunc::tbs_24cxx_wr(u8 address, u32 data)
{
	if (6308 == typeId) {
		if (tunercpy >= 4) {
		//	qDebug("tbs_24cxx_wr");
			return wrReg32_Extern(0x4000 + address, data);
		}
	}
	return wrReg32(0x4000 + address,data);
}

BOOL tbsfunc::Check_host_status(unsigned char cs)  //2 seconds timeout;
{
	unsigned char j, tmp[4];
	int i;
	int count = 0;
	
	j = 1;
	for (i = 0; ((i < 4000) && j); i++)
	{
		*(u32 *)tmp = tbs_24cxx_rd(0 * 4);
		//qDebug("tmp:%#x", *(u32 *)tmp);
		
		if ((tmp[0] & 1) != cs)
		{
			j = 1;
			Sleep(1); //delay 500uS;
						 //        count++;
		}
		else
			j = 0;
	}
	//   qDebug("ctime: %d us\n", count*500);
	return (j != 4000);
}

//===............................................................................

BOOL tbsfunc::wait_for_host_workdone()  //3 seconds timeout;
{
	unsigned char j, tmp[4];
	int i;
	int count = 0;
	j = 0;
	for (i = 0; ((i < 600) && (j == 0)); i++)
	{
		*(u32 *)tmp = tbs_24cxx_rd(0 * 4);
		j = (tmp[0] & 1);
		if (j == 0) {
			Sleep(5);  //delay 5mS
						   //   count ++;
		}
	}

	//    qDebug("wtime: %d ms\n", count*5);
	return (j != 600);
}

//===............................................................................

BOOL tbsfunc::ext_one_time_wt(int m_addr, unsigned char *wtbuff, unsigned char num) {
	switch (typeId) {
	case 5301:
		return write_to_external_memory(m_addr, num, wtbuff);
		break;
	case 6304:
	case 6308:
		return ext_one_time_wt_REG32(m_addr, wtbuff, num);
		break;
	default:
		break;
	}
	return 0;
}

BOOL tbsfunc::ext_one_time_rd(int m_addr, unsigned char *rdbuff, unsigned char num) {
	switch (typeId) {
	case 5301:
		return read_from_external_memory(m_addr, num, rdbuff);
		break;
	case 6304:
	case 6308:
		return ext_one_time_rd_REG32(m_addr, rdbuff, num);
		break;
	default:
		break;
	}
	return 0;
}


BOOL tbsfunc::ext_one_time_wt_REG32(int m_addr, unsigned char *wtbuff, unsigned char num)
{
	unsigned char i, buff[8];

	if (num == 0)
	{
		qDebug(" wt error! (num == 0) ");
		return FALSE;

	}
	else if (num > 8)
	{
		qDebug(" wt error! (num >8) ");
		return FALSE;

	}
	//...check host free -->......................
	if (Check_host_status(1) == 0)
	{
		qDebug(" wait for host free timeout! ");
		return FALSE;
	}

	//...wt data & cs to low-->...................
	for (i = 0; i < num; i++)
		buff[i] = wtbuff[i];
	tbs_24cxx_wr(2 * 4, *(u32 *)&buff[4]);
	tbs_24cxx_wr(1 * 4, *(u32 *)buff);

	buff[0] = 0x30;
	buff[1] = num;
	buff[2] = (unsigned char)(m_addr >> 8);
	buff[3] = (unsigned char)(m_addr & 255);
	tbs_24cxx_wr(0 * 4, *(u32 *)buff);  //cs low;
	Sleep(1);
	//...ck host received data & process-->........
	if (Check_host_status(0) == 0)
	{
		qDebug(" wait for host active timeout!");
		tbs_24cxx_wr(3 * 4, *(u32 *)buff);  //cs high;
		return FALSE;
	}

	//...cs high................................

	tbs_24cxx_wr(3 * 4, *(u32 *)buff);  //cs high;

										//...check host work done-->..................
	if (wait_for_host_workdone() == 0)
	{
		qDebug(" wait host work done timeout!");
		return FALSE;
	}
	return TRUE;
}

BOOL tbsfunc::ext_one_time_rd_REG32(int m_addr, unsigned char *rdbuff, unsigned char num)
{
	unsigned char i, buff[8];

	if (num == 0)
	{
		qDebug(" rd error! (num == 0)  ");
		return FALSE;

	}
	else if (num > 8)
	{
		qDebug(" rd error! (num >8) ");
		return FALSE;

	}

	//...check host free -->......................
	if (Check_host_status(1) == 0)
	{
		qDebug(" wait for host free timeout!");
		return FALSE;
	}

	//...wt data & cs to low-->...................
	buff[0] = 0x31;
	buff[1] = num;
	buff[2] = (unsigned char)(m_addr >> 8);
	buff[3] = (unsigned char)(m_addr & 255);
	tbs_24cxx_wr(0 * 4, *(u32 *)buff);  //cs low;
	Sleep(1);
	//...ck host received data & process-->........
	if (Check_host_status(0) == 0)
	{
		qDebug(" wait for host active timeout! ");
		tbs_24cxx_wr(3 * 4, *(u32 *)buff);  //cs high;
		return FALSE;
	}

	//...cs high................................

	tbs_24cxx_wr(3 * 4, *(u32 *)buff);  //cs high;

										//...check host work done-->..................
	if (wait_for_host_workdone() == 0)
	{
		qDebug(" wait host work done timeout!");
		return FALSE;
	}

	*(u32 *)buff = tbs_24cxx_rd(1 * 4);
	*(u32 *)(&buff[4]) = tbs_24cxx_rd(2 * 4);
	for (i = 0; i < num; i++)
		rdbuff[i] = buff[i];

	return TRUE;
}
//===............................................................................

BOOL tbsfunc::ext_ram_write(int ram_addr, unsigned char *wtbff, int wt_size)
{
	int i, k, x8byte;
	unsigned char finishbyte;
	x8byte = wt_size / 8;
	finishbyte = (unsigned char)(wt_size & 7);

	//first send & remainder bytes.
	if (finishbyte == 0)
	{
		finishbyte = 8;
		x8byte -= 1;
	}
	ext_one_time_wt(ram_addr, wtbff, finishbyte);

	i = finishbyte;
	if (x8byte != 0)
	{
		for (k = 0; k < x8byte; k++)
			ext_one_time_wt((ram_addr + i + k * 8), &wtbff[i + k * 8], 8);
	}

	return TRUE;
}

//===............................................................................



//===............................................................................

BOOL tbsfunc::ext_ram_read(int ram_addr, unsigned char *rdbff, int rd_size)
{
	int i, k, x8byte;
	unsigned char finishbyte, buff[8];
	// qDebug("ram_addr:%#x\n",ram_addr);
	x8byte = rd_size / 8;
	finishbyte = (unsigned char)(rd_size & 7);
	//first send & remainder bytes.
	if (finishbyte == 0)
	{
		finishbyte = 8;
		x8byte -= 1;
	}
	ext_one_time_rd(ram_addr, rdbff, finishbyte);

	i = finishbyte;
	if (x8byte != 0)
	{
		for (k = 0; k < x8byte; k++)
			ext_one_time_rd((ram_addr + i + k * 8), &rdbff[i + k * 8], 8);
	}

	return TRUE;
}

unsigned char tbsfunc::check_host_free_REG64()
{
	unsigned char tmp_buff[8];
	int i = 0;

	do
	{
		REG64_rd(MAP_FPGA_BASEADDR + HOST_STATUS, 1, tmp_buff);
		//qDebug() <<"tmpbf = "<< tmp_buff[0];
		if ((tmp_buff[0] & 4) == 4)
		{
			Sleep(2);
			i++;
		}
	} while (((tmp_buff[0] & 4) == 4) && (i<1500));  //3 second timeout;

	if (i == 1500)
		return 0;  //host busy and timeout;
	else
		return 1;
}
//--------------------------------------------------------------------------------------------

unsigned char tbsfunc::Check_host_status_REG64(unsigned char status)
{
	unsigned char tmp_buff[8] = { 0 };
	int i = 0;

	do
	{
		REG64_rd(MAP_FPGA_BASEADDR + HOST_STATUS, 8, tmp_buff);
		//if (1 == networkflg) {
		//	for(int i = 0;i < 8;i++)
		//	qDebug("tmp_buff[%d]:%#02x", i,tmp_buff[i]);
		//}
		if ((tmp_buff[0] & 1) != status)
		{
			Sleep(2);
			i++;
		}

	} while (((tmp_buff[0] & 1) != status) && (i<1500));  //3 second timeout;

	if (i == 1500)
		return 0;  // timeout;
	else
		return 1;
}
//--------------------------------------------------------------------------------------------

unsigned char tbsfunc::read_from_external_memory(int addr, unsigned char num, unsigned char *buff)
{
	unsigned char i, tmp_buff[8];

	if (num == 0)
	{
		qDebug(" read_from_external_memory parameter error! [num == 0]");
		return 0;
	}
	else if (num > 8)
	{
		qDebug(" read_from_external_memory parameter error! [num >8]");
		return 0;
	}
	for (i = 0; i < num; i++)
		buff[i] = 0xff;

	//check_host_free;...............................
	if (check_host_free_REG64() == 0)
	{
		qDebug(" wait for host free timeout! ");
		//return 0;
	}

	//read tag;
	tmp_buff[0] = 0x31;
	tmp_buff[1] = num;
	tmp_buff[2] = (unsigned char)(addr >> 8);
	tmp_buff[3] = (unsigned char)(addr & 255);
	REG64_wt(MAP_FPGA_BASEADDR + WRITE_CMD, 4, tmp_buff);

	//check_host_active;...............................
	if (Check_host_status_REG64(0) == 0)
	{
		qDebug(" wait for host active timeout!");
		//return 0;
	}

	//release cs;
	REG64_wt(MAP_FPGA_BASEADDR + RELEASE_CS, 1, tmp_buff);


	//check host work done;...........................
	if (Check_host_status_REG64(1) == 0)
	{
		qDebug("wait for host work done timeout!");
		//return 0;
	}
	//read back;
	REG64_rd(MAP_FPGA_BASEADDR + READ_DATA, num, buff);

	return 1;
}
//--------------------------------------------------------------------------------------------

unsigned char tbsfunc::write_to_external_memory(int addr, unsigned char num, unsigned char *buff)
{
	unsigned char i, tmp_buff[8];

	if (num > 8)
	{
		qDebug(" write_to_external_memory parameter error! [num >8]");
		return 0;
	}

	//check_host_free;...............................
	if (check_host_free_REG64() == 0)
	{
		qDebug(" wait for host free timeout!");
		return 0;
	}

	if (num != 0)
	{
		for (i = 0; i < num; i++)
			tmp_buff[i] = buff[i];
		REG64_wt(MAP_FPGA_BASEADDR + WRITE_DATA, num, tmp_buff);
	}

	//write tag;
	tmp_buff[0] = 0x30;
	tmp_buff[1] = num;
	tmp_buff[2] = (unsigned char)(addr >> 8);
	tmp_buff[3] = (unsigned char)(addr & 255);
	REG64_wt(MAP_FPGA_BASEADDR + WRITE_CMD, 4, tmp_buff);

	//check_host_active;...............................
	if (Check_host_status_REG64(0) == 0)
	{
		qDebug(" wait for host active timeout!");
		//return 0;
	}

	//release cs;
	REG64_wt(MAP_FPGA_BASEADDR + RELEASE_CS, 1, tmp_buff);
	//0x11 ?= 0x55 5s
	if (5301 == typeId) {
		if (-1 == checkI2cBusStatus()) {
			//	return 0;
		}
	}
	if (Check_host_status_REG64(1) == 0)
	{
		qDebug("wait for host work done timeout!");
		return 0;
	}
	return 1;
}



BOOL tbsfunc::target_ext_memory_rd_wt(unsigned char tg_mode,
	int tg_addr,
	unsigned char *buff,
	int tg_size)
{
	if (tg_mode == READ)
		ext_ram_read(tg_addr, buff, tg_size);
	else if (tg_mode == WRITE)
		ext_ram_write(tg_addr, buff, tg_size);
	else
		qDebug("[target_external_memory_rd_wt] read write mode error!");
	return TRUE;
}
bool tbsfunc::wrReg32_Extern(u32 uAddr, u32 udata)
{
	int i = 0;
	u32 tmp = ((uAddr & 0xff00) >> 8) | ((uAddr & 0x00ff) << 8);
	wrReg32(0x1004, udata);
	wrReg32(0x1000, (((tmp << 16) & 0xffff0000) + 0x00000000));
	while (0xff == rdReg32(0x1000)) {
		if (10 == i) {
			qDebug("wrReg32_Extern:rdReg32(0x1000) time out");
			return 0;
		}
		i++;
	}
	return 0;
}

u32 tbsfunc::rdReg32_Extern(u32 uAddr)
{
	int i = 0;
	u32 tmp = ((uAddr & 0xff00) >> 8) | ((uAddr & 0x00ff) << 8);
	//qDebug("uaddr:%#x", (((*(u32*)tmp << 16) & 0xffff0000) + 0x00000080));
	wrReg32(0x1000, (((tmp << 16) & 0xffff0000) + 0x00000080));
	while (0xff == rdReg32(0x1000)) {
		if (10 == i) {
			qDebug("rdReg32_Extern:rdReg32(0x1000) time out");
			return 0;
		}
		i++;
	}
	return rdReg32(0x1004);
}

int tbsfunc::usb_checkI2cBusStatus()
{
	int ret = 0;
	u8 tmp_buff[8] = { 0 };
	int i = 0;
	for (i = 0; i < 5000; i++) {
		i2c_read(I2C_CHIP_ADDRESS, CHECK_I2C_BUS_ADDR, 1, tmp_buff);
		if (0x55 == tmp_buff[0]) {
			ret = 0;
			break;
		}
		else {
			Sleep(1);
		}
	}
	if (5000 == i) {
		ret = -1;
		qDebug() << "check i2c bus status time out!!!";
	}
	return ret;
}

int tbsfunc::checkI2cBusStatus()
{
	switch (typeId) {
	case 5301:
		return usb_checkI2cBusStatus();
		break;
	default:
		break;
	}
	return -1;
}

DEFINE_GUID(CLSID_DestFilter,
	0x90d647a7, 0xdfbb, 0x48f9, 0x86, 0x1f, 0x33, 0xf3, 0x66, 0xc, 0x3, 0xfd);

CDestFilter::CDestFilter(LPUNKNOWN pUnk,
	CCritSec *pLock,
	HRESULT *phr) :
	CBaseFilter("CDestFilter", pUnk, pLock, CLSID_DestFilter)

{
	m_pPin = new CDumpInputPin(this, GetOwner(), this, pLock, phr);

}
//
// GetPin
//
CBasePin * CDestFilter::GetPin(int n)
{
	if (n == 0) {
		return (CBasePin *)m_pPin;
	}
	else {
		return NULL;
	}
}


//
// GetPinCount
//
int CDestFilter::GetPinCount()
{
	return 1;
}

STDMETHODIMP CDestFilter::NonDelegatingQueryInterface(REFIID riid, void ** ppv) {
	CheckPointer(ppv, E_POINTER);

	// Do we have this interface

	if (riid == IID_IBaseFilter || riid == IID_IMediaFilter || riid == IID_IPersist) {
		return CBaseFilter::NonDelegatingQueryInterface(riid, ppv);
	};

	return CUnknown::NonDelegatingQueryInterface(riid, ppv);

};


//
// Receive
//
// Do something with this media sample
//
STDMETHODIMP CDumpInputPin::Receive(IMediaSample *pSample)
{
	if (!m_bOpened) return S_OK;

	CAutoLock lock(m_pReceiveLock);
	PBYTE pbData;
	HRESULT hr = pSample->GetPointer(&pbData);
	//HRESULT hr = pSample->GetPointer(&&pbData);
	if (FAILED(hr))
	{
		return hr;
	}

	int size = pSample->GetActualDataLength();
	if (0 == isSendTs) {
		u32 maxfs = MaxFileSize * 1024;
		if (1 == IsSizeOrTimeFlg) {
			KbCount = KbCount + (size / 1024);
			if (KbCount > maxfs) {
				size = size - (KbCount - maxfs) * 1024;
				KbCount = maxfs;
			}
		}else if (0 == IsSizeOrTimeFlg) {
			KbCount = KbCount + (size / 1024);
		}
		fwrite(pbData, 1, size, w_file_fd);
	}else {
		int i = 0;
		//PBYTE tsData;
		int remainder = size % (188 * 7);
		int quotient = size / (188 * 7);
		if (0 == isRtp) {	//udp
			for (i = 0; i < quotient; i++) {
				//memcpy(&tsData[0], &pbData[i* 188 * 7], (188 * 7));
				//g_udp->Write(&tsData[0], (UINT16)(188*7) );
				g_udp->Write(&pbData[i * 188 * 7], (UINT16)(188 * 7));
			}
			//memcpy(&tsData[0], &pbData[i * 188 * 7], remainder);
			//g_udp->Write(&tsData[0], (UINT16)remainder);
			if (0 != remainder) {
				g_udp->Write(&pbData[i * 188 * 7], (UINT16)remainder);
			}
				
		}else { 	//rtp
			for (i = 0; i < quotient; i++) {
				//memcpy(&tsData[0], &pbData[i * 188 * 7], (188 * 7));
				//g_rtp->Write(&tsData[0], (UINT16)(188*7), 0x21, GetTickCount());
				g_rtp->Write(&pbData[i * 188 * 7], (UINT16)(188 * 7), 0x21, GetTickCount());
			}
			//memcpy(&tsData[0], &pbData[i * 188 * 7], remainder);
			//g_rtp->Write(&tsData[0], (UINT16)remainder, 0x21, GetTickCount());
			if (0 != remainder) {
				g_rtp->Write(&pbData[i * 188 * 7], (UINT16)remainder, 0x21, GetTickCount());
			}
		}
	}

	if (0 == isSendTs) {
		if ((0 == IsSizeOrTimeFlg) || (1 == IsSizeOrTimeFlg)) {
			mode = 7;
		}
	}
	return  S_OK;
}

//
// CheckMediaType
//
// Check if the pin can support this specific proposed type and format
//
HRESULT CDumpInputPin::CheckMediaType(const CMediaType *type)
{
	return S_OK;
}



HRESULT CDumpInputPin::GetMediaType(
	int iPosition,
	CMediaType *pMediaType)
{

	if (iPosition < 0)
	{
		return E_INVALIDARG;
	}
	if (iPosition > 0)
	{
		return VFW_S_NO_MORE_ITEMS;
	}

	AM_MEDIA_TYPE type;
	ZeroMemory(&type, sizeof(type));
	type.bFixedSizeSamples = 1;
	type.lSampleSize = 188;
	type.majortype = MEDIATYPE_Stream;
	type.subtype = MEDIASUBTYPE_NULL;

	pMediaType->Set(type);

	return S_OK;
}
//
// BreakConnect
//
// Break a connection
//
HRESULT CDumpInputPin::BreakConnect()
{
	return CBaseInputPin::BreakConnect();
}

//
// EndOfStream
//
STDMETHODIMP CDumpInputPin::EndOfStream(void)
{
	CAutoLock lock(m_pReceiveLock);
	return CBaseInputPin::EndOfStream();

} 
// EndOfStream
  //
  // ReceiveCanBlock
  //
  // We don't hold up source threads on Receive
  //
STDMETHODIMP CDumpInputPin::ReceiveCanBlock()
{
	return S_FALSE;
}

  //
  // NewSegment
  //
  // Called when we are seeked
  //
STDMETHODIMP CDumpInputPin::NewSegment(REFERENCE_TIME tStart,
	REFERENCE_TIME tStop,
	double dRate)
{
	return S_OK;

} // NewSegment



///////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Definition of CDumpInputPin
//

CDumpInputPin::CDumpInputPin(CDestFilter *pDump,
	LPUNKNOWN pUnk,
	CBaseFilter *pFilter,
	CCritSec *pLock,
	HRESULT *phr) :

	CBaseInputPin((char*)"CDestInputPin",
		pFilter,                   // Filter
		pLock,                     // Locking
		phr,                       // Return code
		L"Input"),                 // Pin name
	m_pReceiveLock(pLock),
	m_pFilter(pDump)

{
	m_tmpBufferLen = 0;
	m_bOpened = false;

}
