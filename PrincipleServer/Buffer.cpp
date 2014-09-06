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
	m_nDataCompressedSize = NULL;
	m_nDataSize  = NULL;
	m_data		 = NULL;
	memset(m_header,0,sizeof(m_header));
}
CBuffer::~CBuffer()
{
	delete m_data;
	delete m_sendPacket;
	delete m_recvPacket;
}

