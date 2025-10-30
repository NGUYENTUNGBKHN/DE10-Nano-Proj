#if defined(EXT)
	#undef EXTERN
	#define EXTERN	extern
#else
	#undef EXTERN
#endif

#if 1//
#define	NORMAL_SETTING_AUTHENTICATION_NUMBER	0x41260616		/*	機種コード + Date	*/
#define	ILLIGAL_SET_AUTHENTICATION_NUMBER		0x41260000
#else
#define	NORMAL_SETTING_AUTHENTICATION_NUMBER	0x40740616		/*	機種コード + Date	*/
#define	ILLIGAL_SET_AUTHENTICATION_NUMBER		0x40740000
#endif
#define	NOT_SET_AUTHENTICATION_NUMBER			0

extern	struct	_Authentication	ex_Authentication;


#define	SEED1_VA1	authentication->seed_v1[0]		/* Seed Value1(High)	*/
#define	SEED1_VA2	authentication->seed_v1[1]		/* Seed Value1(Low)		*/
#define	SEED2_VA1	authentication->seed_v2[0]		/* Seed Value2(High)	*/
#define	SEED2_VA2	authentication->seed_v2[1]		/* Seed Value2(Low)		*/
#define	SEED3_VA1	authentication->seed_v3[0]		/* Seed Value3(High)	*/
#define	SEED3_VA2	authentication->seed_v3[1]		/* Seed Value3(Low)		*/
#define	CRC1_VA1	authentication->crc_v1[0]		/* CRC Value1(High)		*/
#define	CRC1_VA2	authentication->crc_v1[1]		/* CRC Value1(Low)		*/
#define	CRC2_VA1	authentication->crc_v2[0]		/* CRC Value2(High)		*/
#define	CRC2_VA2	authentication->crc_v2[1]		/* CRC Value3(Low)		*/
#define	CRC3_VA1	authentication->crc_v3[0]		/* CRC Value3(High)		*/
#define	CRC3_VA2	authentication->crc_v3[1]		/* CRC Value2(Low)		*/
#define	CRC_VA		authentication->crc_work		/* CRC Value		 	*/


extern void Authentication_main(void);
