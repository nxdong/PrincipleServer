#pragma once


// CShellDlg �Ի���

class CShellDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CShellDlg)

public:
	CShellDlg(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CShellDlg();

// �Ի�������
	enum { IDD = IDD_SHELL_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
};
