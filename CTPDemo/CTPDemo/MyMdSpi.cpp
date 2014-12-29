#include "stdafx.h"
#include <iostream>
#include <fstream>
#include <string>
#include <windows.h>
#include "MyMdSpi.h"
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
std::map<std::string, MyFileMapManager<CThostFtdcDepthMarketDataField>> g_mapData;
MyFileMapManager<CThostFtdcDepthMarketDataField> g_totalData;
HANDLE g_totalDataMutex = CreateMutex(NULL, FALSE, _T("g_totalData_Mutex"));

void AddInstrumentIDType(std::string intrusmentID)
{
    g_mapData[intrusmentID].GetDataCount();
}

/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////

CMyMdSpi::CMyMdSpi()
{
    ClearMyParts();
}

CMyMdSpi::~CMyMdSpi()
{
}

void CMyMdSpi::ClearMyParts()
{
    m_nRequestID = 0;
    m_pMdApi = 0;
    memset(m_brokerID, 0, sizeof(m_brokerID));
    memset(m_investorID, 0, sizeof(m_investorID));
    memset(m_password, 0, sizeof(m_password));
    m_observers.clear();
    memset(m_addr1, 0, 256);
    memset(m_addr2, 0, 256);
}

void CMyMdSpi::PostSimpleMsgToObservers(UINT Msg, WPARAM wParam, LPARAM lParam)
{
    std::vector<HWND>::const_iterator itor = m_observers.begin();
    for(; itor!=m_observers.end(); ++itor)
    {
        ::PostMessage(*itor, Msg, wParam, lParam);
    }
}

void CMyMdSpi::OnFrontConnected()
{
    CThostFtdcReqUserLoginField req;
    memset(&req, 0, sizeof(req));
    strcpy(req.BrokerID, m_brokerID);
    strcpy(req.UserID, m_investorID);
    strcpy(req.Password, m_password);
    m_pMdApi->ReqUserLogin(&req, m_nRequestID);
    m_nRequestID++;

    PostSimpleMsgToObservers(WM_OnFrontConnected, NULL, NULL);
}

void CMyMdSpi::OnFrontDisconnected(int nReason)
{
    PostSimpleMsgToObservers(WM_OnFrontDisconnected, NULL, NULL);
}

void CMyMdSpi::OnHeartBeatWarning(int nTimeElapse)
{
    PostSimpleMsgToObservers(WM_OnHeartBeatWarning, NULL, NULL);
}

void CMyMdSpi::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    PostSimpleMsgToObservers(WM_OnRspUserLogin, NULL, NULL);
    if(bIsLast && pRspInfo->ErrorID==0)
    {}
}

void CMyMdSpi::OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    PostSimpleMsgToObservers(WM_OnRspUserLogout, NULL, NULL);
    if(bIsLast && pRspInfo->ErrorID==0){}
}

void CMyMdSpi::OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    std::vector<HWND>::const_iterator itor = m_observers.begin();
    for(; itor!=m_observers.end(); ++itor)
    {
        CThostFtdcRspInfoField *p = new CThostFtdcRspInfoField;
        memcpy(p, pRspInfo, sizeof(CThostFtdcRspInfoField));
        ::PostMessage(*itor, WM_OnRspError, (WPARAM)p, NULL);
    }
}

void CMyMdSpi::OnRspSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    PostSimpleMsgToObservers(WM_OnRspSubMarketData, NULL, NULL);
    if(bIsLast && pRspInfo->ErrorID==0){}
}

void CMyMdSpi::OnRspUnSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    PostSimpleMsgToObservers(WM_OnRspUnSubMarketData, NULL, NULL);
    if(bIsLast && pRspInfo->ErrorID==0){}
}

void CMyMdSpi::OnRspSubForQuoteRsp(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    PostSimpleMsgToObservers(WM_OnRspSubForQuoteRsp, NULL, NULL);
}

void CMyMdSpi::OnRspUnSubForQuoteRsp(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    PostSimpleMsgToObservers(WM_OnRspUnSubForQuoteRsp, NULL, NULL);
}

void CMyMdSpi::OnRtnDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData)
{
    int iErr = ID_SUCCESS;
    WaitForSingleObject(g_totalDataMutex, INFINITE);
    int nDataCount = g_totalData.AddItem(iErr, pDepthMarketData);
    g_mapData[std::string(pDepthMarketData->InstrumentID)].AddItem(iErr, pDepthMarketData);
    if(iErr != ID_SUCCESS)
        return;
    ReleaseMutex(g_totalDataMutex);
    PostSimpleMsgToObservers(WM_OnRtnDepthMarketData, (WPARAM)nDataCount, NULL);
}

void CMyMdSpi::OnRtnForQuoteRsp(CThostFtdcForQuoteRspField *pForQuoteRsp)
{
    PostSimpleMsgToObservers(WM_OnRtnForQuoteRsp, NULL, NULL);
}

void CMyMdSpi::Login(int& iErr)
{
    m_pMdApi = CThostFtdcMdApi::CreateFtdcMdApi("./thostmduserapi.dll");
    m_pMdApi->RegisterSpi(this);
    m_pMdApi->RegisterFront(m_addr1);
    m_pMdApi->RegisterFront(m_addr2);
    m_pMdApi->Init();
}

void CMyMdSpi::Logout(int& iErr)
{
    CThostFtdcUserLogoutField logout;
    memset(&logout, 0, sizeof(logout));
    strcpy(logout.BrokerID, m_brokerID);
    strcpy(logout.UserID, m_investorID);
    m_pMdApi->ReqUserLogout(&logout, m_nRequestID);
    m_nRequestID++;
}

void CMyMdSpi::StartSubscribe(int& iErr)
{
    SYSTEMTIME sysTime;
    GetLocalTime(&sysTime);
    char strFile[64] = {0};
    std::vector<std::string>::iterator itor = m_vIntrusmentID.begin();
    for(; itor!=m_vIntrusmentID.end(); ++itor)
    {
        sprintf(strFile, _T("%s-%4d-%2d-%2d.txt"), itor->c_str(), sysTime.wYear, sysTime.wMonth, sysTime.wDay);
        g_mapData[*itor].MapToFile(iErr, strFile);
        if(iErr != ID_SUCCESS)
            return;
    }
    sprintf(strFile, _T("0total-%4d-%2d-%2d.txt"), sysTime.wYear, sysTime.wMonth, sysTime.wDay);
    g_totalData.MapToFile(iErr, strFile);

    int nCount = m_vIntrusmentID.size();
    char** ppInstrumentID = new char*[nCount];
    itor = m_vIntrusmentID.begin();
    for(int i=0; i<nCount; i++)
    {
        assert(itor != m_vIntrusmentID.end());
        ppInstrumentID[i] = new char[sizeof(TThostFtdcInstrumentIDType)];
        memset(ppInstrumentID[i], 0, sizeof(TThostFtdcInstrumentIDType));
        strncpy(ppInstrumentID[i], itor->c_str(), itor->length());
        ++itor;
    }
    m_pMdApi->UnSubscribeMarketData(ppInstrumentID, nCount);
    m_pMdApi->SubscribeMarketData(ppInstrumentID, nCount);
}

void CMyMdSpi::StopSubscribe(int& iErr)
{
    int nCount = m_vIntrusmentID.size();
    char** ppInstrumentID = new char*[nCount];
    std::vector<std::string>::iterator itor = m_vIntrusmentID.begin();
    for(int i=0; i<nCount; i++)
    {
        assert(itor != m_vIntrusmentID.end());
        ppInstrumentID[i] = new TThostFtdcInstrumentIDType;
        strncpy(ppInstrumentID[i], itor->c_str(), itor->length());
        ++itor;
    }
    m_pMdApi->UnSubscribeMarketData(ppInstrumentID, nCount);

    g_totalData.UnMapFromFile();
    itor = m_vIntrusmentID.begin();
    for(; itor!=m_vIntrusmentID.end(); ++itor)
    {
        g_mapData[*itor].UnMapFromFile();
    }
}

void CMyMdSpi::AddIntrusmentType(int& iErr, std::string strIntrusmentID)
{
    size_t nCount = m_vIntrusmentID.size();
    for(size_t i=0; i<nCount; i++)
    {
        if(m_vIntrusmentID[i] == strIntrusmentID)
        {
            iErr = ID_WARN_INSTRUMENT_TYPE_ALREADY_EXISTS;
            return;
        }
    }
    m_vIntrusmentID.push_back(strIntrusmentID);
    iErr = ID_SUCCESS;
    return;
}