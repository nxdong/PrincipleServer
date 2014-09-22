/********************************************************************
	created:	2014/09/01
	created:	1:9:2014   22:50
	filename: 	Buffer.cpp
	author:		Principle
	
	purpose:	This file achieve CBuffer class.
*********************************************************************/
#include "stdafx.h"
#include "Buffer.h"

CBuffer::CBuffer()
{
	m_size = 0;
	m_sendPacket = NULL;
	m_recvPacket = NULL;
	m_secondFlag = NULL;
	m_firstFlag  = NULL;
	m_nDataSize  = NULL;
	m_data		 = NULL;
	m_nUnCompressSize	= NULL;
	m_nCompressSize		= NULL;
	m_nDataAllSize		= NULL;
	memset(m_header,0,sizeof(m_header));
}
CBuffer::~CBuffer()
{
	delete m_data;
	delete m_sendPacket;
	delete m_recvPacket;
}

void CBuffer::AddFirstFlag(BYTE &bFirstFlag)
{
	m_firstFlag = bFirstFlag;
	return;
}
void CBuffer::AddSecondFlag(BYTE &bSecondFlag)
{
	m_secondFlag = bSecondFlag;
	return;
}
void CBuffer::AddFlags(BYTE &bFirstFlag,BYTE &bSecondFlag)
{
	m_firstFlag = bFirstFlag;
	m_secondFlag = bSecondFlag;
	return;
}
void CBuffer::AddData(const BYTE *const pData,UINT nSize)
{
	AllocateBuffer(m_data,nSize);
	memcpy(m_data,pData,nSize);
	m_nDataSize = nSize;
	return;
}
BOOL CBuffer::AllocateBuffer(PBYTE pBuffer,UINT nNeedSize)
{
	pBuffer = (PBYTE) VirtualAlloc(NULL,nNeedSize,MEM_COMMIT,PAGE_READWRITE);
	if (NULL == pBuffer)
	{
		return FALSE;
	}
	return TRUE;
}
void CBuffer::Fresh()
{
	m_size = 0;
	m_sendPacket = NULL;
	m_recvPacket = NULL;
	m_secondFlag = NULL;
	m_firstFlag  = NULL;
	m_nDataSize  = NULL;
	m_data		 = NULL;
	m_nUnCompressSize	= NULL;
	m_nCompressSize		= NULL;
	m_nDataAllSize		= NULL;
	memset(m_header,0,sizeof(m_header));
}
BOOL CBuffer::PreparePackate()
{
	UINT nCompressSize;
	// size of data and two flags 
	nCompressSize = m_nDataSize + 1 + 1;  
	PBYTE pbCompressSource = new BYTE[nCompressSize];
	// copy first flag to source compress
	memcpy(pbCompressSource,(const void*)m_firstFlag,1);   
	// copy second flag to source compress
	memcpy(pbCompressSource+1,(const void*)m_secondFlag,1); 
	// copy data to source compress
	memcpy(pbCompressSource+2,(const void*)m_data,m_nDataSize);

	//prepare aim information.
	ULONG	ndestLen = (double)nCompressSize * 1.001  + 12;
	LPBYTE	pDest = new BYTE[ndestLen];

	int	nRet = compress(pDest, &ndestLen, pbCompressSource, nCompressSize);
	if (nRet != Z_OK)
	{
		delete []pbCompressSource;
		delete [] pDest;
		return FALSE;
	}
	//  uncompressed data size
	m_nUnCompressSize = nCompressSize;   
	//	compressed data size
	m_nCompressSize = ndestLen;
	// m_size is the size of the all buffer . when send ,it contains compressed data
	m_size = 12 + ndestLen;
	m_sendPacket = new BYTE [m_size];
	BYTE header[] = {'p','r','i','n'};
	memcpy(m_header,header,sizeof(header));

	// 4 is the length of header p r i n
	memcpy(m_sendPacket,m_header,4);
	// move the pos 8 , size of UINT is 4 bytes. 
	memcpy(m_sendPacket+4,(const void*)m_nUnCompressSize,4);
	// move the pos 4 , size of UINT is 4 bytes. 
	memcpy(m_sendPacket+8,(const void*)m_nCompressSize,4);
	// move the pos 12, size of compressed data
	memcpy(m_sendPacket+12,(const void*)pDest,m_nCompressSize);

	// don't use ,delete pDest
	delete  [] pbCompressSource;
	delete	[] pDest;
	return TRUE;
}
BOOL CBuffer::GetInformation()
{
	// get header from recvPacket
	memcpy(m_header,m_recvPacket,4);
	// get 	m_nDataSize  UINT 4 bytes
	memcpy((void*)m_nUnCompressSize,m_recvPacket+4,4);
	// get m_nCompressSize ,  UINT 4 bytes
	memcpy((void*)m_nCompressSize,m_recvPacket+8,4);

	// get compressed data 
	PBYTE pbCompressedData = new BYTE[m_nCompressSize];
	memcpy((void*)pbCompressedData,m_recvPacket+12,m_nCompressSize);

	// create unCompressed pointer
	PBYTE UnCompressedData = new BYTE[m_nUnCompressSize];
	ULONG nUncompressSize;
	// uncompress data
	int nRet = uncompress(UnCompressedData,&nUncompressSize,pbCompressedData,m_nCompressSize);
	if ( Z_OK != nRet)
	{
		delete []UnCompressedData;
		delete []pbCompressedData;
		return FALSE;
	}

	// get first flag . 1 byte
	memcpy((void*)m_firstFlag,UnCompressedData,1);
	// get second flag . 1 byte
	memcpy((void*)m_secondFlag,UnCompressedData+1,1);
	// get m_data.    m_UnCompressSize-2
	m_nDataSize = m_nUnCompressSize -2;
	m_data = new BYTE [m_nDataSize];
	memcpy((void*)m_data,UnCompressedData+2,m_nDataSize);
		
	// don't use ,delete it
	delete [] pbCompressedData;
	delete [] UnCompressedData;
	return TRUE;
}
