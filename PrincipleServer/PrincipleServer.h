
// PrincipleServer.h : PrincipleServer Ӧ�ó������ͷ�ļ�
//
#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"       // ������


// CPrincipleServerApp:
// �йش����ʵ�֣������ PrincipleServer.cpp
//

class CPrincipleServerApp : public CWinApp
{
public:
	CPrincipleServerApp();


// ��д
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

// ʵ��

public:
	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
};

extern CPrincipleServerApp theApp;
