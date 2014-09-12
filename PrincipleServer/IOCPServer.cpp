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
	m_csContextList			= NULL;
}
CIOCPServer::~CIOCPServer()
{
	try
	{
		DeleteCriticalSection(&m_csContextList);
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
	// ����(һ�㶼�����ܳ���)
	if (NO_ERROR != nResult)
	{
		return FALSE; 
	}
	return TRUE;
}

BOOL CIOCPServer::Initialize(NOTIFYPROC pNotifyProc, int nMaxConnections, int nPort)
{
	PrepareLibary();
	InitializeCriticalSection(&m_csContextList);
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
	StartThreadpoolIo(m_listenIO);
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
	// AcceptEx and GetAcceptExSockaddrs GUID��used to export function pointer
	GUID GuidAcceptEx = WSAID_ACCEPTEX;  
	GUID GuidGetAcceptExSockAddrs = WSAID_GETACCEPTEXSOCKADDRS; 
	// ��ȡAcceptEx����ָ��
	DWORD dwBytes = 0;  
	if(SOCKET_ERROR == WSAIoctl(
		m_pListenContext->m_socket, 
		SIO_GET_EXTENSION_FUNCTION_POINTER, 
		&GuidAcceptEx, 
		sizeof(GuidAcceptEx), 
		&m_AcceptEx, 
		sizeof(m_AcceptEx), 
		&dwBytes, 
		NULL, 
		NULL))  
	{  
		closesocket(m_pListenContext->m_socket);
		WSACleanup();
		return FALSE;  
	}  

	// get GetAcceptExSockAddrs  pointer
	if(SOCKET_ERROR == WSAIoctl(
		m_pListenContext->m_socket, 
		SIO_GET_EXTENSION_FUNCTION_POINTER, 
		&GuidGetAcceptExSockAddrs,
		sizeof(GuidGetAcceptExSockAddrs), 
		&m_GetAcceptExSockAddrs, 
		sizeof(m_GetAcceptExSockAddrs),   
		&dwBytes, 
		NULL, 
		NULL))  
	{  
		closesocket(m_pListenContext->m_socket);
		WSACleanup();
		return FALSE; 
	}  

}

BOOL CIOCPServer::PostAccept(PER_IO_CONTEXT* pAcceptIoContext)
{
	ASSERT( INVALID_SOCKET!= m_pListenContext->m_socket);
	// ׼������
	DWORD dwBytes = 0;  
	pAcceptIoContext->m_OpType = ACCEPT_POSTED;  
	WSABUF *p_wbuf   = &pAcceptIoContext->m_wsaBuf;
	OVERLAPPED *p_ol = &pAcceptIoContext->m_Overlapped;

	// Ϊ�Ժ�������Ŀͻ�����׼����Socket( ������봫ͳaccept�������� ) 
	pAcceptIoContext->m_sockAccept  = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);  
	if( INVALID_SOCKET==pAcceptIoContext->m_sockAccept )  
	{  
		//"��������Accept��Socketʧ�ܣ��������: %d", WSAGetLastError() 
		return FALSE;  
	} 

	// Ͷ��AcceptEx
	if(FALSE == m_AcceptEx( m_pListenContext->m_socket, pAcceptIoContext->m_sockAccept, p_wbuf->buf, p_wbuf->len - ((sizeof(SOCKADDR_IN)+16)*2),   
		sizeof(SOCKADDR_IN)+16, sizeof(SOCKADDR_IN)+16, &dwBytes, p_ol))  
	{  
		if(WSA_IO_PENDING != WSAGetLastError())  
		{  
			//"Ͷ�� AcceptEx ����ʧ�ܣ��������: %d", WSAGetLastError() 
			return FALSE;  
		}  
	} 
	return TRUE;
}
BOOL CIOCPServer::DoAccept(ClientContext* pSocketContext,PER_IO_CONTEXT* pIoContext)
{
	SOCKADDR_IN* ClientAddr = NULL;
	SOCKADDR_IN* LocalAddr = NULL;  
	int remoteLen = sizeof(SOCKADDR_IN), localLen = sizeof(SOCKADDR_IN);  

	// ��������ȡ�ÿͻ��˺ͱ��ض˵ĵ�ַ��Ϣ������˳��ȡ���ͻ��˷����ĵ�һ������
	m_GetAcceptExSockAddrs(pIoContext->m_wsaBuf.buf,
		pIoContext->m_wsaBuf.len - ((sizeof(SOCKADDR_IN)+16)*2),  
		sizeof(SOCKADDR_IN)+16, sizeof(SOCKADDR_IN)+16,
		(LPSOCKADDR*)&LocalAddr, &localLen,
		(LPSOCKADDR*)&ClientAddr, &remoteLen);
	// 2. ������Ҫע�⣬���ﴫ��������ListenSocket�ϵ�Context�����Context���ǻ���Ҫ���ڼ�����һ������
	// �����һ���Ҫ��ListenSocket�ϵ�Context���Ƴ���һ��Ϊ�������Socket�½�һ��SocketContext

	ClientContext* pNewSocketContext = new ClientContext;
	pNewSocketContext->m_socket           = pIoContext->m_sockAccept;
	memcpy(&(pNewSocketContext->m_ClientAddr), ClientAddr, sizeof(SOCKADDR_IN));
	pNewSocketContext->m_recvBuffer.m_recvPacket = (BYTE*)pIoContext->m_wsaBuf.buf;
	pNewSocketContext->m_recvBuffer.GetInformation();

	// ����������ϣ������Socket����ɶ˿ڰ�(��Ҳ��һ���ؼ�����)
	pNewSocketContext->m_threadPoolIo = CreateThreadpoolIo((HANDLE)pNewSocketContext->m_socket,
		(PTP_WIN32_IO_CALLBACK)m_pThreadPoolCallBack,
		NULL,NULL);
	if (NULL == m_listenIO)
	{
		return FALSE;
	}
	StartThreadpoolIo(pNewSocketContext->m_threadPoolIo);
	
	// 3. �������µ�IoContext�����������Socket��Ͷ�ݵ�һ��Recv��������
	PER_IO_CONTEXT* pNewIoContext = pNewSocketContext->GetNewIoContext();
	pNewIoContext->m_OpType       = RECV_POSTED;
	pNewIoContext->m_sockAccept   = pNewSocketContext->m_socket;
	
	// ����ϣ���ʼ�����Socket��Ͷ���������
	if( false==this->PostRecv( pNewIoContext) )
	{
		pNewSocketContext->RemoveContext( pNewIoContext );
		return false;
	}
	// 4. ���Ͷ�ݳɹ�����ô�Ͱ������Ч�Ŀͻ�����Ϣ�����뵽ContextList��ȥ,���������ڴ���
	m_pNotifyProc(pNewSocketContext);
	AddToContextList( pNewSocketContext );
	// 5. ʹ�����֮�󣬰�Listen Socket���Ǹ�IoContext���ã�Ȼ��׼��Ͷ���µ�AcceptEx
	pIoContext->ResetBuffer();
	return PostAccept( pIoContext ); 	
}
BOOL CIOCPServer::PostRecv(PER_IO_CONTEXT* pIoContext)
{
	// ��ʼ������
	DWORD dwFlags = 0;
	DWORD dwBytes = 0;
	WSABUF *p_wbuf   = &pIoContext->m_wsaBuf;
	OVERLAPPED *p_ol = &pIoContext->m_Overlapped;

	pIoContext->ResetBuffer();
	pIoContext->m_OpType = RECV_POSTED;

	// ��ʼ����ɺ󣬣�Ͷ��WSARecv����
	int nBytesRecv = WSARecv( pIoContext->m_sockAccept, p_wbuf, 1, &dwBytes, &dwFlags, p_ol, NULL );
	// �������ֵ���󣬲��Ҵ���Ĵ��벢����Pending�Ļ����Ǿ�˵������ص�����ʧ����
	if ((SOCKET_ERROR == nBytesRecv) && (WSA_IO_PENDING != WSAGetLastError()))
	{
		//"Ͷ�ݵ�һ��WSARecvʧ�ܣ�
		return FALSE;
	}
	return TRUE;
}
BOOL CIOCPServer::DoRecv( ClientContext* pSocketContext, PER_IO_CONTEXT* pIoContext )
{
	// �Ȱ���һ�ε�������ʾ���֣�Ȼ�������״̬��������һ��Recv����
	SOCKADDR_IN* ClientAddr = &pSocketContext->m_ClientAddr;
	//this->_ShowMessage( _T("�յ�  %s:%d ��Ϣ��%s"),inet_ntoa(ClientAddr->sin_addr), ntohs(ClientAddr->sin_port),pIoContext->m_wsaBuf.buf );
	pSocketContext->m_recvBuffer.m_recvPacket = (PBYTE)pIoContext->m_wsaBuf.buf;
	pSocketContext->m_recvBuffer.GetInformation();
	m_pNotifyProc(pSocketContext);
	// Ȼ��ʼͶ����һ��WSARecv����
	return PostRecv( pIoContext );
}
void CIOCPServer::AddToContextList(ClientContext *pSocketContext )
{
	EnterCriticalSection(&m_csContextList);
	m_arrayClientContext.Add(pSocketContext);	
	LeaveCriticalSection(&m_csContextList);

}
void CIOCPServer::RemoveContext(ClientContext *pSocketContext)
{
	EnterCriticalSection(&m_csContextList);
	CloseThreadpoolIo(pSocketContext->m_threadPoolIo);
	for( int i=0;i<m_arrayClientContext.GetCount();i++ )
	{
		if( pSocketContext==m_arrayClientContext.GetAt(i) )
		{
			
			delete pSocketContext;
			pSocketContext = NULL;
			m_arrayClientContext.RemoveAt(i);			
			break;
		}
	}
	LeaveCriticalSection(&m_csContextList);
}
void CIOCPServer::ClearContextList()
{
	EnterCriticalSection(&m_csContextList);
	for( int i=0;i<m_arrayClientContext.GetCount();i++ )
	{
		CloseThreadpoolIo(m_arrayClientContext.GetAt(i)->m_threadPoolIo);
		delete m_arrayClientContext.GetAt(i);
	}
	m_arrayClientContext.RemoveAll();
	LeaveCriticalSection(&m_csContextList);
}
BOOL CIOCPServer::IsSocketAlive(SOCKET s)
{
	int nByteSent=send(s,"",0,0);
	if (-1 == nByteSent) return FALSE;
	return TRUE;
}
