// ShellDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "PrincipleServer.h"
#include "ShellDlg.h"
#include "afxdialogex.h"


// CShellDlg 对话框

IMPLEMENT_DYNAMIC(CShellDlg, CDialogEx)

CShellDlg::CShellDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CShellDlg::IDD, pParent)
{

}

CShellDlg::~CShellDlg()
{
}

void CShellDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CShellDlg, CDialogEx)
END_MESSAGE_MAP()


// CShellDlg 消息处理程序
