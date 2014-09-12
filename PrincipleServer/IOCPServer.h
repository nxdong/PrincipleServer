/********************************************************************
	created:	2014/09/08
	created:	8:9:2014   19:32
	filename: 	H:\PrincipleServer\PrincipleServer\IOCPServer.h
	author:		principle
	
	purpose:	iocp server header
*********************************************************************/

#include "Buffer.h"
#include <Mswsock.h>
// used in thread pool 
#define THREAD_POOL_MAX_THREAD		4
#define THREAD_POOL_MIN_THREAD		1
#define MAX_BUFFER_LEN				8192  
// the number of post accept times when start iocp server
#define MAX_POST_ACCEPT             6
//////////////////////////////////////////////////////////////////////////
// define Client Context struct
//////////////////////////////////////////////////////////////////////////




typedef enum _OPERATION_TYPE  
{  
	ACCEPT_POSTED,					   // post accept
	SEND_POSTED,                       // post send
	RECV_POSTED,                       // post recv
	NULL_POSTED                        // used in initialize

}OPERATION_TYPE;
typedef struct _PER_IO_CONTEXT
{
	OVERLAPPED     m_Overlapped;                               // 每一个重叠网络操作的重叠结构(针对每一个Socket的每一个操作，都要有一个)              
	SOCKET         m_sockAccept;                               // 这个网络操作所使用的Socket
	WSABUF         m_wsaBuf;                                   // WSA类型的缓冲区，用于给重叠操作传参数的
	char           m_szBuffer[MAX_BUFFER_LEN];                 // 这个是WSABUF里具体存字符的缓冲区
	OPERATION_TYPE m_OpType;                                   // 标识网络操作的类型(对应上面的枚举)

	// 初始化
	_PER_IO_CONTEXT()
	{
		ZeroMemory(&m_Overlapped, sizeof(m_Overlapped));  
		ZeroMemory( m_szBuffer,MAX_BUFFER_LEN );
		m_sockAccept = INVALID_SOCKET;
		m_wsaBuf.buf = m_szBuffer;
		m_wsaBuf.len = MAX_BUFFER_LEN;
		m_OpType     = NULL_POSTED;
	}
	// 释放掉Socket
	~_PER_IO_CONTEXT()
	{
		if( m_sockAccept!=INVALID_SOCKET )
		{
			closesocket(m_sockAccept);
			m_sockAccept = INVALID_SOCKET;
		}
	}
	// 重置缓冲区内容
	void ResetBuffer()
	{
		ZeroMemory( m_szBuffer,MAX_BUFFER_LEN );
	}

} PER_IO_CONTEXT, *PPER_IO_CONTEXT;

struct ClientContext
{
	// store the client socket
	SOCKET m_socket;		
	// the handle of thread pool IO .
	PTP_IO m_threadPoolIo;
	// the buffer for send
	CBuffer m_sendBuffer;
	// the buffer for receive
	CBuffer m_recvBuffer;

	SOCKADDR_IN m_ClientAddr;                              // 客户端的地址
	CArray<_PER_IO_CONTEXT*> m_arrayIoContext;             // 客户端网络操作的上下文数据，
														   // 也就是说对于每一个客户端Socket，是可以在上面同时投递多个IO请求的
	// 初始化
	ClientContext()
	{
		m_socket = INVALID_SOCKET;
		memset(&m_ClientAddr, 0, sizeof(m_ClientAddr)); 
	}

	// 释放资源
	~ClientContext()
	{
		if( m_socket!=INVALID_SOCKET )
		{
			closesocket( m_socket );
			m_socket = INVALID_SOCKET;
		}
		// 释放掉所有的IO上下文数据
		for( int i=0;i<m_arrayIoContext.GetCount();i++ )
		{
			delete m_arrayIoContext.GetAt(i);
		}
		m_arrayIoContext.RemoveAll();
	}

	// 获取一个新的IoContext
	_PER_IO_CONTEXT* GetNewIoContext()
	{
		_PER_IO_CONTEXT* p = new _PER_IO_CONTEXT;

		m_arrayIoContext.Add( p );

		return p;
	}

	// 从数组中移除一个指定的IoContext
	void RemoveContext( _PER_IO_CONTEXT* pContext )
	{
		ASSERT( pContext!=NULL );

		for( int i=0;i<m_arrayIoContext.GetCount();i++ )
		{
			if( pContext==m_arrayIoContext.GetAt(i) )
			{
				delete pContext;
				pContext = NULL;
				m_arrayIoContext.RemoveAt(i);				
				break;
			}
		}
	}



};

typedef void (CALLBACK* NOTIFYPROC)(ClientContext*);
class CIOCPServer
{
private:
public:
	// listen socket context .   
	ClientContext*	m_pListenContext;
	// User's notify process
	NOTIFYPROC	m_pNotifyProc;
	// max connections number
	UINT		m_nMaxConnections;
	// PTP_IO of listen thread
	PTP_IO		m_listenIO;
	// this function will be called when IO completed .
	PTP_WIN32_IO_CALLBACK m_pThreadPoolCallBack;
	// flag that indicates running. run TRUE ,stop FALSE.
	BOOL		m_isRunning;
	// server listen port 
	UINT		m_nPort;
	// AcceptEx  function pointer
	LPFN_ACCEPTEX                m_AcceptEx;       
	// GetAcceptExSockaddrs function pointer
	LPFN_GETACCEPTEXSOCKADDRS    m_GetAcceptExSockAddrs;
	// used in add to client context list.
	CRITICAL_SECTION             m_csContextList;  
	// store alive client context.
	CArray<ClientContext*>  m_arrayClientContext; 
private:
	// Called before start.
	BOOL PrepareLibary();
	// Get function pointer.AcceptEx and GetAcceptExSockaddrs
	BOOL GetFunctionPointer();
	

public:
	// define functions
	CIOCPServer();
	~CIOCPServer();

	// User call this function to initialize server
	BOOL Initialize(NOTIFYPROC pNotifyProc, int nMaxConnections, int nPort);
	static void CALLBACK CompletionCallback(PTP_CALLBACK_INSTANCE pInstance,
		PVOID pvContext,
		PVOID pOverlapped,
		ULONG IoResult,
		ULONG_PTR NumberOfBytesTransferred,
		PTP_IO  pIo);
	BOOL PostAccept(PER_IO_CONTEXT *pAcceptIoContext);
	BOOL DoAccept(ClientContext* pSocketContext,PER_IO_CONTEXT* pIoContext);
	void AddToContextList(ClientContext *pSocketContext );
	void RemoveContext(ClientContext *pSocketContext);
	// clear all client context
	void ClearContextList();
	// post recv 
	BOOL PostRecv(PER_IO_CONTEXT* pIoContext);
	// process recv
	BOOL DoRecv( ClientContext* pSocketContext, PER_IO_CONTEXT* pIoContext );

	BOOL IsSocketAlive(SOCKET s);

};



