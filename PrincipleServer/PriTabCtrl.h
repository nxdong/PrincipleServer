#pragma once


// CPriTabCtrl

class CPriTabCtrl : public CTabCtrl
{
	DECLARE_DYNAMIC(CPriTabCtrl)

public:
	CPriTabCtrl();
	virtual ~CPriTabCtrl();

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnTcnSelchange(NMHDR *pNMHDR, LRESULT *pResult);
};


