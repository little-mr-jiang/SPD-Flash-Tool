#pragma once
#include "Global.h"

enum pdl_cmd_type {
	CMD_CONNECT,            //< ׼������
	CMD_ERASE_FLASH,        //< ����ĳһ��flash
	CMD_ERASE_PARTITION,    //< ����һ��flash mtd����
	CMD_ERASE_ALL,          //< ��������flash
	CMD_START_DATA,         //< ��ʼdownload����
	CMD_MID_DATA,           //< �м�����
	CMD_END_DATA,           //< �������
	CMD_EXEC_DATA,          //< ִ�и����ݣ���Ҫ��PDL��
	CMD_READ_FLASH,         //< read flash
	CMD_READ_PARTITIONS,    //< read ĳһ������, ����������product info��
	CMD_NORMAL_RESET,       //< reset phone
	CMD_READ_CHIPID,        //< ��chipid
	CMD_SET_BAUDRATE,       //< option�����usbת����û�����壬�������Ĵ�����Ч��
};


enum pdl_rsp {
	RSP_ACK,
	
	/// from PC command
	RSP_INVALID_CMD,
	RSP_UNKNOWN_CMD,
	RSP_INVALID_ADDR,
	RSP_INVALID_BAUDRATE,
	RSP_INVALD_PARTITION,
	RSP_SIZE_ERROR,
	RSP_WAIT_TIMEOUT,
	
	/// from phone
	RSP_VERIFY_ERROR,
	RSP_CHECKSUM_ERROR,
	RSP_OPERATION_FAILED,
	
	/// phone internal
	RSP_DEVICE_ERROR, //DDR,NAND init errors
	RSP_NO_MEMORY
};

static TCHAR *ErrInfo[] = {
    _T("INVALID_CMD"),
	_T("UNKNOWN_CMD"),
	_T("INVALID_ADDR"),
	_T("INVALID_BAUDRATE"),
	_T("INVALD_PARTITION"),
	_T("SIZE_ERROR"),
	_T("WAIT_TIMEOUT"),
	_T("VERIFY_ERROR"),
	_T("CHECKSUM_ERROR"),
	_T("OPERATION_FAILED"),
	_T("DEVICE_ERROR"),
	_T("NO_MEMORY"),
};

#define PACKET_MIN_SIEZ				(12)
#define PACKET_FLAG_TAG				(0xAE)
#define PACKET_FLAG_FLOWID			(0xFF)
#define PACKET_PACKETHEAD_SZIE		(8)                 //< ��ͷ�Ĵ�С
#define PACKET_DATAHEAD_SIZE		(12)                //< ����ͷ�Ĵ�С
#define FDL1_ADDRESS				(0x008000C0)
#define MAX_HOST_DATA_PACKET_SIZE	(2048)              //< һ�����е����ݴ�С

#define MAX_HOST_PACKAGE_LEN		(0x10000)	

#pragma pack(push, 1)
typedef struct T_Packet_Header_Tag
{
	BYTE ucTag;     //< 0xAE
	int  nDataSize; //< data size
	BYTE ucFlowID;  //< 0xFF
	WORD wReserved; //< reserved
}T_Packet_Header, *pT_Packet_Header;


typedef struct T_PDL_Data_Packet_Tag
{
     DWORD dwCmdType;     //< ��������
	 DWORD dwDataAddr;    //< ���ݵ�ַ
	 DWORD dwDataSize;    //< ���ݴ�С �� ��ʼ���������ļ���С
	 BYTE  *pData;        //< ����
}T_PDL_Data_Packet, *pT_PDL_Data_Packet;


typedef struct T_Packet_Tag
{
     T_Packet_Header   PkgHeader;
     T_PDL_Data_Packet PkgPdl;
	 T_Packet_Tag()
	 {
		 memset(this,0x0,sizeof(T_Packet));
	 }
}T_Packet, *pT_Packet;
#pragma pack(pop)

#define DeclareHostHeader(_PkgHeader, _DataLen)     \
	_PkgHeader.ucTag     = PACKET_FLAG_TAG;\
	_PkgHeader.nDataSize = PACKET_DATAHEAD_SIZE+_DataLen;\
	_PkgHeader.ucFlowID  = PACKET_FLAG_FLOWID;\
	_PkgHeader.wReserved = 0x00;


#define DeclareHostData(_PkgData, _Cmd,_Addr,_Size,_pData)    \
	_PkgData.dwCmdType  = _Cmd;\
	_PkgData.dwDataAddr = _Addr;\
	_PkgData.dwDataSize = _Size;\
	_PkgData.pData      = _pData;


class CHostProtocol
{
public:
	CHostProtocol(void);
	virtual ~CHostProtocol(void);
	/// ����FDL1���ļ���Ϣ
	BOOL SetFdlFileInfo(const BMFileInfo* pBMFileInfo);

    /// ��װconnect����
	BOOL Assemble_Packet_Connect(LPBYTE& pDestBuff, int& nBuffLen); 

	/// ��װ��ʼ���ݰ�
	BOOL Assemble_Packet_Start(LPBYTE& pDestBuff, int& nBuffLen);

	/// ��װһ�����ݰ�
	BOOL Assemble_Packet_Data(int nPacketNo, LPBYTE& pDestBuff, int& nDestBuffLen); 

	/// ��װ�������ݰ�
    BOOL Assemble_Packet_End(LPBYTE& pDestBuff, int& nBuffLen);

	/// ��װִ�����ݰ�
	BOOL Assemble_Packet_Execute(LPBYTE& pDestBuff, int &nBuffLen); 

	/// ����Ӧ������
	BOOL Decode_Response(LPBYTE pDataBuff, int nDataSize);  

	/// �����ļ����͵İ���
    int CountPackets();


private:
	/// ����һ���������Ĵ�С
	int Calc_Packet_Size(int nDataLen);

	/// ��ȡһ�����ݰ��а���������
	BOOL Get_Packet_FDL1(int nPacketNo, LPBYTE& pDestBuff, int& nDestBuffLen);

	/// ��װ���ݰ�
    void Assemble_Packet(const T_Packet& sPacket, LPBYTE pDestBuff, int nBuffLen);

	LPVOID	GetMemory(DWORD dwSize);
	void    FreeMemory(LPBYTE& lpMemBlock);
	BOOL	CheckPackageMemory();

private:
	LPBYTE	m_pFileBuf;                  //< ����FDL1�ļ����ݻ�����
	DWORD   m_dwFileSize;                //< ����FDL1�ļ���С
	DWORD	m_dwAddress;                 //< ��д��ַ
	DWORD   m_dwMaxLength;
	LPBYTE	m_pPackageBuf;               //< hostЭ�������

	LPBYTE			m_pRingBuf;			 // Internal buffer 
	unsigned int	m_nDataSize;		 // Internal data size
};
