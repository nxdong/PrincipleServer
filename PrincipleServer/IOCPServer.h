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
#define THREAD_POOL_MAX_THREAD 4
#define THREAD_POOL_MIN_THREAD 1
#define MAX_BUFFER_LEN        8192  
//////////////////////////////////////////////////////////////////////////
// define Client Context struct
//////////////////////////////////////////////////////////////////////////


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


};

typedef enum _OPERATION_TYPE  
{  
	ACCEPT_POSTED,					   // post accept
	SEND_POSTED,                       // post send
	RECV_POSTED,                       // post recv
	NULL_POSTED                        // used in initialize

}OPERATION_TYPE;
typedef struct _PER_IO_CONTEXT
{
	OVERLAPPED     m_Overlapped;                               // ÿһ���ص�����������ص��ṹ(���ÿһ��Socket��ÿһ����������Ҫ��һ��)              
	SOCKET         m_sockAccept;                               // ������������ʹ�õ�Socket
	WSABUF         m_wsaBuf;                                   // WSA���͵Ļ����������ڸ��ص�������������
	char           m_szBuffer[MAX_BUFFER_LEN];                 // �����WSABUF�������ַ��Ļ�����
	OPERATION_TYPE m_OpType;                                   // ��ʶ�������������(��Ӧ�����ö��)

	// ��ʼ��
	_PER_IO_CONTEXT()
	{
		ZeroMemory(&m_Overlapped, sizeof(m_Overlapped));  
		ZeroMemory( m_szBuffer,MAX_BUFFER_LEN );
		m_sockAccept = INVALID_SOCKET;
		m_wsaBuf.buf = m_szBuffer;
		m_wsaBuf.len = MAX_BUFFER_LEN;
		m_OpType     = NULL_POSTED;
	}
	// �ͷŵ�Socket
	~_PER_IO_CONTEXT()
	{
		if( m_sockAccept!=INVALID_SOCKET )
		{
			closesocket(m_sockAccept);
			m_sockAccept = INVALID_SOCKET;
		}
	}
	// ���û���������
	void ResetBuffer()
	{
		ZeroMemory( m_szBuffer,MAX_BUFFER_LEN );
	}

} PER_IO_CONTEXT, *PPER_IO_CONTEXT;


typedef void (CALLBACK* NOTIFYPROC)(ClientContext*, UINT nCode);
class CIOCPServer
{
private:
public:
	// listen socket .
	SOCKET		m_listenSocket;
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
	BOOL PostAccept(PER_IO_CONTEXT* pAcceptIoContext);


	



};



