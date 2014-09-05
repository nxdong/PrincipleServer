/********************************************************************
	created:	2014/09/01
	created:	1:9:2014   22:44
	filename: 	Buffer.h
	author:		Principle
	
	purpose:	This file define CBuffer Class used in iocp model.
*********************************************************************/
#pragma once
#include "windows.h"

class CBuffer
{
public:
	// members
	BYTE m_header[4];			// store the header of the packet 'p' 'r' 'i' 'c'
	BYTE m_firstFlag;			// store the first flag.
	BYTE m_secondFlag;			// store the second flag.

	PBYTE m_data;				// pointer to the bytes data.
	PBYTE m_sendPacket;			// the bytes stream will be send.
	PBYTE m_recvPacket;			// the bytes stream received,will be process.

	UINT m_size;				// size of all the buffer.
	UINT m_nDataSize;			// size of data will be compressed.(include flags)
	UINT m_nDataCompressedSize;	// size of compressed data.m_size = this + 4 + 4 + 4.

private:
	BOOL AllocateBuffer(UINT nNeedSize);				  // AllocateMemory 
public:
	// functions
	void AddFirstFlag(BYTE &bFirstFlag);
	void AddSecondFlag(BYTE &bSecondFlag);
	void AddFlags(BYTE &bFirstFlag,BYTE &bSecondFlag);
	void AddData(const BYTE *const pData,UINT nSize);     // this size is the byte size
	BOOL PreparePackate();                                // call this function before send message.
	BOOL GetMessage();									  // call this function before use information.
	BOOL Fresh();										  // reset all information.									
};

