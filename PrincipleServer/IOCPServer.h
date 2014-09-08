/********************************************************************
	created:	2014/09/08
	created:	8:9:2014   19:32
	filename: 	H:\PrincipleServer\PrincipleServer\IOCPServer.h
	author:		principle
	
	purpose:	iocp server header
*********************************************************************/

#include "Buffer.h"
// used in thread pool 
#define THREAD_POOL_MAX_THREAD 4
#define THREAD_POOL_MIN_THREAD 1
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
private:
	// Called before start.
	BOOL PrepareLibary();
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




};



