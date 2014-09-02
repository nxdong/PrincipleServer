
// ChildView.cpp : CChildView ���ʵ��
//

#include "stdafx.h"
#include "PrincipleServer.h"
#include "ChildView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CChildView

CChildView::CChildView()
{
}

CChildView::~CChildView()
{
}


BEGIN_MESSAGE_MAP(CChildView, CWnd)
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_WM_CREATE()
	ON_MESSAGE(UM_VIEW_CREATED,OnViewCreated)
END_MESSAGE_MAP()



// CChildView ��Ϣ�������

BOOL CChildView::PreCreateWindow(CREATESTRUCT& cs) 
{
	if (!CWnd::PreCreateWindow(cs))
		return FALSE;

	cs.dwExStyle |= WS_EX_CLIENTEDGE;
	cs.style &= ~WS_BORDER;
	cs.lpszClass = AfxRegisterWndClass(CS_HREDRAW|CS_VREDRAW|CS_DBLCLKS, 
		::LoadCursor(NULL, IDC_ARROW), reinterpret_cast<HBRUSH>(COLOR_WINDOW+1), NULL);
	return TRUE;
}

void CChildView::OnPaint() 
{
	CPaintDC dc(this); // ���ڻ��Ƶ��豸������
	
	// TODO: �ڴ˴������Ϣ����������
	
	// ��ҪΪ������Ϣ������ CWnd::OnPaint()
}



void CChildView::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize(nType, cx, cy);
	//when 
	if (NULL == m_TabCtrl.m_hWnd)
		return;
	CRect rect;   
	GetClientRect(&rect);
	m_TabCtrl.MoveWindow(rect);
	
	CRect tabRect;
	m_TabCtrl.GetClientRect(&tabRect);
	tabRect.left += 2;                  
	tabRect.right -= 2;   
	tabRect.top += 22;   
	tabRect.bottom -= 2; 
	m_ListDlg.MoveWindow(tabRect);
	m_ShellDlg.MoveWindow(tabRect);
	m_FileDlg.MoveWindow(tabRect);
	// TODO: �ڴ˴������Ϣ����������
}


int CChildView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  �ڴ������ר�õĴ�������
	PostMessage(UM_VIEW_CREATED);

	return 0;
}
LRESULT CChildView::OnViewCreated(WPARAM wParam , LPARAM lParam)
{
	CRect viewRect;
	GetClientRect(&viewRect);
	m_TabCtrl.Create(TCS_TABS | TCS_FIXEDWIDTH | WS_CHILD | WS_VISIBLE,viewRect,this,NULL);
	m_TabCtrl.InsertItem(0,_T("Computer"));
	m_TabCtrl.InsertItem(1,_T("Cmd"));
	m_TabCtrl.InsertItem(2,_T("File"));
	m_ListDlg.Create(IDD_LIST_DLG,&m_TabCtrl);
	m_ShellDlg.Create(IDD_SHELL_DLG,&m_TabCtrl);
	m_FileDlg.Create(IDD_FILE_DLG,&m_TabCtrl);
	// move dialogs matching tab control
	viewRect.left += 2;                  
	viewRect.right -= 2;   
	viewRect.top += 22;   
	viewRect.bottom -= 2; 
	m_ListDlg.MoveWindow(viewRect);
 	m_ShellDlg.MoveWindow(viewRect);
 	m_FileDlg.MoveWindow(viewRect);
	// set which window to show
 	m_ListDlg.ShowWindow(TRUE);
 	m_ShellDlg.ShowWindow(FALSE);
 	m_FileDlg.ShowWindow(FALSE);

	return 0;
}