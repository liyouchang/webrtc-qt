// phupdate.h: interface for the CBaseThread class.
//
//////////////////////////////////////////////////////////////////////
/* ! \file phupdate.h
  \author skyvense
  \date   2009-09-14
  \brief PHDDNS 客户端实现
*/

#ifndef _PHUPDATE_H_
#define _PHUPDATE_H_

#include "../ddns.h"
#include "../phglobal/phglobal.h"
#include "../phsocket/phsocket.h"
#include "../../../../common_api.h"

// ! 花生壳DDNS客户端实现基类
/* !
*/
// ! 步进调用，配置好参数后需要立即进入此函数，函数返回下次需要执行本函数的时间（秒数）
extern	int phddns_step(PHGlobal *phglobal);

// ! 停止花生壳DDNS更新，重新配置参数后可进入另一个
extern	void phddns_stop(PHGlobal *phglobal);

// ! 初始化socket
extern	unsigned int  InitializeSockets(PHGlobal *phglobal);
// ! 关闭所有socket
extern	unsigned int DestroySockets(PHGlobal *phglobal);
// ! 与DDNS服务器连接的TCP主过程
//extern	int ExecuteUpdate(PHGlobal *phglobal,int iSock);
// ! 启动UDP“连接”
//extern	unsigned int BeginKeepAlive(PHGlobal *phglobal,int iSock);
// ! 发送一个UDP心跳包
//extern	unsigned int SendKeepAlive(PHGlobal *phglobal, int iSock,int opCode);
// ! 接收心跳包返回
//extern	int RecvKeepaliveResponse(PHGlobal *phglobal,int iSock);

#endif
