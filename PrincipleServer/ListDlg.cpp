// ListDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "PrincipleServer.h"
#include "ListDlg.h"
#include "afxdialogex.h"

typedef struct
{
	CString	title;
	double		nWidth;
}COLUMNSTRUCT;
COLUMNSTRUCT m_Column_Data[] = 
{
	{_T("ID")	 ,				0.05	},
	{_T("Wan/Lan"),				0.15	},
	{_T("Hostname"),			0.15	},
	{_T("Version"),				0.35	},
	{_T("Relay"),				0.1		},
	{_T("Comment"),				0.2		},
};
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

	int width = rect.Width();
	int column_Count = 6;  // six columns 
	for (int i=0;i<column_Count;i++)
	{
		m_ListCtrl.SetColumnWidth(i,(int)width*m_Column_Data[i].nWidth);
	}
}


BOOL CListDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  �ڴ���Ӷ���ĳ�ʼ��
	DWORD dwStyle = m_ListCtrl.GetExtendedStyle();
	dwStyle |= LVS_EX_FULLROWSELECT;//ѡ��ĳ��ʹ���и�����ֻ������report����listctrl��
	dwStyle |= LVS_EX_GRIDLINES;//�����ߣ�ֻ������report����listctrl��
	m_ListCtrl.SetExtendedStyle(dwStyle);
	CRect rect;
	GetClientRect(&rect);
	int width = rect.Width();
	int column_Count = 6;  // six columns 
	for (int i=0;i<column_Count;i++)
	{
		m_ListCtrl.InsertColumn(i,m_Column_Data[i].title,LVCFMT_LEFT,(int)width*m_Column_Data[i].nWidth);
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	// �쳣: OCX ����ҳӦ���� FALSE
}
void CListDlg::OnReceive(ClientContext *pContext)
{
	//TODO:add message process their
	return;
}