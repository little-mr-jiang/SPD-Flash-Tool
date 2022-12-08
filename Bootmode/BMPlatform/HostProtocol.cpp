#pragma once
#include "stdafx.h"
#include "HostProtocol.h"
#include <exception>

CHostProtocol::CHostProtocol(void)
{
	m_pFileBuf = NULL;
	m_dwFileSize = 0;
	m_nDataSize = 0;
	m_dwMaxLength = MAX_HOST_DATA_PACKET_SIZE;
	m_pPackageBuf = (LPBYTE)GetMemory(MAX_HOST_PACKAGE_LEN);
	m_pRingBuf = (LPBYTE)GetMemory(MAX_HOST_PACKAGE_LEN);
}

CHostProtocol::~CHostProtocol(void)
{
	FreeMemory(m_pFileBuf);
	FreeMemory(m_pPackageBuf);
	FreeMemory(m_pRingBuf);
}

LPVOID	CHostProtocol::GetMemory( DWORD dwSize)
{
	if( dwSize == 0)
	{
		return NULL;
	}

	LPBYTE lpBuf = NULL;
	try
	{
		lpBuf = new BYTE[dwSize];
	}
	catch (const std::bad_alloc& /*e*/)
	{
		lpBuf = NULL;
	}

	return lpBuf;

}

void 	CHostProtocol::FreeMemory( LPBYTE& lpMemBlock )
{
	if(lpMemBlock != NULL)
	{
		delete[] lpMemBlock;
		lpMemBlock = NULL;
	}
}

BOOL CHostProtocol::CheckPackageMemory()
{
	if (NULL==m_pPackageBuf)
	{
		m_pPackageBuf = (LPBYTE)GetMemory(MAX_HOST_PACKAGE_LEN);
	}
	return m_pPackageBuf ? TRUE : FALSE;
}

/** 
* @brief               <p> ����һ������������С </p>
* @param int nDataLen  <p> ���е����ݴ�С </p>
* @return value int    <p> ���Ĵ�С </p>
*/
int CHostProtocol::Calc_Packet_Size(int nDataLen)
{
	int nSizes = 0;
	nSizes += PACKET_PACKETHEAD_SZIE;
	nSizes += PACKET_DATAHEAD_SIZE;
	nSizes += nDataLen;
	return nSizes;
}

/** 
* @brief                    <p> ��װconnect���� </p>
* @param LPBYTE pDestBuff   <p> Ŀ�껺���� </p>
* @param int nBuffLen       <p> Ŀ�껺�����Ĵ�С </p>
* @param value BOOL         <p> TRUE/FALSE </p>
*/
BOOL CHostProtocol::Assemble_Packet_Connect(LPBYTE& pDestBuff, int &nBuffLen)
{
	m_nDataSize = 0;
	int nPkgSizes = Calc_Packet_Size(0);
	if (!CheckPackageMemory() || nPkgSizes > MAX_HOST_PACKAGE_LEN)
	{
		return FALSE;
	}

	T_Packet PktConnect;
	memset((void *)&PktConnect, 0x00, sizeof(T_Packet));
	DeclareHostHeader(PktConnect.PkgHeader,0);
	DeclareHostData(PktConnect.PkgPdl,CMD_CONNECT,0x00,0,NULL);

	/// ��װ����
	Assemble_Packet(PktConnect, m_pPackageBuf, 0);
	pDestBuff = m_pPackageBuf;
	nBuffLen = nPkgSizes;
	return TRUE;
}


/** 
* @brief                  <p> ��װһ�����ݰ� </p>
* @param int nPacketNo    <p> ���ݰ���� </p>
* @param LPBYTE pDestBuff <p> ������ݰ���Ŀ�껺���� </p>
* @param int nDestBuffLen <p> Ŀ�껺�����Ĵ�С </p>
* @param value BOOL         <p> TRUE/FALSE </p>
*/
BOOL CHostProtocol::Assemble_Packet_Data(int nPacketNo, LPBYTE& pDestBuff, int& nDestBuffLen)
{
    m_nDataSize = 0;
	LPBYTE pDataBuf = NULL; //m_pFileBuf�е�ƫ��
	int nDataLen = 0;
	
	/// ��ȡ�ð��е����ݻ��漰��С
    if ( !Get_Packet_FDL1(nPacketNo, pDataBuf, nDataLen))
    {
		return FALSE;
    }

	/// �������ݰ��Ĵ�С
	int nPkgSizes = Calc_Packet_Size(nDataLen);
	if (!CheckPackageMemory() || nPkgSizes > MAX_HOST_PACKAGE_LEN)
	{
		return FALSE;
	}
	T_Packet PktConnect;
	memset((void *)&PktConnect, 0x00, sizeof(T_Packet));
	DeclareHostHeader(PktConnect.PkgHeader,nDataLen);
	DeclareHostData(PktConnect.PkgPdl,CMD_MID_DATA,nPacketNo,nDataLen,pDataBuf);

	/// ��װ����
	Assemble_Packet(PktConnect, m_pPackageBuf, nDataLen);
	pDestBuff = m_pPackageBuf;
	nDestBuffLen = nPkgSizes;
	return TRUE;
}

/** 
* @brief                  <p> ��װ��ʼ���ݰ� </p>
* @param LPBYTE pDestBuff <p> ������ݰ���Ŀ�껺���� </p>
* @param int nDestBuffLen <p> Ŀ�껺�����Ĵ�С </p>
* @param value BOOL         <p> TRUE/FALSE </p>
*/
BOOL CHostProtocol::Assemble_Packet_Start(LPBYTE& pDestBuff, int &nBuffLen)
{
    m_nDataSize = 0;

#define MAX_START_FLAG_LEN (5)
	BYTE StartFlag[MAX_START_FLAG_LEN] = {0x50, 0x44, 0x4c, 0x31, 0x00};      //< "PDL1"
	int nPkgSizes = Calc_Packet_Size(MAX_START_FLAG_LEN);
	if (!CheckPackageMemory() || nPkgSizes > MAX_HOST_PACKAGE_LEN)
	{
		return FALSE;
	}

	T_Packet PktConnect;
	memset((void *)&PktConnect, 0x00, sizeof(T_Packet));
	DeclareHostHeader(PktConnect.PkgHeader,MAX_START_FLAG_LEN);
	DeclareHostData(PktConnect.PkgPdl,CMD_START_DATA,m_dwAddress,m_dwFileSize,StartFlag);

	/// ��װ����
	Assemble_Packet(PktConnect, m_pPackageBuf, MAX_START_FLAG_LEN);
	pDestBuff = m_pPackageBuf;
	nBuffLen = nPkgSizes;
	return TRUE;
}

/** 
* @brief                  <p> ��װ�������ݰ� </p>
* @param LPBYTE pDestBuff <p> ������ݰ���Ŀ�껺���� </p>
* @param int nDestBuffLen <p> Ŀ�껺�����Ĵ�С </p>
* @param value BOOL         <p> TRUE/FALSE </p>
*/
BOOL CHostProtocol::Assemble_Packet_End(LPBYTE& pDestBuff, int &nBuffLen)
{
    m_nDataSize = 0;

#define MAX_END_FLAG_LEN (4)
	BYTE EndFlag[MAX_END_FLAG_LEN] = {0x1c, 0x3c, 0x6e, 0x06};
	int nPkgSizes = Calc_Packet_Size(MAX_END_FLAG_LEN);
	if (!CheckPackageMemory() || nPkgSizes > MAX_HOST_PACKAGE_LEN)
	{
		return FALSE;
	}

	T_Packet PktConnect;
	memset((void *)&PktConnect, 0x00, sizeof(T_Packet));
	DeclareHostHeader(PktConnect.PkgHeader,MAX_END_FLAG_LEN);
	DeclareHostData(PktConnect.PkgPdl,CMD_END_DATA,0,0,EndFlag);

	/// ��װ����
	Assemble_Packet(PktConnect, m_pPackageBuf, MAX_END_FLAG_LEN);
	pDestBuff = m_pPackageBuf;
	nBuffLen = nPkgSizes;
	return TRUE;
}


/** 
* @brief                  <p> ��װִ�����ݰ� </p>
* @param LPBYTE pDestBuff <p> ������ݰ���Ŀ�껺���� </p>
* @param int nDestBuffLen <p> Ŀ�껺�����Ĵ�С </p>
* @param value BOOL         <p> TRUE/FALSE </p>
*/
BOOL CHostProtocol::Assemble_Packet_Execute(LPBYTE& pDestBuff, int &nBuffLen)
{
	m_nDataSize = 0;
	int nPkgSizes = Calc_Packet_Size(0);
	if (!CheckPackageMemory() || nPkgSizes > MAX_HOST_PACKAGE_LEN)
	{
		return FALSE;
	}

	T_Packet PktConnect;
	memset((void *)&PktConnect, 0x00, sizeof(T_Packet));
	DeclareHostHeader(PktConnect.PkgHeader,0);
	DeclareHostData(PktConnect.PkgPdl,CMD_EXEC_DATA,0x00,0,NULL);

	/// ��װ����
	Assemble_Packet(PktConnect, m_pPackageBuf, 0);
	pDestBuff = m_pPackageBuf;
	nBuffLen = nPkgSizes;
	return TRUE;
}

/** 
* @brief                     <p> ��װ���ְ� </p>
* @param T_Packet& sPacket   <p> ���ݰ��Ļ�����Ϣ </p>
* @param LPBYTE pDestBuff    <p> ������ݵ�Ŀ�껺���� </p>
* @param int nBuffLen        <p> ���ݰ��Ĵ�С </p>
*/
void CHostProtocol::Assemble_Packet(const T_Packet& sPacket, LPBYTE pDestBuff, int nBuffLen)
{
    LPBYTE pDst = pDestBuff;
	int nCpyLen = sizeof(T_Packet)-4;
	memcpy(pDst,&sPacket,nCpyLen);
	pDst += nCpyLen;
	if(sPacket.PkgPdl.pData && nBuffLen)
	{
		memcpy(pDst, sPacket.PkgPdl.pData, nBuffLen);
	}
}

/** 
* @brief                    <p> ��������Ӧ���� </p>
* @param LPBYTE pDataBuff   <p> Դ���ݻ����� </p>
* @param int nDataSize      <p> �������е����ݴ�С </p>
* @param value BOOL         <p> Ӧ��OK������TRUE�����򣬷���FALSE </p>
*/
BOOL CHostProtocol:: Decode_Response(LPBYTE pDataBuff, int nDataSize)
{
	//[2019-03-22 18:24:17:957] <<- 12(0x0000000c)/12(0x0000000c) Bytes
	//00000000h: AE 04 00 00 00 FF 00 00 00 00 00 00             ; ............
	
	if (NULL == m_pRingBuf)
	{
		return FALSE;
	}
	if (m_nDataSize+nDataSize > MAX_HOST_PACKAGE_LEN)
	{
		m_nDataSize = 0;
	}
	memcpy(m_pRingBuf+m_nDataSize,pDataBuff,nDataSize);
	m_nDataSize += nDataSize;
	if (m_nDataSize<PACKET_PACKETHEAD_SZIE)
	{
		return FALSE;
	}
	
	BOOL bOK = FALSE;
    int nOffset = 0;
	int nLeft = m_nDataSize;
	while(nLeft > 0)
	{
		if (nLeft < PACKET_PACKETHEAD_SZIE)
		{
			break;
		}
		T_Packet_Header   PkgHeader;
		memcpy(&PkgHeader,m_pRingBuf+nOffset,sizeof(T_Packet_Header));
		if (
			PkgHeader.ucTag == PACKET_FLAG_TAG && 
			PkgHeader.ucFlowID == PACKET_FLAG_FLOWID &&
			PkgHeader.wReserved == 0x00
			)
		{
			nOffset += PACKET_PACKETHEAD_SZIE;
			nLeft -= PACKET_PACKETHEAD_SZIE;
			if(nLeft < 4)
			{
				break;
			}
			if (RSP_ACK == *((DWORD *)(m_pRingBuf + nOffset)))
			{
				bOK = TRUE;
			}
			nOffset += 4;
			nLeft -= 4;
			memmove(m_pRingBuf,m_pRingBuf+nOffset,nLeft);
			break;

		}
		else
		{
			++nOffset;
			--nLeft;
		}
		
	}
	return bOK;

}

BOOL CHostProtocol::SetFdlFileInfo(const BMFileInfo* pBMFileInfo)
{
	BOOL bOK = FALSE;
	do 
	{
		if(NULL == pBMFileInfo || NULL == pBMFileInfo->hFDLCode || 0 == pBMFileInfo->llCodeSize )
		{
			break;
		}

		LARGE_INTEGER liOffset;
		liOffset.QuadPart = pBMFileInfo->llCodeOffset;
		if (::SetFilePointer(pBMFileInfo->hFDLCode,liOffset.LowPart, &liOffset.HighPart,FILE_BEGIN) == INVALID_SET_FILE_POINTER)
		{
			break;
		}

		FreeMemory(m_pFileBuf);
		DWORD dwTmpSize = (DWORD)(pBMFileInfo->llCodeSize);
		m_pFileBuf = (LPBYTE)GetMemory(dwTmpSize);
		if(NULL == m_pFileBuf)
		{
			break;
		}

		DWORD dwReaded = 0;
		if(!::ReadFile(pBMFileInfo->hFDLCode,m_pFileBuf,dwTmpSize,&dwReaded,NULL) || dwReaded != dwTmpSize)
		{
			FreeMemory(m_pFileBuf);
			break;
		}

		m_dwFileSize = (DWORD)(pBMFileInfo->llCodeSize);
		m_dwAddress = (DWORD)(pBMFileInfo->llBase);
		m_dwMaxLength = pBMFileInfo->dwMaxLength ?  pBMFileInfo->dwMaxLength : MAX_HOST_DATA_PACKET_SIZE;
		bOK = TRUE;

	} while (0);
	return bOK;
}

/** 
* @brief      <p> ���㷢�͵����ݰ��� </p>
*/
int CHostProtocol::CountPackets()
{
	int nPacketCount = 0;

	if(m_dwFileSize == 0 || NULL == m_pFileBuf || 0 == m_dwMaxLength)
	{
		return nPacketCount;
	}

	nPacketCount = ((m_dwFileSize)% m_dwMaxLength ) ? ((m_dwFileSize)/m_dwMaxLength + 1) : ((m_dwFileSize)/m_dwMaxLength) ;

	return nPacketCount;
}


/** 
* @brief                      <p> ��ȡһ�����ݰ������� <p>
* @param int nPacketNo        <p> ���ݰ������ </p>
* @param LPBYTE pDestBuff     <p> ������ݰ���Ŀ�껺���� </p>
* @param int nDestBuffLen     <p> Ŀ�껺�����Ĵ�С </p>
* @return value int           <p> ���ݰ��Ĵ�С </p>
*/
BOOL CHostProtocol::Get_Packet_FDL1(int nPacketNo, LPBYTE& pDestBuff, int& nDestBuffLen)
{
	int nPacketNums = CountPackets();                       //< ���͵��ܰ���
	if(nPacketNums == 0 || nPacketNo < 0 || nPacketNo >= nPacketNums || 0 == m_dwMaxLength)
	{
		return FALSE;
	}

	/// �������ݰ��Ĵ�С
	if(nPacketNo < nPacketNums - 1)
	{
		nDestBuffLen = m_dwMaxLength;
	}
	else
	{
		nDestBuffLen = m_dwFileSize - nPacketNo * m_dwMaxLength; 
	}
	/// ���ݰ���FDL1�е�ƫ�Ƶ�ַ
	pDestBuff = m_pFileBuf + nPacketNo * m_dwMaxLength;

	return TRUE;
}


