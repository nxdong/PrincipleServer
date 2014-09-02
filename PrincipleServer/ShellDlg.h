#pragma once


// CShellDlg 对话框

class CShellDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CShellDlg)

public:
	CShellDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CShellDlg();

// 对话框数据
	enum { IDD = IDD_SHELL_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
};
