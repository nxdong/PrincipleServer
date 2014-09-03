
/********************************************************************
	created:	2014/09/02
	created:	2:9:2014   7:26
	filename: 	H:\PrincipleServer\PrincipleServer\macrosForServer.h
	file ext:	h
	author:		Principle
	
	purpose:	This file defines macros and struct used in server .
*********************************************************************/
#pragma once


/* user defined message,when view was crated ,this 
	 message transmit to view class. In order to 
	 creat tab ctrl*/
#define UM_VIEW_CREATED		(WM_USER + 101)
/* this is the id of the tab ctrl 
	it will used  in message macro */
#define IDC_TAB_CTRL		(WM_USER + 102)