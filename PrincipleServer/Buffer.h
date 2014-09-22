/********************************************************************
	created:	2014/09/01
	created:	1:9:2014   22:44
	filename: 	Buffer.h
	author:		Principle
	
	purpose:	This file define CBuffer Class used in iocp model.
*********************************************************************/
#pragma once
#include "windows.h"
#include "zlib/zlib.h"
#pragma comment(lib,"zlib/zlib.lib")
class CBuffer
{
public:
	// members
	BYTE m_header[4];			// store the header of the packet 'p' 'r' 'i' 'n'
	BYTE m_firstFlag;			// store the first flag.
	BYTE m_secondFlag;			// store the second flag.

	PBYTE m_data;				// pointer to the bytes data.
	PBYTE m_sendPacket;			// the bytes stream will be send.
	PBYTE m_recvPacket;			// the bytes stream received,will be process.

	UINT m_size;				// size of all the buffer.
	UINT m_nDataSize;			// size of bytes data will be compressed.

	UINT m_nDataAllSize;		// size of all data.  m_size = this + 4 + 4 + 4.
	UINT m_nUnCompressSize;		// size of uncompress data (their 12 bytes not compress)
	UINT m_nCompressSize;		// size of compressed data

private:
	BOOL AllocateBuffer(PBYTE pBuffer,UINT nNeedSize);				  // AllocateMemory 
public:
	// functions
	CBuffer();
	~CBuffer();
	void AddFirstFlag(BYTE &bFirstFlag);
	void AddSecondFlag(BYTE &bSecondFlag);
	void AddFlags(BYTE &bFirstFlag,BYTE &bSecondFlag);
	void AddData(const BYTE *const pData,UINT nSize);     // this size is the byte size
	BOOL PreparePackate();                                // call this function before send message.
	BOOL GetInformation();									  // call this function before use information.
	void Fresh();										  // reset all information.	
};

