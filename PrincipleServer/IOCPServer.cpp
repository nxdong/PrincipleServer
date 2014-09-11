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
	m_pListenContext		= NULL;
	m_pNotifyProc			= NULL;
	m_pThreadPoolCallBack	= NULL;
	m_isRunning				= FALSE;
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
	m_nPort = nPort;
	// create listen socket
	m_pListenContext->m_socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (INVALID_SOCKET == m_pListenContext->m_socket)
	{
		return FALSE;
	}
	// add listen socket to thread pool
	m_listenIO = CreateThreadpoolIo((HANDLE)m_pListenContext->m_socket,
		(PTP_WIN32_IO_CALLBACK)m_pThreadPoolCallBack,
		NULL,NULL);
	if (NULL == m_listenIO)
	{
		return FALSE;
	}

	// service address information ,used in binding socket
	sockaddr_in ServerAddress;
	ZeroMemory((char *)&ServerAddress, sizeof(ServerAddress));
	ServerAddress.sin_family = AF_INET;
	ServerAddress.sin_addr.s_addr = INADDR_ANY;         
	ServerAddress.sin_port = htons(m_nPort);                       
	// bind
	if (SOCKET_ERROR == bind(m_pListenContext->m_socket,(sockaddr*)&ServerAddress,sizeof(ServerAddress)))
	{
		return FALSE;
	}
	// listen
	if (SOCKET_ERROR == listen(m_pListenContext->m_socket,SOMAXCONN))
	{
		return false;
	}
	// get AcceptEx and GetAcceptExSockaddrs functions pointers
	GetFunctionPointer();
	// post several accept requires
	for( int i=0;i<MAX_POST_ACCEPT;i++ )
	{
		// create a new io context
		PER_IO_CONTEXT* pAcceptIoContext = m_pListenContext->GetNewIoContext();
		if( false== PostAccept( pAcceptIoContext ) )
		{
			return false;
		}
	}

	m_isRunning = TRUE;
	return TRUE;
}
BOOL CIOCPServer::GetFunctionPointer()
{
	// AcceptEx and GetAcceptExSockaddrs GUID，used to export function pointer
	GUID GuidAcceptEx = WSAID_ACCEPTEX;  
	GUID GuidGetAcceptExSockAddrs = WSAID_GETACCEPTEXSOCKADDRS; 
	// 获取AcceptEx函数指针
	DWORD dwBytes = 0;  
	if(SOCKET_ERROR == WSAIoctl(
		m_listenSocket, 
		SIO_GET_EXTENSION_FUNCTION_POINTER, 
		&GuidAcceptEx, 
		sizeof(GuidAcceptEx), 
		&m_AcceptEx, 
		sizeof(m_AcceptEx), 
		&dwBytes, 
		NULL, 
		NULL))  
	{  
		closesocket(m_listenSocket);
		WSACleanup();
		return FALSE;  
	}  

	// get GetAcceptExSockAddrs  pointer
	if(SOCKET_ERROR == WSAIoctl(
		m_listenSocket, 
		SIO_GET_EXTENSION_FUNCTION_POINTER, 
		&GuidGetAcceptExSockAddrs,
		sizeof(GuidGetAcceptExSockAddrs), 
		&m_GetAcceptExSockAddrs, 
		sizeof(m_GetAcceptExSockAddrs),   
		&dwBytes, 
		NULL, 
		NULL))  
	{  
		closesocket(m_listenSocket);
		WSACleanup();
		return FALSE; 
	}  

}

BOOL CIOCPServer::PostAccept(PER_IO_CONTEXT* pAcceptIoContext)
{
	ASSERT( INVALID_SOCKET!= m_pListenContext->m_socket);
	// 准备参数
	DWORD dwBytes = 0;  
	pAcceptIoContext->m_OpType = ACCEPT_POSTED;  
	WSABUF *p_wbuf   = &pAcceptIoContext->m_wsaBuf;
	OVERLAPPED *p_ol = &pAcceptIoContext->m_Overlapped;

	// 为以后新连入的客户端先准备好Socket( 这个是与传统accept最大的区别 ) 
	pAcceptIoContext->m_sockAccept  = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);  
	if( INVALID_SOCKET==pAcceptIoContext->m_sockAccept )  
	{  
		//"创建用于Accept的Socket失败！错误代码: %d", WSAGetLastError() 
		return false;  
	} 

	// 投递AcceptEx
	if(FALSE == m_AcceptEx( m_pListenContext->m_socket, pAcceptIoContext->m_sockAccept, p_wbuf->buf, p_wbuf->len - ((sizeof(SOCKADDR_IN)+16)*2),   
		sizeof(SOCKADDR_IN)+16, sizeof(SOCKADDR_IN)+16, &dwBytes, p_ol))  
	{  
		if(WSA_IO_PENDING != WSAGetLastError())  
		{  
			//"投递 AcceptEx 请求失败，错误代码: %d", WSAGetLastError() 
			return false;  
		}  
	} 
	return TRUE;
}
