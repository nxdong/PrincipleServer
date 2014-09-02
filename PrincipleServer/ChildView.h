
// ChildView.h : CChildView ��Ľӿ�
//


#pragma once
#include "PriTabCtrl.h"
#include "ListDlg.h"
#include "ShellDlg.h"
#include "FileDlg.h"
// CChildView ����

class CChildView : public CWnd
{
// ����
public:
	CChildView();

// ����
public:
	CPriTabCtrl m_TabCtrl;		//	Create tab control
	CListDlg	m_ListDlg;		//	Create list dialog
	CShellDlg	m_ShellDlg;		//	Create shell dialog
	CFileDlg	m_FileDlg;		//	Create file dialog
// ����
public:

// ��д
	protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

// ʵ��
public:
	virtual ~CChildView();

	// ���ɵ���Ϣӳ�亯��
protected:
	afx_msg void OnPaint();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	/* When view was created ,this function was called.Please initialize
	in this function*/
	afx_msg LRESULT OnViewCreated(WPARAM wParam , LPARAM lParam);
};

