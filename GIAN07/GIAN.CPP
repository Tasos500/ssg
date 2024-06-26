/*                                                                           */
/*   GIAN.cpp   ゲーム全体の管理                                             */
/*                                                                           */
/*                                                                           */

#include <windows.h>
#include "GIAN.H"
#include "LEVEL.H"
#include "CONFIG.H"


///// [グローバル変数] /////
//HIGH_SCORE		*HighScore;
//char			ScoreTable[8][80];
uint32_t	GameCount;
uint8_t	GameStage;



///// [ 関数(非公開) ] /////
#ifdef PBG_DEBUG
// デバッグ情報を出力する //
extern void OutputDebugMsg(void)
{
	static DWORD	fps,count;
	static DWORD	time,temp;
	//extern InputConfig			IConfig;
	const char	*DItem[4] = {"Easy","Norm","Hard","Luna"};
	char	buf[100];

	if(!DebugDat.MsgDisplay) return;

	temp = timeGetTime();
	if(temp-time<=1000) count++;
	else{
		fps   = count;
		count = 0;
		time  = temp;
	}

	sprintf(buf,"%03d Fps",fps);
	GrpPut16(0,0,buf);

	sprintf(buf, "%2dBppMode", ConfigDat.BitDepth.v.value());
	GrpPut16(0,32,buf);
	//sprintf(buf,"%s",DItem[ConfigDat.GameLevel.v]);
	//GrpPut16(0,50,buf);

	sprintf(buf,"%s",DItem[PlayRank.GameLevel]);
	GrpPut16(0,50,buf);
	sprintf(buf,"%s<Cfg>",DItem[ConfigDat.GameLevel.v]);
	GrpPut16(0,68,buf);
	sprintf(buf,"Pr %d",PlayRank.Rank);
	GrpPut16(0,86,buf);


	sprintf(buf,"Enemy %3d",EnemyNow);
	GrpPut16(0,96+40,buf);

	sprintf(buf,"Tama1 %3d",Tama1Now);
	GrpPut16(0,128+40,buf);
	sprintf(buf,"Tama2 %3d",Tama2Now);
	GrpPut16(0,148+40,buf);
	sprintf(buf,"Laser %3d",LaserNow);
	GrpPut16(0,176+40,buf);
	sprintf(buf,"HLaser %2d",HLaserNow);
	GrpPut16(0,196+40,buf);

	sprintf(buf,"MTama %3d",MaidTamaNow);
	GrpPut16(0,224+40,buf);

	sprintf(buf,"Item  %3d",ItemNow);
	GrpPut16(0,252+40,buf);

	sprintf(buf,"Pow   %3d",Viv.exp);
	GrpPut16(0,290+40,buf);

	sprintf(buf,"SSPD  %3d",ScrollInfo.ScrollSpeed);
	GrpPut16(0,320+40,buf);

	GrpPut16(0,440,"Gian07");
	GrpPut16(0,460,"DebugMode");

	_strdate(buf);
	GrpPut16(640-128,0,"Date");
	GrpPut16(640-128,20,buf);

	_strtime(buf);
	GrpPut16(640-128,50,"Time");
	GrpPut16(640-128,70,buf);

	GrpPut16(640-128,100,"SCL Count");
	sprintf(buf," %5d",GameCount);
	GrpPut16(640-128,120,buf);

	sprintf(buf,"Left   %d",Viv.left);
	GrpPut16(640-128,440,buf);
	sprintf(buf,"Credit %d",Viv.credit);		// -1 に注意だ！！
	GrpPut16(640-128,460,buf);
}
#endif

extern void StdStatusOutput(void)
{
	static DWORD	fps,count;
	static DWORD	time,temp;
	//extern InputConfig			IConfig;
	const char	*DItem[4] = {"Easy","Norm","Hard","Luna"};
	char	buf[100];

//	if(!DebugDat.MsgDisplay) return;

	temp = timeGetTime();
	if(temp-time<=1000) count++;
	else{
		fps   = count;
		count = 0;
		time  = temp;
	}
/*
	sprintf(buf,"%03d Fps",fps);
	GrpPut16(0,0,buf);

	sprintf(buf, "%2dBppMode", ConfigDat.BitDepth.v);
	GrpPut16(0,32,buf);

	sprintf(buf, "%s<Cfg>", DItem[ConfigDat.GameLevel.v]);
	GrpPut16(0,68,buf);
*/
//	GrpPut16(0,440,"G07");
//	GrpPut16(0,460,"12/5 Ver");

	_strdate(buf);
	GrpPut16(640-128,0,"Date");
	GrpPut16(640-128,20,buf);

	_strtime(buf);
	GrpPut16(640-128,50,"Time");
	GrpPut16(640-128,70,buf);

	sprintf(buf,"Bomb   %d",Viv.bomb);
	GrpPut16(640-128,400,buf);

	sprintf(buf,"Left   %d",Viv.left);
	GrpPut16(640-128,440,buf);
	sprintf(buf,"Credit %d",Viv.credit);		// -1 に注意だ！！
	GrpPut16(640-128,460,buf);
}
