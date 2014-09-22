/********************************************************************
	created:	2014/09/08
	created:	8:9:2014   19:32
	filename: 	IOCPServer.cpp
	author:		principle
	
	purpose:	iocp server cpp
*********************************************************************/
#include "stdafx.h"
#include "IOCPServer.h"

CRITICAL_SECTION CIOCPServer::m_cs;
CIOCPServer::CIOCPServer(void)
{
	m_nPort				= DEFAULT_PORT;
	m_strIP				= DEFAULT_IP;
	m_pNotifyProc		= NULL;
	m_hCompletionPort	= NULL;
	m_hShutdownEvent	= NULL;
	m_bIsStart			= FALSE;
	m_hNetworkIO		= NULL;
	m_listenThread		= NULL;
	m_bDisconnectAll	= FALSE;
	m_bTimeToKill		= FALSE;

	BYTE bPacketFlag[] = {'p', 'r', 'i', 'n'};
	memcpy(m_bPacketFlag, bPacketFlag, sizeof(bPacketFlag));
}
CIOCPServer::~CIOCPServer()
{
	try
	{
		Shutdown();
		WSACleanup();
	}catch(...){}
}

BOOL CIOCPServer::Initialize(NOTIFYPROC pNotifyProc,IN int nMaxConnections,IN int nPort)
{
	m_pNotifyProc		= pNotifyProc;
	m_nMaxConnections	= nMaxConnections;
	m_nPort				= nPort;
	m_socListen = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (m_socListen == INVALID_SOCKET)
	{
		//"Could not create listen socket
		return FALSE;
	}
	// Event for handling Network IO
	m_hNetworkIO = WSACreateEvent();

	if (m_hNetworkIO == WSA_INVALID_EVENT)
	{
		//"WSACreateEvent() error %ld\n")
		closesocket(m_socListen);
		return false;
	}

	// The listener is ONLY interested in FD_ACCEPT
	int nRet = WSAEventSelect(m_socListen,
		m_hNetworkIO,
		FD_ACCEPT);

	if (nRet == SOCKET_ERROR)
	{
		//TRACE(_T("WSAAsyncSelect() error %ld\n"),WSAGetLastError());
		closesocket(m_socListen);
		return false;
	}

	SOCKADDR_IN		saServer;		
	char *charIP;
	WideCharToMultiByte(CP_OEMCP,0,(LPCTSTR)m_strIP,-1,charIP,260,0,false);
	// Listen on our designated Port#
	saServer.sin_port = htons(nPort);
	// Fill in the rest of the address structure
	saServer.sin_family = AF_INET;
	saServer.sin_addr.s_addr = inet_addr(charIP);

	// bind our name to the socket
	nRet = bind(m_socListen, 
		(LPSOCKADDR)&saServer, 
		sizeof(struct sockaddr));
	if (nRet == SOCKET_ERROR)
	{
		closesocket(m_socListen);
		return FALSE;
	}
	// Set the socket to listen
	nRet = listen(m_socListen, SOMAXCONN);
	if (nRet == SOCKET_ERROR)
	{
		closesocket(m_socListen);
		return FALSE;
	}

	UINT	dwThreadId = 0;
	m_listenThread =
		(HANDLE)_beginthreadex(NULL,				// Security
		0,					// Stack size - use default
		ListenThreadProc,  // Thread function entry point
		(void*) this,	    
		0,					// Init flag
		&dwThreadId);	// Thread address
	if ( INVALID_HANDLE_VALUE != m_listenThread)
	{
		InitializeIOCP();
		m_bIsStart = TRUE;
		return TRUE;
	}

	return FALSE;
}

unsigned CIOCPServer::ListenThreadProc(LPVOID lParam)
{
	CIOCPServer* pThis = reinterpret_cast<CIOCPServer*>(lParam);
	WSANETWORKEVENTS events;
	while(1)
	{
		// Wait for something to happen
		if (WaitForSingleObject(pThis->m_hShutdownEvent, 100) == WAIT_OBJECT_0)
			break;

		DWORD dwRet;
		dwRet = WSAWaitForMultipleEvents(1,
			&pThis->m_hNetworkIO,
			FALSE,
			100,
			FALSE);
		if (dwRet == WSA_WAIT_TIMEOUT)
			continue;
		// Figure out what happened
		int nRet = WSAEnumNetworkEvents(pThis->m_socListen,
			pThis->m_hNetworkIO,
			&events);

		if (nRet == SOCKET_ERROR)
		{
			//WSAEnumNetworkEvents error
			break;
		}
		// Handle Network events //
		// ACCEPT
		if (events.lNetworkEvents & FD_ACCEPT)
		{
			if (events.iErrorCode[FD_ACCEPT_BIT] == 0)
				pThis->OnAccept();
			else
			{
				//Unknown network event error
				break;
			}
		}
	} // while....
	return 0; // Normal Thread Exit Code...
}

void CIOCPServer::OnAccept()
{
	SOCKADDR_IN	SockAddr;
	SOCKET		clientSocket;
	int			nRet;
	int			nLen;
	if (m_bTimeToKill || m_bDisconnectAll)
		return;
	// accept the new socket descriptor
	nLen = sizeof(SOCKADDR_IN);
	clientSocket = accept(m_socListen,
		(LPSOCKADDR)&SockAddr,
		&nLen); 
	if (clientSocket == SOCKET_ERROR)
	{
		nRet = WSAGetLastError();
		if (nRet != WSAEWOULDBLOCK)
		{
			//TRACE(_T("accept() error\n"),WSAGetLastError());
			return;
		}
	}
	// Create the Client context to be associated with the completion port
	ClientContext* pContext = AllocateContext();
	// AllocateContext fail
	if (pContext == NULL)
		return;

	pContext->m_Socket = clientSocket;

	// Fix up In Buffer
	pContext->m_wsaInBuffer.buf = (char*)pContext->m_byInBuffer;
	pContext->m_wsaInBuffer.len = sizeof(pContext->m_byInBuffer);

	// Associate the new socket with a completion port.
	if (!AssociateSocketWithCompletionPort(clientSocket, m_hCompletionPort, (DWORD) pContext))
	{
		delete pContext;
		pContext = NULL;
		closesocket( clientSocket );
		closesocket( m_socListen );
		return;
	}
	CLock cs(m_cs,_T("OnAccept") );
	// Hold a reference to the context
	m_listContexts.AddTail(pContext);

	OVERLAPPEDPLUS	*pOverlap = new OVERLAPPEDPLUS(IOInitialize);
	BOOL bSuccess = PostQueuedCompletionStatus(m_hCompletionPort, 0, (DWORD) pContext, &pOverlap->m_ol);
	if ( (!bSuccess && GetLastError() != ERROR_IO_PENDING))
	{            
		RemoveStaleClient(pContext,TRUE);
		return;
	}
	m_pNotifyProc(pContext);
	// Post to WSARecv Next
	PostRecv(pContext);
}

BOOL CIOCPServer::InitializeIOCP(void)
{
	DWORD i;
	UINT  nThreadID;
	// Create the completion port that will be used by all the worker
	// threads.
	m_hCompletionPort = CreateIoCompletionPort( NULL, NULL, 0, 0 );
	if ( m_hCompletionPort == NULL ) 
	{
		return FALSE;
	}

	int nWorkerCnt;
	nWorkerCnt = GetNoOfProcessors() * WORKER_THREADS_PER_PROCESSOR;
	// We need to save the Handles for Later Termination...
	m_phWorkerThreads = new HANDLE[nWorkerCnt];
	m_nWorkerCnt = 0;
	for ( i = 0; i < nWorkerCnt; i++ ) 
	{
		m_phWorkerThreads[i] = (HANDLE)_beginthreadex(NULL,					// Security
			0,						// Stack size - use default
			ThreadPoolFunc,     		// Thread function entry point
			(void*) this,			// Param for thread
			0,						// Init flag
			&nThreadID);			// Thread address

		if (m_phWorkerThreads[i] == NULL ) 
		{
			CloseHandle( m_hCompletionPort );
			return FALSE;
		}
		m_nWorkerCnt++;
	}

	return TRUE;
}

unsigned CIOCPServer::ThreadPoolFunc (LPVOID thisContext)    
{
	// Get back our pointer to the class
	ULONG ulFlags = MSG_PARTIAL;
	CIOCPServer* pThis = reinterpret_cast<CIOCPServer*>(thisContext);
	ASSERT(pThis);
	HANDLE hCompletionPort = pThis->m_hCompletionPort;
	DWORD dwIoSize;
	LPOVERLAPPED lpOverlapped;
	ClientContext* lpClientContext;
	OVERLAPPEDPLUS*	pOverlapPlus;
	bool			bError;
	// Loop round and round servicing I/O completions.
	for (;FALSE == pThis->m_bTimeToKill; ) 
	{
		pOverlapPlus	= NULL;
		lpClientContext = NULL;
		bError			= false;
		// Get a completed IO request.
		BOOL bIORet = GetQueuedCompletionStatus(
			hCompletionPort,
			&dwIoSize,
			(LPDWORD) &lpClientContext,
			&lpOverlapped, INFINITE);

		DWORD dwIOError = GetLastError();
		pOverlapPlus = CONTAINING_RECORD(lpOverlapped, OVERLAPPEDPLUS, m_ol);
		if (!bIORet && dwIOError != WAIT_TIMEOUT )
		{
			if (lpClientContext && pThis->m_bTimeToKill == false)
			{
				pThis->RemoveStaleClient(lpClientContext, FALSE);
			}
			continue;
			bError = true;
		}
		if (!bError)
		{
			if(bIORet && NULL != pOverlapPlus && NULL != lpClientContext) 
			{
				try
				{
					pThis->ProcessIOMessage(pOverlapPlus->m_ioType, lpClientContext, dwIoSize);
				}
				catch (...) {}
			}
		}
		if(pOverlapPlus)
			delete pOverlapPlus; // from previous call
	}
	return 0;
} 

void CIOCPServer::ProcessIOMessage(IOType iotype,ClientContext *pClientContext,DWORD dwIoSize)
{
	switch(iotype)
	{
	case IOInitialize:
		OnClientInitializing(pClientContext,dwIoSize);
		break;
	case IORead:
		OnClientReading(pClientContext,dwIoSize);
		break;
	case IOWrite:
		OnClientWriting(pClientContext,dwIoSize);
		break;
	default:
		break;
	}
}
void CIOCPServer::Stop()
{
	::SetEvent(m_hShutdownEvent);
	WaitForSingleObject(m_listenThread, INFINITE);
	CloseHandle(m_listenThread);
	CloseHandle(m_hShutdownEvent);
}

void CIOCPServer::PostRecv(ClientContext* pContext)
{
	// issue a read request 
	OVERLAPPEDPLUS * pOverlap = new OVERLAPPEDPLUS(IORead);
	ULONG			ulFlags = MSG_PARTIAL;
	DWORD			dwNumberOfBytesRecvd;
	UINT nRetVal = WSARecv(pContext->m_Socket, 
		&pContext->m_wsaInBuffer,
		1,
		&dwNumberOfBytesRecvd, 
		&ulFlags,
		&pOverlap->m_ol, 
		NULL);

	if ( nRetVal == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING) 
	{
		RemoveStaleClient(pContext, FALSE);
	}
}
void CIOCPServer::Send(ClientContext* pContext)
{
	if (pContext == NULL)
		return;
	
	// Wait for Data Ready signal to become available
	WaitForSingleObject(pContext->m_hWriteComplete, INFINITE);
	pContext->m_WriteBuffer.PreparePackate();
	pContext->m_ResendBuffer = pContext->m_WriteBuffer;
	//  Prepare Packet
	//	pContext->m_wsaOutBuffer.buf = (CHAR*) new BYTE[nSize];
	//	pContext->m_wsaOutBuffer.len = pContext->m_WriteBuffer.GetBufferLen();

	OVERLAPPEDPLUS * pOverlap = new OVERLAPPEDPLUS(IOWrite);
	PostQueuedCompletionStatus(m_hCompletionPort, 0, (DWORD) pContext, &pOverlap->m_ol);

}

BOOL CIOCPServer::OnClientInitializing(ClientContext* pContext, DWORD dwIoSize)
{
	// We are not actually doing anything here, but we could for instance make
	// a call to Send() to send a greeting message or something

	return TRUE;		// make sure to issue a read after this
}

BOOL CIOCPServer::OnClientReading(ClientContext* pContext, DWORD dwIoSize)
{
	CLock cs(CIOCPServer::m_cs, _T("OnClientReading"));
	try
	{
		static DWORD nBytes = 0;
		nBytes += dwIoSize;

		if (dwIoSize == 0)
		{
			RemoveStaleClient(pContext, FALSE);
			return false;
		}
		if (dwIoSize == FLAG_SIZE && memcmp(pContext->m_byInBuffer, m_bPacketFlag, FLAG_SIZE) == 0)
		{
			// 重新发送
			Send(pContext,pContext->m_ResendBuffer);
			// 必须再投递一个接收请求
			PostRecv(pContext);
			return TRUE;
		}
		pContext->m_RecvBuffer.m_recvPacket = (PBYTE)pContext->m_wsaInBuffer.buf;
		pContext->m_RecvBuffer.GetInformation();
		m_pNotifyProc(pContext);
		// Post to WSARecv Next
		PostRecv(pContext);
	}catch(...)
	{
		Send(pContext);
		PostRecv(pContext);
	}
}



int CIOCPServer::GetNoOfProcessors()
{
	SYSTEM_INFO si;
	GetSystemInfo(&si);
	return si.dwNumberOfProcessors;
}