// GPL80_ESCPOS_USB.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <windows.h>
#include <string>
#include <IOSTREAM>
#include <winioctl.h>
#include <setupapi.h>

#pragma comment(lib, "setupapi.lib")

using namespace std;

// SetupDiGetInterfaceDeviceDetail所需要的输出长度，定义足够大
#define INTERFACE_DETAIL_SIZE    (1024)

//设备数量上限，假设16台上限
#define MAX_DEVICE 16


//USB类的GUID
const GUID USB_GUID = { 0xa5dcbf10, 0x6530, 0x11d2, { 0x90, 0x1f, 0x00, 0xc0, 0x4f, 0xb9, 0x51, 0xed } };

int GetDevicePath(LPGUID lpGuid, LPTSTR* pszDevicePath);

////////////////////////////////////////////////////////////////////////////////////////////////////////
//获取CreateFile的USB端口号
////////////////////////////////////////////////////////////////////////////////////////////////////////

// 根据GUID获得设备路径
// lpGuid: GUID指针
// pszDevicePath: 设备路径指针的指针，用于返回找到的路径
// 返回: 成功得到的设备路径个数，可能不止1个
int GetDevicePath(LPGUID lpGuid, LPTSTR* pszDevicePath)
{
	HDEVINFO hDevInfoSet;
	SP_DEVINFO_DATA spDevInfoData;
	SP_DEVICE_INTERFACE_DATA ifData;
	PSP_DEVICE_INTERFACE_DETAIL_DATA pDetail;
	int nCount;
	int nTotle;
	BOOL bResult;

	TCHAR* strUSBPrint = TEXT("USB 打印支持");

	// 取得一个该GUID相关的设备信息集句柄
	hDevInfoSet = ::SetupDiGetClassDevs(lpGuid,     // class GUID 
		NULL,                    // 无关键字 
		NULL,                    // 不指定父窗口句柄 
		DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);    // 目前存在的设备

	// 失败...
	if (hDevInfoSet == INVALID_HANDLE_VALUE)
	{
		printf("failed \r\n");

		return 0;
	}

	// 申请设备接口数据空间
	pDetail = (PSP_DEVICE_INTERFACE_DETAIL_DATA)::GlobalAlloc(LMEM_ZEROINIT, INTERFACE_DETAIL_SIZE);

	pDetail->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

	nTotle = -1;
	nCount = 0;
	bResult = TRUE;

	// 设备序号=0,1,2... 逐一测试设备接口，到失败为止
	while (bResult)
	{
		nTotle++;
		spDevInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

		// 枚举符合该GUID的设备接口
		bResult = ::SetupDiEnumDeviceInfo(
			hDevInfoSet,     // 设备信息集句柄
			(ULONG)nTotle,   // 设备信息集里的设备序号
			&spDevInfoData);        // 设备接口信息

		if (bResult)
		{
			DWORD DataT;
			TCHAR buf[MAX_PATH];
			DWORD nSize = 0;

			// get Friendly Name or Device Description
			if (SetupDiGetDeviceRegistryProperty(hDevInfoSet, &spDevInfoData,
				SPDRP_FRIENDLYNAME, &DataT, (PBYTE)buf, sizeof(buf), &nSize)) {
			}
			else if (SetupDiGetDeviceRegistryProperty(hDevInfoSet, &spDevInfoData,
				SPDRP_DEVICEDESC, &DataT, (PBYTE)buf, sizeof(buf), &nSize)) {
			}
			else {
				lstrcpy(buf, _T("Unknown"));
			}

			_tprintf(_T("buf = %s \r\n"), buf);
			//是否是要找的设备类型
			if (_tcscmp(buf, strUSBPrint) != 0)
				continue;

			_tprintf(_T("OK\r\n"));

			ifData.cbSize = sizeof(ifData);

			// 枚舉符合該GUID的設備接口
			bResult = ::SetupDiEnumDeviceInterfaces(
				hDevInfoSet,     // 設備信息集句柄
				NULL,            // 不需額外的設備描述
				lpGuid,          // GUID
				(ULONG)nTotle,   // 設備信息集里的設備序號
				&ifData);        // 設備接口信息

			if (bResult)
			{
				// 取得该设备接口的细节(设备路径)
				bResult = SetupDiGetInterfaceDeviceDetail(
					hDevInfoSet,    // 设备信息集句柄
					&ifData,        // 设备接口信息
					pDetail,        // 设备接口细节(设备路径)
					INTERFACE_DETAIL_SIZE,    // 输出缓冲区大小
					NULL,           // 不需计算输出缓冲区大小(直接用设定值)
					NULL);          // 不需额外的设备描述

				if (bResult)
				{
					// 复制设备路径到输出缓冲区
					::_tcscpy_s(pszDevicePath[nCount], 256, pDetail->DevicePath);
					// 调整计数值
					nCount++;
					_tprintf(_T("Cnt = %d,pDetail->DevicePath =%s\r\n"), nCount, pDetail->DevicePath);
				}
			}
		}
	}

	// 释放设备接口数据空间
	::GlobalFree(pDetail);

	// 关闭设备信息集句柄
	::SetupDiDestroyDeviceInfoList(hDevInfoSet);

	return nCount;
}


HANDLE hPort = NULL;  //句柄


int   WriteData(string meg)
{
	DWORD dwWrite;
	return WriteFile(hPort, meg.c_str(), (DWORD)meg.length(), &dwWrite, NULL);
}

int WriteBuf(char *buf, int len)
{
	DWORD dwWrite;
	return WriteFile(hPort, buf, len, &dwWrite, NULL);
}

int POS_Reset(void)
{
	string s;
	s = "\x1B\x40";
	WriteData(s);

	return 0;
}

int POS_FeedLine(void)
{
	string s;
	s = "\x0A";
	WriteData(s);

	return 0;

}

int POS_SetMotionUnit(int x, int y)
{
	string s;
	s = "\x1D\x50\xB4\xB4";
	WriteData(s);s
	s = "\x1B\x53";
	WriteData(s);

	return 0;
}

int POS_S_TextOut(string &abc)
{
	string s;

	char SetAbsPos[4] = { 0x1B, 0x24, 0x46, 0x00 };
	WriteBuf(SetAbsPos, 4);


	char SelctFontType[3] = { 0x1B, 0x4D, 0x03 };
	WriteBuf(SelctFontType, 3);

	char SelctOutMode[3] = { 0x1D, 0x21, 0x00 };

	WriteBuf(SelctOutMode, 3);


	WriteData(abc);

	return 0;
}

int POS_CutPaper()
{

	char CutPaperMode[4] = { 0x1D, 0x56, 0x41, 0x00 };

	WriteBuf(CutPaperMode, 4);

	return 0;
}

int POS_OutQRCode()
{
	char  QRCode1[8] = { 0x1d, 0x28, 0x6b, 0x03, 0x00, 0x31, 0x43, 0x05 };

	char  QRCode2[16] = { 0x1d, 0x28, 0x6b, 0x0b, 0x00, 0x31, 0x50, 0x30, 0x47, 0x70, 0x72, 0x69,
		0x6e, 0x74, 0x65, 0x72 };

	char  QRCode3[8] = { 0x1d, 0x28, 0x6b, 0x03, 0x00, 0x31, 0x51, 0x30 };


	WriteBuf(QRCode1, 8);

	WriteBuf(QRCode2, 16);

	WriteBuf(QRCode3, 8);

	return 0;
}

int _tmain(int argc, _TCHAR* argv[])
{

	//遍历USB设备，找到POS打印机路径
	int i, nDevice;
	TCHAR * szDevicePath[MAX_DEVICE];        // 设备路径

	//  string Port;  //串口端口号
	setlocale(LC_CTYPE, "chs");//设置中文字符

	TCHAR * Port;

	// 分配需要的空间
	for (i = 0; i < MAX_DEVICE; i++)
	{
		szDevicePath[i] = new TCHAR[256];
	}

	// 取设备路径
	nDevice = GetDevicePath((LPGUID)&USB_GUID, szDevicePath);

	i = 0;


	while (i < nDevice){

		Port = szDevicePath[i++];

		_tprintf(_T("device.Port = %s\n"), Port);
	}


	hPort = CreateFile(Port, GENERIC_READ | GENERIC_WRITE,
		0, NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL, NULL);

	if (hPort == INVALID_HANDLE_VALUE)
	{   // 打开端口失败
		return false;
	}


	DWORD iBytesLength;
	string s;

	POS_Reset();
	POS_FeedLine();
	POS_FeedLine();

	POS_SetMotionUnit(180, 180);

	s = "你好";
	POS_S_TextOut(s);
	POS_FeedLine();

	s = "123abc";
	POS_S_TextOut(s);
	POS_FeedLine();

	s = "666";
	POS_S_TextOut(s);
	POS_FeedLine();
	POS_FeedLine();

	POS_OutQRCode();

	//s = "777";
	//POS_S_TextOut(s);

	POS_FeedLine();
	POS_FeedLine();

	POS_FeedLine();
	POS_FeedLine();

	POS_CutPaper();




	return 0;
}