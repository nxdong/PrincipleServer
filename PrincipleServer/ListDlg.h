#pragma once
#include "afxcmn.h"


// CListDlg �Ի���
class CIOCPServer;
struct ClientContext;
class CListDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CListDlg)

public:
	CListDlg(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CListDlg();

// �Ի�������
	enum { IDD = IDD_LIST_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
public:
	CIOCPServer *m_pIocpServer; //  This is the pointer of the iocp server
	CListCtrl m_ListCtrl;		//	This is the handle of list ctrl 
	afx_msg void OnSize(UINT nType, int cx, int cy);
	virtual BOOL OnInitDialog();
	void OnReceive(ClientContext *pContext);
};
