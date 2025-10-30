jtagconfig --setparam 1 JtagClock 6M
quartus_hps -c 1 -o P -a 0x00080000 BIF.bin


#if (_DEBUG_SIDE_ENABLE==1)
				_main_send_msg(ID_SIDE_MBX, TMSG_SIDE_STANDBY_REQ, 0, 0, 0, 0);
#endif

#if (_DEBUG_SIDE_ENABLE==1)
				_main_send_msg(ID_SIDE_MBX, TMSG_SIDE_ACTIVE_REQ, 0, 0, 0, 0);
#endif

#if (_DEBUG_SIDE_ENABLE==1)
				_main_send_msg(ID_SIDE_MBX, TMSG_SIDE_STR_CHECK_REQ, 0, 0, 0, 0);
#endif


			// イベント通知し、搬送停止
			if(ex_side_ad > ex_side_idle_ad + SIDE_AD_THD_VALUE)
			{
				iset_flg(ID_FEED_CTRL_FLAG, EVT_FEED_STRING_UP);
			}
			else if(ex_side_ad < ex_side_idle_ad - + SIDE_AD_THD_VALUE)
			{
				iset_flg(ID_FEED_CTRL_FLAG, EVT_FEED_STRING_DOWN);
			}
  {                                                         0, 0x0501}	/* [Sequence:0x0500] wait motor start enable */
 ,{                                                         0, 0x0502}	/* [Sequence:0x0501] feed APB-IN-sensor OFF position */
 ,{                                                         0, 0x0503}	/* [Sequence:0x0502] feed APB-OUT-sensor OFF position */
 ,{                                                         0, 0x0504}	/* [Sequence:0x0503] feed EXIT-sensor OFF position */
 ,{                                                         0, 0x0505}	/* [Sequence:0x0504] feed stack position */
 ,{                                                         0,      0}	/* [Sequence:0x0505] stop motor */
#define DIST_ESP_TO_PBI		(DIST_ENT_TO_PBI-DIST_ENT_TO_ESP)			/* distance of escrow-position to APB-IN-sensor */
#define DIST_PBI_TO_PBO		(DIST_ENT_TO_PBO-DIST_ENT_TO_PBI)			/* distance of APB-IN-sensor to APB-OUT-sensor */
