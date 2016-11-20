//
// Created by biggreyhairboy on 7/21/16.
//
#ifndef CTPCLIENTDEMO_MARKETDATAHANDLE_H
#define CTPCLIENTDEMO_MARKETDATAHANDLE_H

#include <string>
#include "DBDriver.h"
#include "ctpapi_linux64/ThostFtdcMdApi.h"
using namespace std;

class MarketDataHandle : public CThostFtdcMdSpi{
public:
    CThostFtdcMdApi* pUserApi;
    char FRONT_ADDR_quote[];
    TThostFtdcBrokerIDType brokerIDType;
    TThostFtdcInvestorIDType investorIDType;
    TThostFtdcPasswordType passwordType;
    const char* ppIntrumentID[10];
    int InstrumentID;
    DBDriver* dbDriver;
    int iRequestID_quote = 0;
    //double OpenPrice = 0;
    MarketDataHandle(char *, TThostFtdcBrokerIDType, TThostFtdcInvestorIDType, TThostFtdcPasswordType, DBDriver *,
                     vector<string>, int);
    //virtual void OnRspError(CThostFtdcRspInfoField* pRspInfo, int nRquestID, bool bIsLast);

    ///���ͻ����뽻�׺�̨������ͨ������ʱ����δ��¼ǰ�����÷��������á�
    virtual void OnFrontConnected();

    ///���ͻ����뽻�׺�̨ͨ�����ӶϿ�ʱ���÷��������á���������������API���Զ��������ӣ��ͻ��˿ɲ�������
    ///@param nReason ����ԭ��
    ///        0x1001 �����ʧ��
    ///        0x1002 ����дʧ��
    ///        0x2001 ����������ʱ
    ///        0x2002 ��������ʧ��
    ///        0x2003 �յ�������
    virtual void OnFrontDisconnected(int nReason);

    ///������ʱ���档����ʱ��δ�յ�����ʱ���÷��������á�
    ///@param nTimeLapse �����ϴν��ձ��ĵ�ʱ��
    virtual void OnHeartBeatWarning(int nTimeLapse);


    ///��¼������Ӧ
    virtual void OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) ;

//    ///�ǳ�������Ӧ
//    virtual void OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) ;

    ///����Ӧ��
    virtual void OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) ;

    ///��������Ӧ��
    virtual void OnRspSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) ;

//    ///ȡ����������Ӧ��
//    virtual void OnRspUnSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    ///����ѯ��Ӧ��
    virtual void OnRspSubForQuoteRsp(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) ;

    ///ȡ������ѯ��Ӧ��
    virtual void OnRspUnSubForQuoteRsp(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) ;

    ///�������֪ͨ
    virtual void OnRtnDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData) ;

    ///ѯ��֪ͨ
    virtual void OnRtnForQuoteRsp(CThostFtdcForQuoteRspField *pForQuoteRsp) ;

private:
    void ReqUserLogin();
    void SubscribeMarketData(char* [], int);
    void SubscribeForQuoteRsp(char* [], int);
    bool IsErrorRspInfo(CThostFtdcRspInfoField *pRspInfo);
};


#endif //CTPCLIENTDEMO_MARKETDATAHANDLE_H
