/*************************************************************************************************
 * @Brief:						DHT11 - humidity and temperature sensor
 * @Team:						Dinh Vi So R&D
 * @Author(s):					Duy Vinh To
 *
 * @details:
 *************************************************************************************************/

/*-----------------------------------------------------------------------------------------------
 * INCLUDE
 ------------------------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------------------------
 * TYPEDEFS
 ------------------------------------------------------------------------------------------------*/
typedef unsigned char U8;
typedef signed char S8;
typedef unsigned short U16;
typedef unsigned long U32;

typedef struct
{
	U8 humi_int;
	U8 humi_dec;
	S8 temp_int;
	U8 temp_dec;
}ENVIRONMENT_t;

typedef void (*DHT11_delayus_CB)(U32 t);
typedef void (*DHT11_wrtIO_CB)(U8 set);
typedef U8 (*DHT11_rdIO_CB)(void);
/*-----------------------------------------------------------------------------------------------
 * FUNC.PROTOTYPES
 ------------------------------------------------------------------------------------------------*/
extern void DHT11_setup(DHT11_delayus_CB delayusCB, DHT11_wrtIO_CB wrtIOCB, DHT11_rdIO_CB rdIOCB);
//extern void DHT11_initIO( void );
extern U8 DHT11_read( ENVIRONMENT_t *env );
