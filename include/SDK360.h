#pragma once
#include "windows.h"
#pragma pack(1)
// SDK初始化所需环境信息
struct EnvInfo
{
	DWORD dwSize;                         // EnvInfo结构体大小
	WCHAR wszAppId[128];     // appid ,厂商提供
	ULONGLONG u64Qid;                     // qid，厂商提供
};

// 创建支付订单需要厂商提供的相关信息
struct OderRequest
{
	DWORD dwSize;                          // 订单请求结构体大小
	WCHAR wszOrderId[40];                  // 厂商的订单id，厂商需要保证在同一应用下订单id的唯一性
	DWORD dwAmount;                        // 订单金额，单位“分”
	__time64_t tOrderCreateTime;           // 厂商创建该订单的时间戳，后期对账使用
	WCHAR wszUserId[40];                   // 用户id，厂商的用户唯一标识，后期对账使用
	WCHAR wszProductId[40];                // 商品id，商品在厂商方的标识，后期对账使用
	WCHAR wszProductDescription[400];      // 商品描述，后期对账使用

	WCHAR wszAttach[128];                   //* 保留字段
	DWORD dwDeductionType;                  //* 保留字段
	DWORD dwDeductionAmount;                //* 保留字段
	DWORD dwDeductionPeriod;                //* 保留字段
	__time64_t tFirstDeductionTime;         //* 保留字段
	WCHAR wszContractNotify[1024];          //* 保留字段
};

// 创建订单返回的结果。
struct OderResponse
{
	DWORD dwSize;                          // 订单返回结构体大小
	DWORD dwTicket;                        // LYSDK_Pay或LYSDK_AsyncPay接口返回的值
	DWORD nErrno;                          // 订单的生成状态，0:生成订单正确: 非0，生成订单错误,  todo，细化可能的错误码
	WCHAR wszMsg[128];                       // 订单生成状态的详细描述
	WCHAR wszQrCode[2048];                 // 若订单生成成功，该变量保存订单支付链接，用于生成二维码
	DWORD dwAmount;                        // 若订单生成成功，该变量保存订单金额，与请求金额一致
	__time64_t tExpireTime;                // 若订单生成成功，该变量保存订单失效的时间戳
	DWORD dwLive;                          // 若订单生成成功，该变量保存订单有效时长，单位“秒”。若担心客户端本地时间不准确，可使用该字段记录二维码有效期
};

struct OderResponse2 : public OderResponse
{
	WCHAR wszTraceId[33];                  // 若订单生成成功，该变量用来追踪服务端的订单链路
};

#pragma pack()

/*
** 函数功能：订单支付状通知
** dwTicket[in]:  LYSDK_Pay或LYSDK_AsyncPay接口返回的值
** iOderStatus[in]:  订单状态, 1-已支付,2-支付失败,6-二维码失效
** iPayChanel [in]:  支付渠道，1-微信，2-支付宝
*/
typedef void(__stdcall *SDK360_PAYSTATUS_CALLBACK)(DWORD dwTicket, int iOderStatus, int iPayChanel);

/*
** 函数功能：创建订单的结果通知函数
** dwTicket[in]:  LYSDK_Pay或LYSDK_AsyncPay接口返回的值
** fpOderResponse[in]:  创建订单的详细信息，详见OderResponse定义
*/
typedef void(__stdcall *SDK360_ORDERRESULT_CALLBACK)(DWORD dwTicket, const OderResponse& fpOderResponse);

/*
** 函数功能：SDK的初始化接口，调用支付接口之前必须先初始化
** pEnvInfo[in]:     厂商的相关信息，详见EnvInfo结构体定义
** @return [out]:    0:初始化成功;其它值:初始化失败
*/
extern "C" int __stdcall SDK360_Init(const EnvInfo* pEnvInfo);

/*
函数功能：SDK的反初始化接口，不使用该模块时，调用该接口释放
*/
extern "C" int __stdcall SDK360_UnInit();

/*
** 函数功能：同步阻塞方式创建订单接口
** fpOrderRequest[in]:       厂商提供的创建订单所需相关信息，详见OderRequest定义
** fnPayStatusCallBack[in]:  订单创建成功后，支付状态变更的通知回调接口
** fpOrderResponse[out]:     返回的创建订单的详细信息，详见OderResponse定义
** @return [out]:            当前订单的dwTicket值，作为当前订单的唯一id，在OderResponse和回调函数中作为当前支付的关联值
*/
extern "C" DWORD __stdcall SDK360_Pay(const OderRequest& fpOrderRequest, OderResponse& fpOrderResponse, SDK360_PAYSTATUS_CALLBACK fnPayStatusCallBack);

/*
** 函数功能：异步非阻塞方式创建订单接口
** fpOrderRequest[in]:         厂商提供的创建订单所需相关信息，详见OderRequest定义
** fnOrderResultCallback[in]:  订单创建完成的接口回调函数
** fnPayStatusCallBack[in]:    订单创建成功后，支付状态变更的通知回调接口
** @return [out]:          当前订单的dwTicket值，作为当前订单的唯一id，在OderResponse和回调函数中作为当前支付的关联值
*/
extern "C" DWORD __stdcall SDK360_AsyncPay(const OderRequest& fpOrderRequest, SDK360_ORDERRESULT_CALLBACK fnOrderResultCallback, SDK360_PAYSTATUS_CALLBACK fnPayStatusCallback);

/*
** 函数功能：设置网络请求的代理模式
** bFollowSystem[in]:         是否跟随系统代理配置， TRUE:跟随; FALSE:手动配置代理
** lpcwProxy[in]:             代理服务的详细配置，[http://]ip:port;[[https://]ip:port]格式。当bFollowSystem为TRUE时，忽略此参数。
** @return [out]:             TRUE: 设置成功; FALSE: 设置失败
*/
extern "C" BOOL __stdcall SDK360_SetProxy(BOOL bFollowSystem, LPCWSTR lpcwProxy);

/*
** 函数功能：取消订单，sdk在等待用户支付时，底层会做轮询。取消订单可节省系统资源占用
** dwTicket[in]:  LYSDK_Pay或LYSDK_AsyncPay接口返回的值
** @return [out]:           0:取消成功;其它值:取消失败
*/
extern "C" int __stdcall SDK360_CancelPay(DWORD dwTicket);

typedef int(__stdcall *FUN_SDK360_Init)(const EnvInfo* pEnvInfo);

typedef int(__stdcall *FUN_SDK360_UnInit)();

typedef DWORD(__stdcall *FUN_SDK360_Pay)(const OderRequest& fpOrderRequest, OderResponse& fpOrderResponse, SDK360_PAYSTATUS_CALLBACK fnPayStatusCallBack);

typedef DWORD(__stdcall *FUN_SDK360_AsyncPay)(const OderRequest& fpOrderRequest, SDK360_ORDERRESULT_CALLBACK fnOrderResultCallback, SDK360_PAYSTATUS_CALLBACK fnPayStatusCallback);

typedef int(__stdcall *FUN_SDK360_CancelPay)(DWORD dwTicket);
