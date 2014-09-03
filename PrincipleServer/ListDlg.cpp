// ListDlg.cpp : 实现文件
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
// CListDlg 对话框

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


// CListDlg 消息处理程序



void CListDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);

	// TODO: 在此处添加消息处理程序代码
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

	// TODO:  在此添加额外的初始化
	DWORD dwStyle = m_ListCtrl.GetExtendedStyle();
	dwStyle |= LVS_EX_FULLROWSELECT;//选中某行使整行高亮（只适用与report风格的listctrl）
	dwStyle |= LVS_EX_GRIDLINES;//网格线（只适用与report风格的listctrl）
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
	// 异常: OCX 属性页应返回 FALSE
}
void CListDlg::OnReceive(ClientContext *pContext)
{
	//TODO:add message process their
	return;
}