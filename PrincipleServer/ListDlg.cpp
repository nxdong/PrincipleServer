// ListDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "PrincipleServer.h"
#include "ListDlg.h"
#include "afxdialogex.h"


// CListDlg �Ի���

IMPLEMENT_DYNAMIC(CListDlg, CDialogEx)

CListDlg::CListDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CListDlg::IDD, pParent)
{

}

CListDlg::~CListDlg()
{
}

void CListDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_ListCtrl);
}


BEGIN_MESSAGE_MAP(CListDlg, CDialogEx)
	
	ON_WM_SIZE()
END_MESSAGE_MAP()


// CListDlg ��Ϣ�������



void CListDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);

	// TODO: �ڴ˴������Ϣ����������
	if(NULL == m_ListCtrl.m_hWnd)
		return;
	CRect rect;
	GetClientRect(&rect);
	m_ListCtrl.MoveWindow(rect);
}


BOOL CListDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  �ڴ���Ӷ���ĳ�ʼ��
	m_ListCtrl.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES );

	return TRUE;  // return TRUE unless you set the focus to a control
	// �쳣: OCX ����ҳӦ���� FALSE
}
