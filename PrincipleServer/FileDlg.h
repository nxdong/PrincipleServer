#pragma once


// CFileDlg �Ի���
class CIOCPServer;
struct ClientContext;
class CFileDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CFileDlg)

public:
	CFileDlg(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CFileDlg();
	CIOCPServer* m_Server;  // pointer to server
// �Ի�������
	enum { IDD = IDD_FILE_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
public:
	void OnReceive(ClientContext* pContext);
};
