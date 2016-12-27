//
// Created by biggreyhairboy on 7/21/16.
//


#include "MarketDataHandle.h"
#include <cstring>
#include <iostream>
#include <iomanip>
#include <mutex>
#include "boost/format.hpp"
#include <math.h>
#include <string>
#include <iostream>
#include <vector>
using namespace std;
using boost::format;



extern double lastorderprice;
mutex lastorderpricemutex;
void UpdateLastPrice(double price)
{
    lock_guard<mutex> guard(lastorderpricemutex);
    lastorderprice = price;
//    cout << "last mutex price is " << lastorderprice << endl;
}
MarketDataHandle::MarketDataHandle(CThostFtdcMdApi* iMdapi, CThostFtdcTraderApi *traderapi, char *front_address, TThostFtdcBrokerIDType brokerid,
                                   TThostFtdcInvestorIDType investorid, TThostFtdcPasswordType password,
                                   DBDriver *dbdriver, vector<string> ppinsturment, int insturmentid)
{
    int abc;
    pTraderApi = traderapi;
    pUserApi = iMdapi;
    strcpy(this->FRONT_ADDR_quote, front_address);
    strcpy(this->brokerIDType, brokerid);
    strcpy(this->investorIDType, investorid);
    strcpy(this->passwordType, password);
    this->iRequestID_quote = 0;
    strppInstrument = ppinsturment;
    InstrumentID = insturmentid;
    dbDriver = dbdriver;
    MarketTrend.insert({0, 0});
    MarketTrend.insert({1, 0});
    MarketTrend.insert({2, 0});
}

void MarketDataHandle::OnFrontDisconnected(int nReason){
    cerr << "---->>> " << "OnFrontDisconnected" << endl;
    cerr << "--->>> Reason = " << nReason << endl;
}

void MarketDataHandle::OnHeartBeatWarning(int nTimeLapse){
    cerr << "--->>> " << "OnHeartBeatWarning" <<endl;
    cerr << "--->>> nTimerLapse = " << nTimeLapse << endl;
}

void MarketDataHandle::OnFrontConnected()
{
    cerr << "--->>> " << "OnFrontConnected" <<endl;
    ReqUserLogin();
}


void MarketDataHandle::ReqUserLogin() {
    CThostFtdcReqUserLoginField req;
    memset(&req, 0, sizeof(req));
    strcpy(req.BrokerID, brokerIDType);
    strcpy(req.UserID, investorIDType);
    strcpy(req.Password, passwordType);
    int iResult = pUserApi->ReqUserLogin(&req, ++iRequestID_quote);
    cerr << "--->>> sending user login request: " << ((iResult == 0) ? "success" : "fail" )<< endl;
}

void MarketDataHandle::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
    cerr << "--->>> " << "OnRspUserLogin" << endl;
    if (bIsLast && !IsErrorRspInfo(pRspInfo))
    {
        cerr << "--->>> current trading date = " << pUserApi->GetTradingDay() << endl;
    }

    char** instrumentarry= new char*[strppInstrument.size()];
//    int n = 0;
//    //����ʹ��std::copy
//    for(vector<string>::iterator iter = strppInstrument.begin(); iter != strppInstrument.end(); iter++)
//    {
//        //��vector����ת����char* ����
//        strcpy(instrumentarry[n], (*iter).c_str());
//        n++;
//    }
    for(size_t i = 0; i < strppInstrument.size(); i++)
    {
        instrumentarry[i] = new char[strppInstrument[i].size() + 1];
        std::strcpy(instrumentarry[i], strppInstrument[i].c_str());
    }
    SubscribeMarketData(instrumentarry, InstrumentID);
}

void MarketDataHandle::SubscribeMarketData(char* ppIntrumentID[], int iInstrumentID) {
    int iResult = pUserApi->SubscribeMarketData(ppIntrumentID, iInstrumentID);
    cerr << "--->>> request subscribe market data: " << ((iResult == 0) ? "success" : "fail") << endl;
    //SubscribeMarketData(ppIntrumentID,InstrumentID);
    //SubscribeForQuoteRsp(ppIntrumentID, InstrumentID);
}
void MarketDataHandle::SubscribeForQuoteRsp(char* ppIntrumentID[], int iInstrumentID) {
    int iResult = pUserApi->SubscribeForQuoteRsp(ppIntrumentID, iInstrumentID);
    cerr << "--->>> request subscribe quoting: " << ((iResult == 0) ? "success" : "fail") << endl;
    cout << "--->>> symbol " << ppIntrumentID[0] << endl;
}

void MarketDataHandle::OnRspSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast){
    cerr << "OnRspSubmarketData" << endl;

}

void MarketDataHandle::OnRspSubForQuoteRsp(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast){
    cerr << "OnRspSubForQuoteRsp" << endl;
}

void MarketDataHandle::OnRspUnSubForQuoteRsp(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast){
    cerr << "OnRspUnSubForQuoteRsp" << endl;
}

void ColorfulConsolePrint(TThostFtdcTimeType time, double lastprice, double volumechange, double openinterestchange, string ticktype, int color)
{
    switch(color)
    {
        //��ɫ
        case 0:
            cout << boost::format("%1%")%string(time) <<setw(10)<<  setprecision(2) << lastprice << setw(10)
                 << boost::format("\033[;m%1%\033[0m")%to_string((long)volumechange) << setw(10)
                 <<boost::format("\033[;m%1%\033[0m")%to_string((long)volumechange) << setw(10)  << fixed << right << boost::format("\033[;m%1%\033[0m")%ticktype<< endl;
            break;
            //"\033[;36msome text\033[0m"; ����ɫ
        case 1:
            cout << boost::format("%1%")%string(time) <<setw(10) << setprecision(2) << lastprice << setw(10)
                 <<  boost::format("\033[;31m%1%\033[0m")%to_string((long)volumechange)  << setw(10)
                 << boost::format("\033[;31m%1%\033[0m")%to_string((long)openinterestchange) << setw(10)  << boost::format("\033[;31m%1%\033[0m")%ticktype << endl;
            break;
        case 2:
            cout << boost::format("%1%")%string(time) <<setw(10) << setprecision(2) <<lastprice << setw(10)
                 << boost::format("\033[;32m%1%\033[0m")%to_string((long)volumechange) << setw(10)
                 <<boost::format("\033[;32m%1%\033[0m")%to_string((long)openinterestchange)<< setw(10) << boost::format("\033[;32m%1%\033[0m")%ticktype << endl;
            break;
            //"\033[;31msome text\033[0m";

        default:
            cout << "error " << endl;
    }
}

void MarketDataHandle::OnRtnDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData){
    //todo:д��־�������������
    UpdateLastPrice(pDepthMarketData->LastPrice);

    //��ô��ʼ���۸��ָ��
    if (pPreDepthMarketData.LastPrice == 0)
    {
        //����Ӧ����Ҫ��ȫ����
        pPreDepthMarketData = *pDepthMarketData;
    }
    dbDriver->ExcuteQuery(pDepthMarketData);
    OpenInterestChange = pDepthMarketData->OpenInterest - pPreDepthMarketData.OpenInterest;
    VolumeChange = pDepthMarketData->Volume - pPreDepthMarketData.Volume;

    if (OpenInterestChange > 0)
    {
        if(VolumeChange ==  abs(OpenInterestChange)) {
            ticktype = "˫��";
            ColorfulConsolePrint(pDepthMarketData->UpdateTime, pDepthMarketData->LastPrice, VolumeChange, OpenInterestChange,"˫��", 0);
            MarketTrend[1] = MarketTrend[1] + 1;
        }
        else {
            if(pDepthMarketData->LastPrice > pPreDepthMarketData.LastPrice) {
                ticktype = "�࿪";
                ColorfulConsolePrint(pDepthMarketData->UpdateTime, pDepthMarketData->LastPrice, VolumeChange, OpenInterestChange,"�࿪", 1);
                MarketTrend[1] = MarketTrend[1] + 1;
            }
            else if (pDepthMarketData->LastPrice > pPreDepthMarketData.LastPrice){
                ticktype = "�տ�";
                ColorfulConsolePrint(pDepthMarketData->UpdateTime, pDepthMarketData->LastPrice, VolumeChange, OpenInterestChange,"�տ�", 2);
                MarketTrend[2] = MarketTrend[2] + 1;
            } else {
                if (pDepthMarketData->LastPrice >= pPreDepthMarketData.AskPrice1){
                    ticktype = "�࿪";
                    ColorfulConsolePrint(pDepthMarketData->UpdateTime, pDepthMarketData->LastPrice, VolumeChange, OpenInterestChange,"�࿪", 1);
                    MarketTrend[1] = MarketTrend[1] + 1;
                } else if (pDepthMarketData->LastPrice <= pPreDepthMarketData.BidPrice1){
                    ticktype = "�տ�";
                    ColorfulConsolePrint(pDepthMarketData->UpdateTime, pDepthMarketData->LastPrice, VolumeChange, OpenInterestChange,"�տ�", 2);
                    MarketTrend[2] = MarketTrend[2] + 1;
                } else {
                    ticktype = "�������ʹ���";
                    ColorfulConsolePrint(pDepthMarketData->UpdateTime, pDepthMarketData->LastPrice, VolumeChange, OpenInterestChange,"�������ʹ���", 2);
                    //�޷������tickֱ��drop
                    // MarketTrend[2] = MarketTrend[2] + 1;
                }
            }
        }
    }
    else if (OpenInterestChange < 0)
    {
        if(VolumeChange ==  abs(OpenInterestChange)) {
            ticktype = "˫ƽ";
            ColorfulConsolePrint(pDepthMarketData->UpdateTime, pDepthMarketData->LastPrice, VolumeChange, OpenInterestChange,"˫ƽ", 0);
            MarketTrend[2] = MarketTrend[2] + 1;
        }

        else {
            if(pDepthMarketData->LastPrice > pPreDepthMarketData.LastPrice) {
                ticktype = "��ƽ";
                ColorfulConsolePrint(pDepthMarketData->UpdateTime, pDepthMarketData->LastPrice, VolumeChange, OpenInterestChange,"��ƽ", 1);
                MarketTrend[1] = MarketTrend[1] + 1;
            }
            else if (pDepthMarketData->LastPrice < pPreDepthMarketData.LastPrice){
                ticktype = "��ƽ";
                ColorfulConsolePrint(pDepthMarketData->UpdateTime, pDepthMarketData->LastPrice, VolumeChange, OpenInterestChange,"��ƽ", 2);
                MarketTrend[2] = MarketTrend[2] + 1;
            } else {
                if (pDepthMarketData->LastPrice >= pPreDepthMarketData.AskPrice1){
                    ticktype = "��ƽ";
                    ColorfulConsolePrint(pDepthMarketData->UpdateTime, pDepthMarketData->LastPrice, VolumeChange, OpenInterestChange,"��ƽ", 2);
                    MarketTrend[2] = MarketTrend[2] + 1;
                } else if (pDepthMarketData->LastPrice <= pPreDepthMarketData.BidPrice1){
                    ticktype = "��ƽ";
                    ColorfulConsolePrint(pDepthMarketData->UpdateTime, pDepthMarketData->LastPrice, VolumeChange, OpenInterestChange,"��ƽ", 1);
                    MarketTrend[1] = MarketTrend[1] + 1;
                } else {
                    ticktype = "ƽ�����ʹ���";
                    ColorfulConsolePrint(pDepthMarketData->UpdateTime, pDepthMarketData->LastPrice, VolumeChange, OpenInterestChange,"ƽ�����ʹ���", 1);
                    //�޷������tickֱ��drop
                    //MarketTrend[1] = MarketTrend[1] + 1;
                }
            }
        }
    }
    else if (OpenInterestChange == 0) {
        MarketTrend[0] = MarketTrend[0] + 1;
        if(pDepthMarketData->LastPrice >= pPreDepthMarketData.LastPrice) {
            ticktype = "�໻";
            ColorfulConsolePrint(pDepthMarketData->UpdateTime, pDepthMarketData->LastPrice, VolumeChange, OpenInterestChange,"�໻", 1);
        }
        else{
            ticktype = "�ջ�";
            ColorfulConsolePrint(pDepthMarketData->UpdateTime, pDepthMarketData->LastPrice, VolumeChange, OpenInterestChange,"�ջ�", 2);
        }
    }
    else {
        ticktype = "��������";
        ColorfulConsolePrint(pDepthMarketData->UpdateTime, pDepthMarketData->LastPrice, VolumeChange, OpenInterestChange,"��������", 0);
    }
    //todo: �߳�detach֮���Ƿ���Ե��ûص�����
    for(map<int, int>::iterator mapiter = MarketTrend.begin(); mapiter != MarketTrend.end(); mapiter++)
    {
        if(mapiter->second == 5){
            //order
            if(pTraderApi->GetApiVersion() != NULL)
            {
                CThostFtdcInputOrderField req;
                memset(&req, 0, sizeof(req));
                ///���͹�˾����
                strcpy(req.BrokerID, brokerIDType);
                ///Ͷ���ߴ���
                strcpy(req.InvestorID, investorIDType);
                ///��Լ����
                char abc[] = "rb1705";
                strcpy(req.InstrumentID, abc);
                ///��������
                //orderref
                strcpy(req.OrderRef, new char[3]);
                ///�û�����
                //	TThostFtdcUserIDType	UserID;
                ///�����۸�����: �޼�
                req.OrderPriceType = THOST_FTDC_OPT_LimitPrice;
                ///��������:
                req.Direction = '0';
                ///��Ͽ�ƽ��־: ����
                req.CombOffsetFlag[0] = THOST_FTDC_OF_Open;
                ///���Ͷ���ױ���־
                req.CombHedgeFlag[0] = THOST_FTDC_HF_Speculation;
                ///�۸�
                //����22900����
                req.LimitPrice = 22300;
                ///����: 1
                req.VolumeTotalOriginal = 1;
                ///��Ч������: ������Ч
                req.TimeCondition = THOST_FTDC_TC_GFD;
                ///GTD����
                //	TThostFtdcDateType	GTDDate;
                ///�ɽ�������: �κ�����
                req.VolumeCondition = THOST_FTDC_VC_AV;
                ///��С�ɽ���: 1
                req.MinVolume = 1;
                ///��������: ����
                req.ContingentCondition = THOST_FTDC_CC_Immediately;
                ///ֹ���
                //	TThostFtdcPriceType	StopPrice;
                ///ǿƽԭ��: ��ǿƽ
                req.ForceCloseReason = THOST_FTDC_FCC_NotForceClose;
                ///�Զ������־: ��
                req.IsAutoSuspend = 0;
                ///ҵ��Ԫ
                //	TThostFtdcBusinessUnitType	BusinessUnit;
                ///������
                //	TThostFtdcRequestIDType	RequestID;
                ///�û�ǿ����־: ��
                req.UserForceClose = 0;
                int iRequestID_trade = 2;
                int iResult = pTraderApi->ReqOrderInsert(&req, ++iRequestID_trade);
            }
            MarketTrend.clear();
            break;
        }

    }




//    //todo: matain a price queue of last five minutes
//    if (iRequestID_quote > 15)
//    {
//        return ;
//    }
    //������һ������tick
    pPreDepthMarketData = *pDepthMarketData;
}

void MarketDataHandle::OnRtnForQuoteRsp(CThostFtdcForQuoteRspField *pForQuoteRsp){
    cerr << "OnRtnForQuoteRsp" << endl;
}

bool MarketDataHandle::IsErrorRspInfo(CThostFtdcRspInfoField *pRspInfo) {
    bool bResult = ((pRspInfo) && (pRspInfo->ErrorID != 0));
    if(bResult)
    {
        cerr << "--->>> ErrorID = " << pRspInfo->ErrorID << ", ErrorMsg= " << pRspInfo->ErrorMsg << endl;
    }
    return bResult;
}

void MarketDataHandle::OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    cerr << "error on responce "<<pRspInfo->ErrorID << "  " << pRspInfo->ErrorMsg << endl;
}


