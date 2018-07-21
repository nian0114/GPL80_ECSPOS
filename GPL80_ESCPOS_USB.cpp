// GPL80_ESCPOS_USB.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include <windows.h>
#include <string>
#include <IOSTREAM>
#include <winioctl.h>
#include <setupapi.h>

#pragma comment(lib, "setupapi.lib")

using namespace std;

// SetupDiGetInterfaceDeviceDetail����Ҫ��������ȣ������㹻��
#define INTERFACE_DETAIL_SIZE    (1024)

//�豸�������ޣ�����16̨����
#define MAX_DEVICE 16


//USB���GUID
const GUID USB_GUID = { 0xa5dcbf10, 0x6530, 0x11d2, { 0x90, 0x1f, 0x00, 0xc0, 0x4f, 0xb9, 0x51, 0xed } };

int GetDevicePath(LPGUID lpGuid, LPTSTR* pszDevicePath);

////////////////////////////////////////////////////////////////////////////////////////////////////////
//��ȡCreateFile��USB�˿ں�
////////////////////////////////////////////////////////////////////////////////////////////////////////

// ����GUID����豸·��
// lpGuid: GUIDָ��
// pszDevicePath: �豸·��ָ���ָ�룬���ڷ����ҵ���·��
// ����: �ɹ��õ����豸·�����������ܲ�ֹ1��
int GetDevicePath(LPGUID lpGuid, LPTSTR* pszDevicePath)
{
	HDEVINFO hDevInfoSet;
	SP_DEVINFO_DATA spDevInfoData;
	SP_DEVICE_INTERFACE_DATA ifData;
	PSP_DEVICE_INTERFACE_DETAIL_DATA pDetail;
	int nCount;
	int nTotle;
	BOOL bResult;

	TCHAR* strUSBPrint = TEXT("USB ��ӡ֧��");

	// ȡ��һ����GUID��ص��豸��Ϣ�����
	hDevInfoSet = ::SetupDiGetClassDevs(lpGuid,     // class GUID 
		NULL,                    // �޹ؼ��� 
		NULL,                    // ��ָ�������ھ�� 
		DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);    // Ŀǰ���ڵ��豸

	// ʧ��...
	if (hDevInfoSet == INVALID_HANDLE_VALUE)
	{
		printf("failed \r\n");

		return 0;
	}

	// �����豸�ӿ����ݿռ�
	pDetail = (PSP_DEVICE_INTERFACE_DETAIL_DATA)::GlobalAlloc(LMEM_ZEROINIT, INTERFACE_DETAIL_SIZE);

	pDetail->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

	nTotle = -1;
	nCount = 0;
	bResult = TRUE;

	// �豸���=0,1,2... ��һ�����豸�ӿڣ���ʧ��Ϊֹ
	while (bResult)
	{
		nTotle++;
		spDevInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

		// ö�ٷ��ϸ�GUID���豸�ӿ�
		bResult = ::SetupDiEnumDeviceInfo(
			hDevInfoSet,     // �豸��Ϣ�����
			(ULONG)nTotle,   // �豸��Ϣ������豸���
			&spDevInfoData);        // �豸�ӿ���Ϣ

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
			//�Ƿ���Ҫ�ҵ��豸����
			if (_tcscmp(buf, strUSBPrint) != 0)
				continue;

			_tprintf(_T("OK\r\n"));

			ifData.cbSize = sizeof(ifData);

			// ö�e����ԓGUID���O��ӿ�
			bResult = ::SetupDiEnumDeviceInterfaces(
				hDevInfoSet,     // �O����Ϣ�����
				NULL,            // �����~����O������
				lpGuid,          // GUID
				(ULONG)nTotle,   // �O����Ϣ������O����̖
				&ifData);        // �O��ӿ���Ϣ

			if (bResult)
			{
				// ȡ�ø��豸�ӿڵ�ϸ��(�豸·��)
				bResult = SetupDiGetInterfaceDeviceDetail(
					hDevInfoSet,    // �豸��Ϣ�����
					&ifData,        // �豸�ӿ���Ϣ
					pDetail,        // �豸�ӿ�ϸ��(�豸·��)
					INTERFACE_DETAIL_SIZE,    // �����������С
					NULL,           // ������������������С(ֱ�����趨ֵ)
					NULL);          // ���������豸����

				if (bResult)
				{
					// �����豸·�������������
					::_tcscpy_s(pszDevicePath[nCount], 256, pDetail->DevicePath);
					// ��������ֵ
					nCount++;
					_tprintf(_T("Cnt = %d,pDetail->DevicePath =%s\r\n"), nCount, pDetail->DevicePath);
				}
			}
		}
	}

	// �ͷ��豸�ӿ����ݿռ�
	::GlobalFree(pDetail);

	// �ر��豸��Ϣ�����
	::SetupDiDestroyDeviceInfoList(hDevInfoSet);

	return nCount;
}


HANDLE hPort = NULL;  //���


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

	//����USB�豸���ҵ�POS��ӡ��·��
	int i, nDevice;
	TCHAR * szDevicePath[MAX_DEVICE];        // �豸·��

	//  string Port;  //���ڶ˿ں�
	setlocale(LC_CTYPE, "chs");//���������ַ�

	TCHAR * Port;

	// ������Ҫ�Ŀռ�
	for (i = 0; i < MAX_DEVICE; i++)
	{
		szDevicePath[i] = new TCHAR[256];
	}

	// ȡ�豸·��
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
	{   // �򿪶˿�ʧ��
		return false;
	}


	DWORD iBytesLength;
	string s;

	POS_Reset();
	POS_FeedLine();
	POS_FeedLine();

	POS_SetMotionUnit(180, 180);

	s = "���";
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