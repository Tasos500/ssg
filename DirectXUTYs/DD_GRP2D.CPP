/*                                                                           */
/*   DD_GRP2D.cpp   ２Ｄ＆８Ｂｉｔ時の図形描画関数                           */
/*                                                                           */
/*                                                                           */

#include "DD_UTY.H"
#include "DD_GRP2D.H"
#include "DD_CLIP2D.H"		// 図形クリッピング用関数群
#include <ddraw.h>

#pragma message(PBGWIN_DD_GRP2D_H)

#define ATABLE_MAX	8

typedef struct tagAlphaInfo {
	DWORD	CheckValue;
	DWORD	RefCount;
} AlphaInfo;


// ２Ｄ用変数 //
BYTE				*Grp_Vram;				// これは公開しない
long				Grp_Pitch = 0;			// これも公開しない
uint32_t	Grp_AddrY[480];	// Ｙ座標テーブル
uint32_t	Col2D;	// 現在の色 x ４バイト分
BYTE				Col2D_R;				// 赤(α用)
BYTE				Col2D_G;				// 緑(α用)
BYTE				Col2D_B;				// 青(α用)
BYTE				Col2D_A;				// α値
BYTE				Col2D_Mode;				// αモード
BOOL				AlphaFlag = TRUE;		// αテーブルが更新されたかどうか
BYTE				AlphaBuf[ATABLE_MAX][256];		// αテーブル
BYTE				*AlphaTable = AlphaBuf[0];		// 現在の参照先
AlphaInfo			AInfo[ATABLE_MAX];
BYTE				ATableRef = 0;					// αテーブル格納先


// ２Ｄ用関数(補助) //
static void _2DGrpHline(int x1, int x2, int y);	// 横ラインを引く
static void _2DGrpCHline(int x1, int x2, int y);	// 横ラインを引く(ｸﾘｯﾋﾟﾝｸﾞ有り)
static void _2DGrpHlineA(int x1, int x2, int y);	// α横ラインを引く
static void _2DUpdateAlphaTable(void);	// αテーブルを更新する


// 描画系関数ポインタをセットする //
void _2DSetup(void)
{
	int i;

	// 関数ポインタをセットする //
	GrpLock     = _2DLock;
	GrpUnlock   = _2DUnlock;
	GrpSetColor = _2DSetColor;
	GrpSetAlpha = _2DSetAlpha;
	GrpLine     = _2DLine;
	GrpBox      = _2DBox;
	GrpCircle   = _2DCircle;
	GrpCircleF  = _2DCircleF;
	GrpPolygon  = _2DPolygon;
	GrpBoxA     = _2DBoxA;
	GrpCircleA  = _2DCircleA;
	GrpPolygonA = _2DPolygonA;

	// 固有の処理(この関数はSurfaceが存在するときに呼ばれるはず) //
	_2DLock();
	for(i=0;i<480;i++) Grp_AddrY[i]=i*Grp_Pitch;
	_2DUnlock();

	// テーブル初期化要求を発行 //
	DisableAlphaTable();
}

// 画面描画の準備をする //
void _2DLock(void)
{
	DDSURFACEDESC	ddsd;

	memset(&ddsd,0,sizeof(DDSURFACEDESC));
	ddsd.dwSize = sizeof(ddsd);
	DxObj.Back->Lock(NULL,&ddsd,DDLOCK_WAIT,NULL);

	Grp_Vram  = (BYTE *)ddsd.lpSurface;
	Grp_Pitch = ddsd.lPitch;
}

// 画面描画を完了する //
void _2DUnlock(void)
{
	DxObj.Back->Unlock(NULL);
	Grp_Vram  = NULL;
	Grp_Pitch = 0;		// これはいらないかな...
}

// 色セット //
void _2DSetColor(uint8_t r, uint8_t g, uint8_t b)
{
	DWORD temp;

	Col2D_R = (r>5) ? 5 : r;
	Col2D_G = (g>5) ? 5 : g;
	Col2D_B = (b>5) ? 5 : b;

	temp  = RGB256(Col2D_R,Col2D_G,Col2D_B);
	Col2D = temp + (temp<<8) + (temp<<16) + (temp<<24);

	AlphaFlag = TRUE;
}

// αセット //
void _2DSetAlpha(uint8_t a, uint8_t mode)
{
	Col2D_A    = a;
	Col2D_Mode = mode;

	AlphaFlag = TRUE;
}

// 直線 //
void _2DLine(int x1, int y1, int x2, int y2)
{
	int sx,sy,dx,dy,s,i;
	BYTE c = (BYTE)Col2D&0xff;	// ここでは１ビットずつの書き込みとなるので

	if(Grp_Vram==NULL) return;

	//if(!Grp_LClip(&x1,&y1,&x2,&y2)) return;
	if(!GrpLineClip(&x1,&y1,&x2,&y2)) return;

	if(x1>x2)	sx = -1, dx = x1-x2;
	else		sx =  1, dx = x2-x1;

	if(y1>y2)	sy = -1, dy = y1-y2;
	else		sy =  1, dy = y2-y1;

	if(dx>dy){
		s = dx>>1;
		for(i=0;i<=dx;i++){
			*(Grp_Vram+x1+Grp_AddrY[y1]) = c;
			s+=dy;
			if(s>dx){
				s-=dx;
				y1+=sy;
			}
			x1+=sx;
		}
	}
	else{
		s = dy>>1;
		for(i=0;i<=dy;i++){
			*(Grp_Vram+x1+Grp_AddrY[y1]) = c;
			s+=dx;
			if(s>dy){
				s-=dy;
				x1+=sx;
			}
			y1+=sy;
		}
	}
}

// 長方形 //
void _2DBox(int x1, int y1, int x2, int y2)
{
	int i;

	if(_2DClipRect(&x1,&y1,&x2,&y2)){
		for(i=y1;i<y2;i++)
			_2DGrpHline(x1,x2,i);
	}
}

// 円 //
void _2DCircle(int x, int y, int r)
{
	int dx,dy;
	int s;
	BYTE c = (BYTE)Col2D&0xff;

	if(Grp_Vram==NULL) return;

	if(r==0){
		_2DPset(x,y,c);
		return;
	}

	dx = s = r;
	dy = 0;

	while(dx>=dy){
		_2DPset(x+dx,y+dy,c); _2DPset(x+dx,y-dy,c);
		_2DPset(x-dx,y+dy,c); _2DPset(x-dx,y-dy,c);
		_2DPset(x+dy,y+dx,c); _2DPset(x+dy,y-dx,c);
		_2DPset(x-dy,y+dx,c); _2DPset(x-dy,y-dx,c);
		s -= (dy<<1)+1;
		if(s<0){
			s+=((dx-1)<<1);
			dx--;
		}
		dy++;
	}
}

// 円塗りつぶし //
void _2DCircleF(int x, int y, int r)
{

	int si,di;
	int s;
	//static int	pmin[480],pmax[480];
	//int			ymin,ymax;

	if(Grp_Vram==NULL) return;

	if(r==0){
		_2DPset(x,y,(BYTE)Col2D&0xff);
		return;
	}

	for(di=0,si=s=r;si>=di;di++){
		_2DGrpCHline(x-si,x+si,y-di);
		_2DGrpCHline(x-si,x+si,y+di);
		s -= ((di<<1)+1);
		if(s>=0) continue;
		_2DGrpCHline(x-di,x+di,y-si);
		_2DGrpCHline(x-di,x+di,y+si);
		s += ((--si)<<1);
	}
}

// α長方形 //
void _2DBoxA(int x1, int y1, int x2, int y2)
{
	int i;

	// 工事中につき、少々お待ち下さい... //
	if(AlphaFlag) _2DUpdateAlphaTable();

	if(_2DClipRect(&x1,&y1,&x2,&y2)){
		for(i=y1;i<y2;i++)
			_2DGrpHlineA(x1,x2,i);
	}

}

// α円塗りつぶし //
void _2DCircleA(int x, int y, int r)
{
	// 工事中につき、少々お待ち下さい... //
	if(AlphaFlag) _2DUpdateAlphaTable();
}

// αポリゴン塗りつぶし //
void _2DPolygonA(WINDOW_POINT *p, int n)
{
	// 工事中につき、少々お待ち下さい... //
	if(AlphaFlag) _2DUpdateAlphaTable();
}

// 横ラインを引く //
static void _2DGrpHline(int x1, int x2, int y)
{
	memset((Grp_Vram + Grp_AddrY[y] + x1), Col2D, ((x2 - x1) + 1));
}

// 横ラインを引く(ｸﾘｯﾋﾟﾝｸﾞ有り) //
static void _2DGrpCHline(int x1, int x2, int y)
{
	extern int CLIP_X_MIN,CLIP_X_MAX,CLIP_Y_MIN,CLIP_Y_MAX;
	int		temp;

	// 完全判定 //
	if(x1>=CLIP_X_MAX || x2<CLIP_X_MIN || y>=CLIP_Y_MAX || y<=CLIP_Y_MIN-1) return;

	// Ｘクリッピング //
	if(x1<CLIP_X_MIN) x1  = CLIP_X_MIN;
	if(x2>=CLIP_X_MAX) x2 = CLIP_X_MAX-1;

	if(x1>x2){
		temp = x1;
		x1   = x2;
		x2   = temp;
	}

	_2DGrpHline(x1,x2,y);
}

// α横ラインを引く //
static void _2DGrpHlineA(int x1, int x2, int y)
{
	union{
		DWORDLONG	dq;
		struct{
			BYTE	db0;
			BYTE	db1;
			BYTE	db2;
			BYTE	db3;
			BYTE	db4;
			BYTE	db5;
			BYTE	db6;
			BYTE	db7;
		};
	} Dat64;

	int		i,temp;
	BYTE	*write;

	if(Grp_Vram==NULL) return;
	write = Grp_Vram + x1 + Grp_AddrY[y];
	x2-=x1;
	temp = x2>>3;
	for(i=0;i<temp;i++){
		Dat64.dq  = *(DWORDLONG *)write;
		Dat64.db0 = AlphaTable[Dat64.db0];
		Dat64.db1 = AlphaTable[Dat64.db1];
		Dat64.db2 = AlphaTable[Dat64.db2];
		Dat64.db3 = AlphaTable[Dat64.db3];
		Dat64.db4 = AlphaTable[Dat64.db4];
		Dat64.db5 = AlphaTable[Dat64.db5];
		Dat64.db6 = AlphaTable[Dat64.db6];
		Dat64.db7 = AlphaTable[Dat64.db7];
		*(DWORDLONG *)write = Dat64.dq;
		write+=8;
	}

	Dat64.dq  = *(DWORDLONG *)write;
	switch(x2&7){
		case 7:		Dat64.db6 = AlphaTable[Dat64.db6];
		case 6:		Dat64.db5 = AlphaTable[Dat64.db5];
		case 5:		Dat64.db4 = AlphaTable[Dat64.db4];
		case 4:		Dat64.db3 = AlphaTable[Dat64.db3];
		case 3:		Dat64.db2 = AlphaTable[Dat64.db2];
		case 2:		Dat64.db1 = AlphaTable[Dat64.db1];
		case 1:		Dat64.db0 = AlphaTable[Dat64.db0];
					*(DWORDLONG *)write = Dat64.dq;
		case 0:		return;
	}
}

// αテーブルを更新する //
static void _2DUpdateAlphaTable(void)
{
	int		r,g,b,i,j,k,pal;
	DWORD	chk;
	BYTE	ref;

	AlphaFlag = FALSE;

	// 参照テーブルから検索を行う //
	chk = (Col2D&0xff) + ((DWORD)Col2D_A<<8) + ((DWORD)Col2D_Mode<<16);
	for(i=0; i<ATABLE_MAX; i++){
		// すでにキャッシュが効いている場合 //
		if(chk == AInfo[i].CheckValue){
			AInfo[i].RefCount++;
			AlphaTable = AlphaBuf[i];
			// DebugOut(u8"αテーブルキャッシュＯＫ");
			return;
		}
	}

	// 参考：後で参照カウントの最小値をループ内で検索し、
	//       それに対して変更を行うようにするのだ

	ref        = ATableRef;
	ATableRef  = (ATableRef+1)%ATABLE_MAX;
	AlphaTable = AlphaBuf[ref];
	AInfo[ref].CheckValue = chk;
	AInfo[ref].RefCount   = 0;

	switch(Col2D_Mode){
		case(ALPHA_ONE):
			// 固定パレットに対する処理 //
			for(r=0;r<6;r++){
				for(g=0;g<6;g++){
					for(b=0;b<6;b++){
						i = (Col2D_R + r)>5 ? 5 : Col2D_R;
						j = (Col2D_G + g)>5 ? 5 : Col2D_G;
						k = (Col2D_B + b)>5 ? 5 : Col2D_B;
						AlphaTable[RGB256(r,g,b)] = RGB256(i,j,k);
					}
				}
			}
			// 可変パレットに対する処理 //
			for(pal=0;pal<20;pal++){
				r = ((WORD)DxObj.Palette[pal].r * 5) / 255;
				g = ((WORD)DxObj.Palette[pal].g * 5) / 255;
				b = ((WORD)DxObj.Palette[pal].b * 5) / 255;
				i = (Col2D_R + r)>5 ? 5 : Col2D_R;
				j = (Col2D_G + g)>5 ? 5 : Col2D_G;
				k = (Col2D_B + b)>5 ? 5 : Col2D_B;
				AlphaTable[pal] = RGB256(i,j,k);
			}
			for(pal=236;pal<256;pal++){
				r = ((WORD)DxObj.Palette[pal].r * 5) / 255;
				g = ((WORD)DxObj.Palette[pal].g * 5) / 255;
				b = ((WORD)DxObj.Palette[pal].b * 5) / 255;
				i = (Col2D_R + r)>5 ? 5 : Col2D_R;
				j = (Col2D_G + g)>5 ? 5 : Col2D_G;
				k = (Col2D_B + b)>5 ? 5 : Col2D_B;
				AlphaTable[pal] = RGB256(i,j,k);
			}
		break;

		case(ALPHA_NORM):
			// 固定パレットに対する処理 //
			for(r=0;r<6;r++){
				for(g=0;g<6;g++){
					for(b=0;b<6;b++){
						// Result := Src*Alpha + Dest*(1-Alpha) //
						i = (Col2D_R*Col2D_A + r*(255-Col2D_A))>>8;		if(i>6) i=5;
						j = (Col2D_G*Col2D_A + g*(255-Col2D_A))>>8;		if(j>6) j=5;
						k = (Col2D_B*Col2D_A + b*(255-Col2D_A))>>8;		if(k>6) k=5;
						AlphaTable[RGB256(r,g,b)] = RGB256(i,j,k);
					}
				}
			}
			// 可変パレットに対する処理 //
			for(pal=0;pal<20;pal++){
				r = ((WORD)DxObj.Palette[pal].r * 5) / 255;
				g = ((WORD)DxObj.Palette[pal].g * 5) / 255;
				b = ((WORD)DxObj.Palette[pal].b * 5) / 255;
				i = (Col2D_R*Col2D_A + r*(255-Col2D_A))>>8;		if(i>6) i=5;
				j = (Col2D_G*Col2D_A + g*(255-Col2D_A))>>8;		if(j>6) j=5;
				k = (Col2D_B*Col2D_A + b*(255-Col2D_A))>>8;		if(k>6) k=5;
				AlphaTable[pal] = RGB256(i,j,k);
			}
			for(pal=236;pal<256;pal++){
				r = ((WORD)DxObj.Palette[pal].r * 5) / 255;
				g = ((WORD)DxObj.Palette[pal].g * 5) / 255;
				b = ((WORD)DxObj.Palette[pal].b * 5) / 255;
				i = (Col2D_R*Col2D_A + r*(255-Col2D_A))>>8;		if(i>6) i=5;
				j = (Col2D_G*Col2D_A + g*(255-Col2D_A))>>8;		if(j>6) j=5;
				k = (Col2D_B*Col2D_A + b*(255-Col2D_A))>>8;		if(k>6) k=5;
				AlphaTable[pal] = RGB256(i,j,k);
			}

		break;
	}
}

void DisableAlphaTable(void)
{
	int		i;

	for(i=0; i<ATABLE_MAX; i++){
		AInfo[i].CheckValue = 0xffffffff;	// 無効化する
	}

	ATableRef = 0;
	AlphaFlag = TRUE;
}
