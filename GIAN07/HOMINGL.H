/*                                                                           */
/*   HomingL.h   長いレーザーの処理                                          */
/*                                                                           */
/*                                                                           */


#ifndef PBGWIN_HOMINGL_H
#define PBGWIN_HOMINGL_H		"HOMINGL : Ver 0.01 : Updata 2000/09/04"
//#pragma message(PBGWIN_HOMINGL_H)

#include "EXDEF.H"



///// [ 定数 ] /////
#define HLASER_MAX			162
#define HLASER_LEN			7		// 描画枚数..
#define HLASER_SECTION		4		// 読み込み幅


#define HL_NONE		0			// ただ進むだけ
#define HL_TYPE1	1			// その１

#define HLS_NORM	0x00		// ホーミングレーザー通常
#define HLS_CLEAR	0x01		// ホーミングレーザー消去中
#define HLS_DEAD	0xff		// ホーミングレーザー削除要請



///// [構造体] /////

// ホーミングレーザー //
typedef struct tagHLaserData {
	int		Current;	// 現在の先頭
	int		v;			// 速度
	int		a;			// 加速度

	uint32_t	Count;	// フレームカウンタ

	uint8_t	Type;	// 種類(加速＆ホーミングタイプ)
	uint8_t	State;	// 状態
	uint8_t	c;	// 色
	uint8_t	Left;	// 残りホーミング回数

	struct tagHLaserData	*Next;				// 次のレーザーへのポインタ
	DegPoint	p[HLASER_LEN*HLASER_SECTION];	// 頂点キュー(ExDef.h)
} HLaserData;

// ホーミングレーザーセット情報 //
typedef struct tagHLaserInfo {
	int		x,y;		// 中心座標

	uint8_t	d;	// 角度
	uint8_t	dw;	// 角度の開き
	uint8_t	n;	// 本数

	uint8_t	c;	// 色
	uint8_t	type;	// 種類
} HLaserInfo;



///// [グローバル変数] /////
extern uint16_t	HLaserNow;	// ホーミングレーザーの本数
extern HLaserInfo	HLaserCmd;			// ホーミングレーザーセット用データ


///// [関数プロトタイプ] /////
void HLaserInit(void);	// ホーミングレーザーの初期化を行う
void HLaserSet(HLaserInfo *hinfo);	// ホーミングレーザーをセットする
void HLaserMove(void);	// ホーミングレーザーを動作させる
void HLaserDraw(void);	// ホーミングレーザーを描画する
void HLaserClear(void);	// ホーミングレーザーに消去エフェクトをセット



#endif
