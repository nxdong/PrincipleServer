#pragma once


// CFileDlg 对话框
class CIOCPServer;
struct ClientContext;
class CFileDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CFileDlg)

public:
	CFileDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CFileDlg();
	CIOCPServer* m_Server;  // pointer to server
// 对话框数据
	enum { IDD = IDD_FILE_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	void OnReceive(ClientContext* pContext);
};
