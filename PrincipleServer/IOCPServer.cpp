/********************************************************************
	created:	2014/09/08
	created:	8:9:2014   19:32
	filename: 	IOCPServer.cpp
	author:		principle
	
	purpose:	iocp server cpp
*********************************************************************/
#include "stdafx.h"
#include "IOCPServer.h"

CIOCPServer::CIOCPServer()
{
	m_listenSocket		= NULL;
	m_pNotifyProc		= NULL;
	m_pThreadPoolCallBack = NULL;
	m_isRunning			= FALSE;
}
CIOCPServer::~CIOCPServer()
{
	try
	{
		WSACleanup();
	}catch(...){}
}
void CIOCPServer::CompletionCallback(PTP_CALLBACK_INSTANCE pInstance, 
	PVOID pvContext, PVOID pOverlapped,
	ULONG IoResult, ULONG_PTR NumberOfBytesTransferred, PTP_IO pIo)
{

	return;
}
BOOL CIOCPServer::PrepareLibary()
{
	WSADATA wsaData;
	int nResult;
	nResult = WSAStartup(MAKEWORD(2,2), &wsaData);
	// 错误(一般都不可能出现)
	if (NO_ERROR != nResult)
	{
		return FALSE; 
	}
	return TRUE;
}

BOOL CIOCPServer::Initialize(NOTIFYPROC pNotifyProc, int nMaxConnections, int nPort)
{
	PrepareLibary();
	m_pNotifyProc = pNotifyProc;
	m_nMaxConnections = nMaxConnections;
	m_pThreadPoolCallBack = CompletionCallback;
	// create listen socket
	m_listenSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (INVALID_SOCKET == m_listenSocket)
	{
		return FALSE;
	}
	// add listen socket to thread pool
	m_listenIO = CreateThreadpoolIo((HANDLE)m_listenSocket,
		(PTP_WIN32_IO_CALLBACK)m_pThreadPoolCallBack,
		NULL,NULL);
	if (NULL == m_listenIO)
	{
		return FALSE;
	}


	return TRUE;
}
