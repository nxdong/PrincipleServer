/********************************************************************
	created:	2014/09/08
	created:	8:9:2014   19:32
	filename: 	H:\PrincipleServer\PrincipleServer\IOCPServer.h
	author:		principle
	
	purpose:	iocp server header
*********************************************************************/

#include "Buffer.h"
#include <Mswsock.h>

#define MAX_BUFFER_LEN		8192  
#define DEFAULT_IP			_T("127.0.0.1")
#define DEFAULT_PORT		9527	
// ÿһ���������ϲ������ٸ��߳�
#define WORKER_THREADS_PER_PROCESSOR 2
#define FLAG_SIZE			12

enum IOType 
{
	IOInitialize,
	IORead,
	IOWrite,
	IOIdle
};

class OVERLAPPEDPLUS 
{
public:
	OVERLAPPED			m_ol;
	IOType				m_ioType;

	OVERLAPPEDPLUS(IOType ioType) {
		ZeroMemory(this, sizeof(OVERLAPPEDPLUS));
		m_ioType = ioType;
	}
};

struct ClientContext
{
	SOCKET				m_Socket;
	// Store buffers
	CBuffer				m_WriteBuffer;
	CBuffer				m_RecvBuffer;	
	CBuffer				m_ResendBuffer;

	// Input Elements for Winsock
	WSABUF				m_wsaInBuffer;
	char				m_byInBuffer[MAX_BUFFER_LEN];
	// Output elements for Winsock
	//WSABUF				m_wsaOutBuffer;
	HANDLE				m_hWriteComplete;

	ClientContext()
	{
		m_Socket = INVALID_SOCKET;
		ZeroMemory( m_byInBuffer,MAX_BUFFER_LEN);
		m_WriteBuffer.Fresh();
		m_RecvBuffer.Fresh();
		m_ResendBuffer.Fresh();
		m_wsaInBuffer.buf = m_byInBuffer;
		m_wsaInBuffer.len = MAX_BUFFER_LEN;
		m_hWriteComplete  = INVALID_HANDLE_VALUE;
	}
	~ClientContext()
	{
		if(INVALID_SOCKET == m_Socket)
		{
			closesocket(m_Socket);
			m_Socket = INVALID_SOCKET;
		}
	}
};

class CLock
{
public:
	CLock(CRITICAL_SECTION& cs, const CString& strFunc)
	{
		m_strFunc = strFunc;
		m_pcs = &cs;
		Lock();
	}
	~CLock()
	{
		Unlock();

	}

	void Unlock()
	{
		LeaveCriticalSection(m_pcs);
		TRACE(_T("LC %d %s\n") , GetCurrentThreadId() , m_strFunc);
	}

	void Lock()
	{
		TRACE(_T("EC %d %s\n") , GetCurrentThreadId(), m_strFunc);
		EnterCriticalSection(m_pcs);
	}


protected:
	CRITICAL_SECTION*	m_pcs;
	CString				m_strFunc;

};
typedef void (CALLBACK* NOTIFYPROC)(ClientContext*);

typedef CList<ClientContext*, ClientContext* > ContextList;

class CIOCPServer
{
private:
	// members
	NOTIFYPROC					m_pNotifyProc;
	// ��ɶ˿ھ��
	HANDLE						m_hCompletionPort;
	// ����֪ͨ�߳�ϵͳ�˳����¼���Ϊ���ܹ����õ��˳��߳�
	HANDLE						m_hShutdownEvent; 
	// ������������ַ������˿�
	CString						m_strIP;
	int							m_nPort;
	// ���������
	int							m_nMaxConnections; 
	// bytes header
	BYTE						m_bPacketFlag[4];
	// �ж��Ƿ�����
	BOOL						m_bIsStart;
	// ����socket
	SOCKET						m_socListen;  
	// Event for handling Network IO
	HANDLE						m_hNetworkIO;
	// Handle for listen thread.
	HANDLE						m_listenThread;
	// ���Ͽ���������ʱ��true��Ĭ�� false
	BOOL						m_bDisconnectAll;
	// when shutdown is called , this set true.
	BOOL						m_bTimeToKill;
	//
	static CRITICAL_SECTION		m_cs;
	//
	ContextList					m_listContexts;
	//
	ContextList					m_listFreePool;
	// the number of running worker threads
	int							m_nWorkerCnt;
	// the pointer of worker threads
	HANDLE*						m_phWorkerThreads;
private:
	// private functions
	void PostRecv(ClientContext* pContext);

	void CloseCompletionPort();
	void OnAccept();
	BOOL InitializeIOCP(void);
	void Stop();
	ClientContext*  AllocateContext();
	BOOL AssociateSocketWithCompletionPort(SOCKET device, HANDLE hCompletionPort, DWORD dwCompletionKey);
	void RemoveStaleClient(ClientContext* pContext, BOOL bGraceful);
	void MoveToFreePool(ClientContext *pContext);
	
	static unsigned __stdcall ListenThreadProc(LPVOID lpVoid);
	static unsigned __stdcall ThreadPoolFunc(LPVOID WorkContext);
	// IO ���֮����õĺ��������̳߳���ʹ�á�
	BOOL OnClientInitializing	(ClientContext* pContext, DWORD dwSize = 0);
	BOOL OnClientReading		(ClientContext* pContext, DWORD dwSize = 0);
	BOOL OnClientWriting		(ClientContext* pContext, DWORD dwSize = 0);
	// ��ñ����Ĵ���������
	int GetNoOfProcessors();
	// Called in worker threads
	void ProcessIOMessage(IOType iotype,ClientContext *pClientContext,DWORD dwIoSize);
public:
	// functions provide for users
	CIOCPServer(void);
	virtual ~CIOCPServer();
	BOOL Initialize(NOTIFYPROC pNotifyProc,IN int nMaxConnections,IN int nPort);
	void Send(IN ClientContext* pContext);
	BOOL IsRunning();
	void Shutdown();
	void ResetConnection(ClientContext* pContext);


};