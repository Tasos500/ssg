/*                                                                           */
/*   Maid.h   メイドさん関連の処理                                           */
/*                                                                           */
/*                                                                           */

#ifndef PBGWIN_MAID_H
#define PBGWIN_MAID_H		"めいど : Version 0.01 : Update 2000/02/17"
//#pragma message(PBGWIN_MAID_H)

#include <stdint.h>



///// [ 定数 ] /////

// サボテン定数 //
#define VIVDEAD_VAL		(300)		// びびっとの死亡時間．．．
#define VIVMUTEKI_VAL	(180)			// びびっとの無敵時間
#define VIVBOMBTIMER_VAL (6)        // Frames from getting hit to dying.

#define MAID_MOVE_DISABLE_TIME		(250-100)		// 行動不能な時間

#define BOMBMUTEKI_VAL	60			// ボムの終端無敵時間
#define SBOPT_DX		26			// オプションのずれ幅(x64ではない)
//#define EVADE_X_START	150			// かすりカウンタの初期表示座標
//#define EVADE_X_SPD		5			// かすりカウンタの移動速度
//#define EVADE_TIME		60			// かすり許容フレーム数
#define EVADETIME_MAX	256			// かすりマックス時の待ち時間

#define SSP_WIDE		(64*9)		//
#define SSP_HOMING		(64*9)		//
#define SSP_LASER		(64*13)		//




///// [マクロ] /////



///// [構造体] /////

typedef struct
{
	int x,y;			// 現在の<表示>座標

	int vx,vy;			// オプションのズレ具合
	int opx,opy;		// 現在のオプション基本座標

	__int64 score;		// 得点カウンタ
	__int64	dscore;		// 得点増加値

	uint32_t	evade_sum;	// かすり合計
	int		evadesc;		// かすり得点
	uint16_t	evade;	// かすり回数
	uint16_t	evade_c;	// 連続「かすり」の残り許容時間

	char v;				// サボテンの移動速度基本値(後で64~45倍にする)

	uint8_t weapon;	// "とげ" の種類
	uint8_t exp;	// サボテンの経験値？
	uint8_t bomb;	// ボムの数
	uint8_t left;	// 残りサボテン数
	uint8_t credit;	// のこりクレジット

	uint8_t GrpID;	// 表示すべきグラフィック

	uint16_t bomb_time;	// ボムウェイト用
	uint16_t exp2;	// 経験値増加抑制用
	uint16_t muteki;	// 無敵フラグ(0:off !0:無敵時間カウンタ)
	uint16_t tamahit; // Was VIVIT hit by a bullet?
	uint16_t deathbomb;  // Frames counted until death.
	uint16_t lay_time;	// レーザーの発射タイミング用
	uint8_t lay_grp;	// レーザーのグラフィック用
	uint8_t toge_time;	// "とげ" の発射タイミング用
	uint8_t toge_ex;	// とげ発射用特殊変数
	uint8_t ShiftCounter;	// 押しっぱなし低速移動用

	bool bGameOver;	// ゲームオーバー判定用フラグ
	bool BuzzSound;	// かすった音を連続再生させないためのフラグ
} MAID;



///// [ 関数 ] /////
extern void MaidDraw(void);
extern void StateDraw(void);		// 各種ステータスを描画する
extern void MaidMove(void);
extern void MaidSet(void);			// 初期化
extern void MaidNextStage(void);	// 次のステージの準備
extern void MaidDead(void);			// 死す

extern void evade_add(uint8_t n);	// かすりゲージを上昇させる
//extern void evade_addEx(int x, int y, uint8_t n);	// かすりゲージを上昇させる

extern void score_add(int sc);		// スコアを上昇させる

extern void WideBombDraw(void);		// ワイドショット用のボム(やや例外処理)



///// [ 変数 ] /////
extern MAID			Viv;					// 麗しきメイドさん構造体
extern uint32_t	EvadeRate[256];	// かすり得点レート



inline void PowerUp(uint8_t damage)
{
	// ダメージの分だけ加算する //
	Viv.exp2 += damage;

	// Viv.exp(8bit+1bit) ooo oooooo //
	switch(((uint16_t)Viv.exp+1)>>5){
		case(0):	if(Viv.exp2>5-3)		Viv.exp++,	Viv.exp2=0;	return;
		case(1):	if(Viv.exp2>25-15)		Viv.exp++,	Viv.exp2=0;	return;
		case(2):	if(Viv.exp2>50-20)		Viv.exp++,	Viv.exp2=0;	return;
		case(3):	if(Viv.exp2>80)		Viv.exp++,	Viv.exp2=0;	return;
		case(4):	if(Viv.exp2>120)	Viv.exp++,	Viv.exp2=0;	return;
		case(5):	if(Viv.exp2>140)	Viv.exp++,	Viv.exp2=0;	return;
		case(6):	if(Viv.exp2>160)	Viv.exp++,	Viv.exp2=0;	return;
		case(7):	if(Viv.exp2>180)	Viv.exp++,	Viv.exp2=0;	return;
		case(8):	return;		// フルパワーアップ時
	}
}

uint8_t GetRightLaserDeg(uint8_t LaserDeg, int i);
uint8_t GetLeftLaserDeg(uint8_t LaserDeg, int i);

inline uint8_t GetLaserDeg(void)
{
	return ((120-Viv.bomb_time)*3)/2;
}


#endif
