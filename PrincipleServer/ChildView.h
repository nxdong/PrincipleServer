
// ChildView.h : CChildView 类的接口
//


#pragma once
#include "PriTabCtrl.h"
#include "ListDlg.h"
#include "ShellDlg.h"
#include "FileDlg.h"
// CChildView 窗口

class CChildView : public CWnd
{
// 构造
public:
	CChildView();

// 特性
public:
	CPriTabCtrl m_TabCtrl;		//	Create tab control
	CListDlg	m_ListDlg;		//	Create list dialog
	CShellDlg	m_ShellDlg;		//	Create shell dialog
	CFileDlg	m_FileDlg;		//	Create file dialog
// 操作
public:

// 重写
	protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

// 实现
public:
	virtual ~CChildView();

	// 生成的消息映射函数
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

