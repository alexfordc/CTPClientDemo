/***********************************
 * Author: Patrick Yang
 * Encoding: GB10830
 * Date: 20160927
 * Function: Extract data from CTP interface and store to MySQL
 */
// todo: using valgrind to profile
// todo: query position at start and on return order
// change log/console output to english
// deployment and debug cannot share the same code because of configuration path
// qsql dependency problem on production environment
// get account position for a fixed interval(now it is set to 10 seconds)
//https://startupnextdoor.com/how-to-run-valgrind-in-clion-for-c-and-c-programs/

#include <iostream>
#include <chrono>
#include <sstream>
#include <vector>
#include <thread>
#include <QtCore>
#include <unistd.h>
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/log/utility/setup/file.hpp>
#include "ctpapi_linux64/ThostFtdcMdApi.h"
#include "ctpapi_linux64/ThostFtdcTraderApi.h"
#include "MarketDataHandle.h"
#include "TradingHandle.h"
#include "initialize.h"

double lastorderprice = 0;
int account_position_query_counter = 1;
using namespace std;
namespace  logging = boost::log;
//CThostFtdcTraderApi* pTraderApi;

void split(const string &s, char delim, vector<string> &elems);
vector<string> split(const string &s, char delim);

void split(const string &s, char delim, vector<string> &elems){
    stringstream ss;
    ss.str(s);
    string item;
    while(getline(ss, item, delim)){
        elems.push_back(item);
    }
}

vector<string> split(const string &s, char delim){
    vector<string> elems;
    split(s, delim, elems);
    return elems;
}

void quoteThread(CThostFtdcTraderApi* ptraderapi, char* FRONT_ADDR_quote, TThostFtdcBrokerIDType brokerid, TThostFtdcInvestorIDType investorid,
                 TThostFtdcPasswordType password, DBDriver* dbdriver, vector<string> ppinsturment, int instrument )
{
    CThostFtdcMdApi* pUserApi = CThostFtdcMdApi::CreateFtdcMdApi();
    CThostFtdcMdSpi *pMarketDataHandle = new MarketDataHandle(pUserApi, ptraderapi, FRONT_ADDR_quote, brokerid, investorid,
                                                              password, dbdriver, ppinsturment, instrument);
    pUserApi->RegisterSpi(pMarketDataHandle);
    pUserApi->RegisterFront(FRONT_ADDR_quote);
    pUserApi->Init();
    pUserApi->Join();
}

void tradeThread(TradingHandle *pTradingHandle,CThostFtdcTraderApi  *pTraderApi, char *FRONT_ADDR_trade)
{
    pTraderApi->RegisterSpi((CThostFtdcTraderSpi*) pTradingHandle);
//    pTraderApi->SubscribePublicTopic(THOST_TERT_QUICK);
//    pTraderApi->SubscribePrivateTopic(THOST_TERT_QUICK);
    pTraderApi->SubscribePublicTopic(THOST_TERT_RESTART);
    pTraderApi->SubscribePrivateTopic(THOST_TERT_RESTART);
    pTraderApi->RegisterFront(FRONT_ADDR_trade);
    pTraderApi->Init();
}

void queryT(TThostFtdcBrokerIDType brokerIDType, TThostFtdcInvestorIDType investorIDType, CThostFtdcTraderApi* pTraderApi)
{
    while(true) {
        cout << "account_position_query_counter = "<< account_position_query_counter++ << endl;
        CThostFtdcQryInvestorPositionField req;
        memset(&req, 0, sizeof(req));
        strcpy(req.BrokerID, brokerIDType);
        strcpy(req.InvestorID, investorIDType);
        int iRequestID_trade = 1;
        pTraderApi->ReqQryInvestorPosition(&req, ++iRequestID_trade);
        //sleep for 10 seconds
        std::this_thread::sleep_for(std::chrono::milliseconds(10000));
    }

}

int main() {
    cout << "new added part" << endl;
    iniFrontAdress();
    iniDB();

    cout << "starting" <<endl;
    //�����ļ�����defaultֵ����ֹexception
    //read config
    boost::property_tree::ptree pt;
    boost::property_tree::ini_parser::read_ini("./ini/CTPClientDemo.ini", pt);
    string iniflag = pt.get<std::string>("Flag.iniflag");
    cout << "which ini is being loaded " << iniflag << endl;
    //server
    string MF= pt.get<std::string>("Server_IP.MarketFront");
    string TF = pt.get<std::string>("Server_IP.TradeFront");
    const int chararraylength = 50;
    char FRONT_ADDR_quote[chararraylength];
    char FRONT_ADDR_trade[chararraylength];
    strcpy(FRONT_ADDR_quote, MF.c_str());
    strcpy(FRONT_ADDR_trade, TF.c_str());
    //account
    TThostFtdcBrokerIDType brokerIDType;
    strcpy(brokerIDType, pt.get<std::string>("Account.BrokerID").c_str());
    TThostFtdcInvestorIDType investorIDType;
    strcpy(investorIDType, pt.get<std::string>("Account.InvestorID").c_str());
    TThostFtdcPasswordType passwordType;
    strcpy(passwordType, pt.get<std::string>("Account.Password").c_str());
    //database
    string Server = pt.get<std::string>("Database.Server");
    string User = pt.get<std::string>("Database.User");
    string Password = pt.get<std::string>("Database.Password");
    string Scheme = pt.get<std::string>("Database.Scheme");
    DBDriver dbDriver(Server, User, Password, Scheme);
    //market

    string SubscribeSymbolList = pt.get<std::string>("MarketData.SubscribeSymbolList");
    vector<string> ppIntrumentID(split(SubscribeSymbolList, ','));
    int iInstrumentID = ppIntrumentID.size();
    logging::add_file_log(pt.get<std::string>("CTPClientDemo.LogPath"));
    //trading
    string tradeinstrument = pt.get<std::string>("Trading.tradeinstrument");
    char tinstrumemt[chararraylength];
    strcpy(tinstrumemt, tradeinstrument.c_str());
    int quantity = pt.get<int>("Trading.quantity");
    double price = pt.get<double>("Trading.price");
    string strdirection = pt.get<std::string>("Trading.direction");
    char direction = strdirection.at(0);
//���������

    CThostFtdcTraderApi* pTraderApi = CThostFtdcTraderApi::CreateFtdcTraderApi();

    BOOST_LOG_TRIVIAL(info)<<"quote thread started ...";
    std::thread QuoteT(quoteThread, pTraderApi, FRONT_ADDR_quote, brokerIDType, investorIDType, passwordType,
                       &dbDriver, ppIntrumentID,iInstrumentID);
    QuoteT.detach();
    TradingHandle *pTradingHandle = new TradingHandle(pTraderApi,FRONT_ADDR_trade, brokerIDType, investorIDType, passwordType, &dbDriver,
                                                      tinstrumemt, price, quantity,  direction);
    pTraderApi->RegisterSpi((CThostFtdcTraderSpi*) pTradingHandle);
    pTraderApi->SubscribePublicTopic(THOST_TERT_QUICK);
    pTraderApi->SubscribePrivateTopic(THOST_TERT_QUICK);
    pTraderApi->RegisterFront(FRONT_ADDR_trade);
    pTraderApi->Init();
    this_thread::sleep_for(chrono::seconds(2));
    BOOST_LOG_TRIVIAL(info)<<"spi thread started ...";

    //query investor postion /account postion at a fixed interval and save to db
    //using detached thread to achieve it
    std::thread QueryPostionT(queryT, brokerIDType, investorIDType, pTraderApi);
    QueryPostionT.join();



//
//    //std::thread TradingT(tradeThread, pTradingHandle,  FRONT_ADDR_trade);
////    CThostFtdcTraderApi* orderTradeapi = CThostFtdcTraderApi::CreateFtdcTraderApi();
////    orderTradeapi->RegisterS;pi((CThostFtdcTraderSpi*) pTradingHandle);
////    orderTradeapi->SubscribePublicTopic(THOST_TERT_QUICK);
////    orderTradeapi->SubscribePrivateTopic(THOST_TERT_QUICK);
////    orderTradeapi->RegisterFront(FRONT_ADDR_trade);
//
//    CThostFtdcInputOrderField req;
//    memset(&req, 0, sizeof(req));
//    ///���͹�˾����
//    strcpy(req.BrokerID, brokerIDType);
//    ///Ͷ���ߴ���
//    strcpy(req.InvestorID, investorIDType);
//    ///��Լ����
//    strcpy(req.InstrumentID, tinstrumemt);
//    ///��������
//    //orderref
//    strcpy(req.OrderRef, new char[3]);
//    ///�û�����
//    //	TThostFtdcUserIDType	UserID;
//    ///�����۸�����: �޼�
//    req.OrderPriceType = THOST_FTDC_OPT_LimitPrice;
//    ///��������:
//    req.Direction = direction;
//    ///��Ͽ�ƽ��־: ����
//    req.CombOffsetFlag[0] = THOST_FTDC_OF_Open;
//    ///���Ͷ���ױ���־
//    req.CombHedgeFlag[0] = THOST_FTDC_HF_Speculation;
//    ///�۸�
//    //����22900����
//    req.LimitPrice = 2900;
//    ///����: 1
//    req.VolumeTotalOriginal = quantity;
//    ///��Ч������: ������Ч
//    req.TimeCondition = THOST_FTDC_TC_GFD;
//    ///GTD����
//    //	TThostFtdcDateType	GTDDate;
//    ///�ɽ�������: �κ�����
//    req.VolumeCondition = THOST_FTDC_VC_AV;
//    ///��С�ɽ���: 1
//    req.MinVolume = 1;
//    ///��������: ����
//    req.ContingentCondition = THOST_FTDC_CC_Immediately;
//    ///ֹ���
//    //	TThostFtdcPriceType	StopPrice;
//    ///ǿƽԭ��: ��ǿƽ
//    req.ForceCloseReason = THOST_FTDC_FCC_NotForceClose;
//    ///�Զ������־: ��
//    req.IsAutoSuspend = 0;
//    ///ҵ��Ԫ
//    //	TThostFtdcBusinessUnitType	BusinessUnit;
//    ///������
//    //	TThostFtdcRequestIDType	RequestID;
//    ///�û�ǿ����־: ��
//    req.UserForceClose = 0;
//    int iRequestID_trade = 2;
//    int iResult = pTraderApi->ReqOrderInsert(&req, ++iRequestID_trade);

//    //pTraderApi->Join();
////    TradingT.join();
//
//    //cerr << "trade---->>> ����¼������: " << iResult << ((iResult == 0) ? ", �ɹ�" : ", ʧ��") << endl;
////    pTraderApi->Init();
//    //pTraderApi->Join();
//    //orderTradeapi->Release();
//

    getchar();
    return 0;
}