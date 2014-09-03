// FileDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "PrincipleServer.h"
#include "FileDlg.h"
#include "afxdialogex.h"


// CFileDlg 对话框

IMPLEMENT_DYNAMIC(CFileDlg, CDialogEx)

CFileDlg::CFileDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CFileDlg::IDD, pParent)
{

}

CFileDlg::~CFileDlg()
{
}

void CFileDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CFileDlg, CDialogEx)
END_MESSAGE_MAP()


// CFileDlg 消息处理程序
void CFileDlg::OnReceive(ClientContext* pContext)
{
	//TODO: add message process their.
	return;
}