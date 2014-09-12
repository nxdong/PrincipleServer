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
	// AcceptEx and GetAcceptExSockaddrs GUID，used to export function pointer
	GUID GuidAcceptEx = WSAID_ACCEPTEX;  
	GUID GuidGetAcceptExSockAddrs = WSAID_GETACCEPTEXSOCKADDRS; 
	// 获取AcceptEx函数指针
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
		return FALSE;  
	} 

	// 投递AcceptEx
	if(FALSE == m_AcceptEx( m_pListenContext->m_socket, pAcceptIoContext->m_sockAccept, p_wbuf->buf, p_wbuf->len - ((sizeof(SOCKADDR_IN)+16)*2),   
		sizeof(SOCKADDR_IN)+16, sizeof(SOCKADDR_IN)+16, &dwBytes, p_ol))  
	{  
		if(WSA_IO_PENDING != WSAGetLastError())  
		{  
			//"投递 AcceptEx 请求失败，错误代码: %d", WSAGetLastError() 
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

	// 不但可以取得客户端和本地端的地址信息，还能顺便取出客户端发来的第一组数据
	m_GetAcceptExSockAddrs(pIoContext->m_wsaBuf.buf,
		pIoContext->m_wsaBuf.len - ((sizeof(SOCKADDR_IN)+16)*2),  
		sizeof(SOCKADDR_IN)+16, sizeof(SOCKADDR_IN)+16,
		(LPSOCKADDR*)&LocalAddr, &localLen,
		(LPSOCKADDR*)&ClientAddr, &remoteLen);
	// 2. 这里需要注意，这里传入的这个是ListenSocket上的Context，这个Context我们还需要用于监听下一个连接
	// 所以我还得要将ListenSocket上的Context复制出来一份为新连入的Socket新建一个SocketContext

	ClientContext* pNewSocketContext = new ClientContext;
	pNewSocketContext->m_socket           = pIoContext->m_sockAccept;
	memcpy(&(pNewSocketContext->m_ClientAddr), ClientAddr, sizeof(SOCKADDR_IN));
	pNewSocketContext->m_recvBuffer.m_recvPacket = (BYTE*)pIoContext->m_wsaBuf.buf;
	pNewSocketContext->m_recvBuffer.GetInformation();

	// 参数设置完毕，将这个Socket和完成端口绑定(这也是一个关键步骤)
	pNewSocketContext->m_threadPoolIo = CreateThreadpoolIo((HANDLE)pNewSocketContext->m_socket,
		(PTP_WIN32_IO_CALLBACK)m_pThreadPoolCallBack,
		NULL,NULL);
	if (NULL == m_listenIO)
	{
		return FALSE;
	}
	StartThreadpoolIo(pNewSocketContext->m_threadPoolIo);
	
	// 3. 建立其下的IoContext，用于在这个Socket上投递第一个Recv数据请求
	PER_IO_CONTEXT* pNewIoContext = pNewSocketContext->GetNewIoContext();
	pNewIoContext->m_OpType       = RECV_POSTED;
	pNewIoContext->m_sockAccept   = pNewSocketContext->m_socket;
	
	// 绑定完毕，开始在这个Socket上投递完成请求
	if( false==this->PostRecv( pNewIoContext) )
	{
		pNewSocketContext->RemoveContext( pNewIoContext );
		return false;
	}
	// 4. 如果投递成功，那么就把这个有效的客户端信息，加入到ContextList中去,并交给窗口处理
	m_pNotifyProc(pNewSocketContext);
	AddToContextList( pNewSocketContext );
	// 5. 使用完毕之后，把Listen Socket的那个IoContext重置，然后准备投递新的AcceptEx
	pIoContext->ResetBuffer();
	return PostAccept( pIoContext ); 	
}
BOOL CIOCPServer::PostRecv(PER_IO_CONTEXT* pIoContext)
{
	// 初始化变量
	DWORD dwFlags = 0;
	DWORD dwBytes = 0;
	WSABUF *p_wbuf   = &pIoContext->m_wsaBuf;
	OVERLAPPED *p_ol = &pIoContext->m_Overlapped;

	pIoContext->ResetBuffer();
	pIoContext->m_OpType = RECV_POSTED;

	// 初始化完成后，，投递WSARecv请求
	int nBytesRecv = WSARecv( pIoContext->m_sockAccept, p_wbuf, 1, &dwBytes, &dwFlags, p_ol, NULL );
	// 如果返回值错误，并且错误的代码并非是Pending的话，那就说明这个重叠请求失败了
	if ((SOCKET_ERROR == nBytesRecv) && (WSA_IO_PENDING != WSAGetLastError()))
	{
		//"投递第一个WSARecv失败！
		return FALSE;
	}
	return TRUE;
}
BOOL CIOCPServer::DoRecv( ClientContext* pSocketContext, PER_IO_CONTEXT* pIoContext )
{
	// 先把上一次的数据显示出现，然后就重置状态，发出下一个Recv请求
	SOCKADDR_IN* ClientAddr = &pSocketContext->m_ClientAddr;
	//this->_ShowMessage( _T("收到  %s:%d 信息：%s"),inet_ntoa(ClientAddr->sin_addr), ntohs(ClientAddr->sin_port),pIoContext->m_wsaBuf.buf );
	pSocketContext->m_recvBuffer.m_recvPacket = (PBYTE)pIoContext->m_wsaBuf.buf;
	pSocketContext->m_recvBuffer.GetInformation();
	m_pNotifyProc(pSocketContext);
	// 然后开始投递下一个WSARecv请求
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
