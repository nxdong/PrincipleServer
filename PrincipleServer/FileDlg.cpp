// FileDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "PrincipleServer.h"
#include "FileDlg.h"
#include "afxdialogex.h"


// CFileDlg �Ի���

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


// CFileDlg ��Ϣ�������
void CFileDlg::OnReceive(ClientContext* pContext)
{
	//TODO: add message process their.
	return;
}