#pragma once
#include "afxcmn.h"


// CListDlg 对话框
class CIOCPServer;
struct ClientContext;
class CListDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CListDlg)

public:
	CListDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CListDlg();

// 对话框数据
	enum { IDD = IDD_LIST_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	CIOCPServer *m_pIocpServer; //  This is the pointer of the iocp server
	CListCtrl m_ListCtrl;		//	This is the handle of list ctrl 
	afx_msg void OnSize(UINT nType, int cx, int cy);
	virtual BOOL OnInitDialog();
	void OnReceive(ClientContext *pContext);
};
