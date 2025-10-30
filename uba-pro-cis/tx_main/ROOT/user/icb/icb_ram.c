/*
 * icb.h
 *
 *  Created on: 2018/02/27
 *      Author: suzuki-hiroyuki
 */

#ifndef SRC_MAIN_INCLUDE_ICB_RAM_H_
#define SRC_MAIN_INCLUDE_ICB_RAM_H_

#if defined(EXT)
	#define EXTERN extern
#else
	#undef EXTERN
	#define EXTERN
#endif

/*----------------------------------------------------------*
*	write interval the time data to ICB		'14-07-24		*
*----------------------------------------------------------*/
#define	ICB_WRITE_1ST_INTERVAL_TIME_DATA	(1 * 10)			/*	1 sec(100ms timer)	*/
#define	ICB_WRITE_INTERVAL_TIME_DATA		(30 * 60 * 10)		/*	30 min(100ms timer)	*/
//debug #define	ICB_WRITE_INTERVAL_TIME_DATA	(30 * 10)		/*	30 min(100ms timer)	*/
/*--------------------------------------------------*/
/*		ICB内容保存									*/
/*--------------------------------------------------*/
#define ICB_DATA1_OFFSET  0
#define ICB_DATA2_OFFSET  sizeof(struct _Smrtdat)


#if 1//#if (_RFID_BACK_DATA28==1) //(_DEBUG_NRWA5_2K_TAG_COMMUNICATION_TEST_ENABLE==1) //2023-04-18
	#define	BLK_DENOMI1_NUMBER	  	1		/*	 1:Denomi1-2						*/
//	#define	BLK_DENOMI3_NUMBER	  	2		/*	 2:Denomi3-4 						*/
//	#define	BLK_DENOMI5_NUMBER	  	3		/*	 3:Denomi5-6						*/
//	#define	BLK_DENOMI7_NUMBER	  	4		/*	 4:Denomi7-8						*/
//	#define	BLK_DENOMI9_NUMBER	  	5		/*	 5:Denomi9-10						*/
//	#define	BLK_DENOMI11_NUMBER	  	6		/*	 6:Denomi11-12						*/
//	#define	BLK_DENOMI13_NUMBER	  	7		/*	 7:Denomi13-14						*/
//	#define	BLK_DENOMI15_NUMBER	  	8		/*	 8:Denomi15-16						*/
//	#define	BLK_DENOMI17_NUMBER	  	9		/*	 9:(Denomi17-18)					*/
//	#define	BLK_DENOMI19_NUMBER	 	10		/*	10:(Denomi19-20)					*/
	#define	BLK_INSERT_BILL_NUMBER	6//11	/*	11:Insert bill info					*/
	#define	BLK_LAST_TICKET_NUMBER	7//12	/*	12:Last Ticket number				*/
	#define	BLK_ERR_NUMBER1	 	7			/*	13:error counter1,2					*/
	#define	BLK_ERR_NUMBER2	 	8			/*	13:error counter1,2					*/
//	#define	BLK_ERR12_NUMBER	 	13		/*	13:error counter1,2					*/
//	#define	BLK_ERR3_NUMBER	 		14		/*	14:error counter3					*/
//	#define	BLK_ERR4_NUMBER	 		15		/*	15:(error counter4)					*/
//	#define	BLK_ERR5_NUMBER	 		16		/*	16:(error counter5)					*/
	#define	BLK_MC_NUMBER	 		9//17	/*	17:M/C number						*/
//	#define	BLK_BOX_NUMBER	 		18		/*	18:box number						*/
	#define	BLK_VER_NUMBER	 		11//19	/*	19:software version					*/
//	#define	BLK_RW_VER_NUMBER	 	20		/*	20:RFID-R/W firmware version		*/
	#define	BLK_REM_TIME_NUMBER	 	11//21	/*	21:Box remove time					*/
	#define	BLK_SET_TIME_NUMBER	 	12//22	/*	22:Box setting	time				*/
//	#define	BLK_INI_TIME_NUMBER		23		/*	23:initial	time					*/
	#define	BLK_SUM_NUMBER			13//24	/*	24:flag, currency assign, id, sum	*/
	#define	BLK_REJECT_NUMBER		13//25	/*	25:reject counter					*/
	#define	BLK_CRENCY_NUMBER1		13//26	/*	26:currency assign					*/
	#define	BLK_CRENCY_NUMBER2		14//26	/*	26:currency assign					*/
	#define	BLK_TICKET_REJ_NUMBER1	15//27	/*	27:reject counter for Ticket		*/
	#define	BLK_TICKET_REJ_NUMBER2	16//27	/*	27:reject counter for Ticket		*/
	#define	BLK_MODEL_NUMBER		16//28	/*	28:Model 							*/
	#define	BLK_SERIAL_NUMBER		17//29	/*	29:Serial number					*/
	#define	BLK_SUM2_NUMBER			17//30	/*	30:reserved(3) & sum				*/
	#define	BLK_DUMMY1_NUMBER		17//30	/*	30:									*/
#else

	#define	BLK_DENOMI1_NUMBER	  	1		/*	 1:Denomi1-2						*/
	#define	BLK_DENOMI3_NUMBER	  	2		/*	 2:Denomi3-4 						*/
	#define	BLK_DENOMI5_NUMBER	  	3		/*	 3:Denomi5-6						*/
	#define	BLK_DENOMI7_NUMBER	  	4		/*	 4:Denomi7-8						*/
	#define	BLK_DENOMI9_NUMBER	  	5		/*	 5:Denomi9-10						*/
	#define	BLK_DENOMI11_NUMBER	  	6		/*	 6:Denomi11-12						*/
	#define	BLK_DENOMI13_NUMBER	  	7		/*	 7:Denomi13-14						*/
	#define	BLK_DENOMI15_NUMBER	  	8		/*	 8:Denomi15-16						*/
	#define	BLK_DENOMI17_NUMBER	  	9		/*	 9:(Denomi17-18)					*/
	#define	BLK_DENOMI19_NUMBER	 	10		/*	10:(Denomi19-20)					*/
	#define	BLK_INSERT_BILL_NUMBER	11		/*	11:Insert bill info					*/
	#define	BLK_LAST_TICKET_NUMBER	12		/*	12:Last Ticket number				*/
	#define	BLK_ERR12_NUMBER	 	13		/*	13:error counter1,2					*/
	#define	BLK_ERR3_NUMBER	 		14		/*	14:error counter3					*/
	#define	BLK_ERR4_NUMBER	 		15		/*	15:(error counter4)					*/
	#define	BLK_ERR5_NUMBER	 		16		/*	16:(error counter5)					*/
	#define	BLK_MC_NUMBER	 		17		/*	17:M/C number						*/
	#define	BLK_BOX_NUMBER	 		18		/*	18:box number						*/
	#define	BLK_VER_NUMBER	 		19		/*	19:software version					*/
	#define	BLK_RW_VER_NUMBER	 	20		/*	20:RFID-R/W firmware version		*/
	#define	BLK_REM_TIME_NUMBER	 	21		/*	21:Box remove time					*/
	#define	BLK_SET_TIME_NUMBER	 	22		/*	22:Box setting	time				*/
	#define	BLK_INI_TIME_NUMBER		23		/*	23:initial	time					*/
	#define	BLK_SUM_NUMBER			24		/*	24:flag, currency assign, id, sum	*/
	#define	BLK_REJECT_NUMBER		25		/*	25:reject counter					*/
	#define	BLK_CRENCY_NUMBER		26		/*	26:currency assign					*/
	#define	BLK_TICKET_REJ_NUMBER	27		/*	27:reject counter for Ticket		*/
	#define	BLK_MODEL_NUMBER		28		/*	28:Model 							*/
	#define	BLK_SERIAL_NUMBER		29		/*	29:Serial number					*/
	#define	BLK_SUM2_NUMBER			30		/*	30:reserved(3) & sum				*/
	#define	BLK_DUMMY1_NUMBER		30		/*	30:									*/

#endif



enum ICB_INITIAL_DATA_TYPE
{
	ICB_SUB_INITIAL_WRITE_MCNo = 1,
	ICB_SUB_INITIAL_WRITE_VER,
#if 1//#if (_RFID_BACK_DATA28==1) //(_DEBUG_NRWA5_2K_TAG_COMMUNICATION_TEST_ENABLE==1) //2023-04-18
	ICB_SUB_INITIAL_WRITE_CRENCY1,
	ICB_SUB_INITIAL_WRITE_CRENCY2,
#else
	ICB_SUB_INITIAL_WRITE_CRENCY,
#endif
	ICB_SUB_INITIAL_WRITE_MODEL,	/*	model					*/
	ICB_SUB_INITIAL_WRITE_SERIAL,	/*	model					*/

/* Write flag data last */
	ICB_SUB_INITIAL_WRITE_INFO,		/*	flg, assign, id, sum	*/

	ICB_SUB_INITIAL_WRITE_END,
};/*<	ICB_WRITE_INITIAL_DATA	(N100-180-01 '12-07-01)	>*/


/*--------------------------------------------------*/
#if !defined(UBA_RTQ)
	EXTERN u8 ex_icb_state;
	#define	BIT_ICB_DATA_SUM_ERROR			0x02		/*	ICB	Data error			*/
	#define	BIT_ICB_DATA_ALL_CLEAR			0x08		/*	Initialize all data when the flag of 4	*/
	#define	BIT_ICB_DATA_ALL_CLEAR_NEXT		0x04		/*	Initialize all data when the flag of 4	*/
	/* ｲﾆｼｬﾙﾃﾞｰﾀ書き込み中のHard Reset対策	 */
	#define	BIT_INITIAL_DATA_WRITING		0x01		/*	イニシャルデータ書込み動作中フラグ	*/

	EXTERN u16	ex_Wcnt;
	EXTERN u16	ex_BAR_length[2];
	EXTERN u8	ICBBarcode[256];		//注意 BARCODE_LENGTH	/*	BARCODE ｷｬﾗｸﾀｺｰﾄﾞに変換	*/
#endif
/*--------------------------------------------------*/

#define	ICB_FORMAT_REVISION			1		/*	Rev1: Add Ticket Reject counter, Model, Serial#	*/
//2020-05-05 Added SS / SU installation information to ICB memory
#define	ICB_STYLE_BIT    			0xc0	    /*	??xxxxxxB bit Info	*/
#define	ICB_SS_STYLE    			(0x01 << 6)	/*	01xxxxxxB SS Setttng	*/
#define	ICB_SU_STYLE    			(0x02 << 6)	/*	10xxxxxxB SU Setttng	*/


#endif /* SRC_MAIN_INCLUDE_ICB_H_ */
